#include "all.h"
#include "psutil.h"
#include "checkrunthread.h"
#include "signals.h"
#include "suconfigfile.h"
#include "changelanguage.h"
#include "libutil.h"
#include "util.h"
#include "cmdutil.h"
#include "timeutil.h"
#include "sysutil.h"
#include "netutil.h"
#include "mtypes.h"
#include "threadutil.h"
#include "userconfig.h"
#include "contextcontrolthread.h"
#include "global.h"

enum AUTH_MODE {
    AUTH_INVALID = -1,
    AUTH_WIRELESS,
    AUTH_WIRED
};

enum DHCP_MODE {
    DHCP_INVALID = -1,
    DHCP_LOCAL,
    DHCP_SERVER
};

[[gnu::format(printf, 1, 2)]]
static int iniparser_error_callback(const char *format, ...)
{
    // redirect iniparser issue to g_logSystem
    // this is not included in the original implementation
    va_list va;
    va_start(va, format);
    g_logSystem.AppendText_V(format, va);
    va_end(va);
    return 0;
}

int option = 0;
int save_password = -1;
int list_mode = -1;
int request_exit = -1;
int scan_wlan = -1;
int request_quit = -1;
int version = -1;
bool has_password = false;
enum AUTH_MODE auth_mode = AUTH_INVALID;
enum DHCP_MODE dhcp_mode = DHCP_INVALID;
char *nic = nullptr;
char *password = nullptr;
char *ssid = nullptr;
char *service = nullptr;
char *username = nullptr;
int save_password_tmp = 0;
int start_thread_result = 0;
const char *opt_string = "qlh?wa:d:u:p:s:n:S:I:";
const struct option long_opts[] = {
    { "auth",    1, nullptr, 'a' }, { "dhcp",     1, nullptr, 'd' },
    { "nic",     1, nullptr, 'n' }, { "ssid",     1, nullptr, 'I' },
    { "wlan",    0, nullptr, 'w' }, { "service",  1, nullptr, 's' },
    { "user",    1, nullptr, 'u' }, { "password", 1, nullptr, 'p' },
    { "save",    1, nullptr, 'S' }, { "quit",     0, nullptr, 'q' },
    { "list",    0, nullptr, 'l' }, { "help",     0, nullptr, 'h' },
    { "version", 0, nullptr, 'v' }, { }
};

int main(int argc, char **argv)
{
    static unsigned long time = GetDayTime();
    int longind = 0;
    char option_parse_error_str[1024] = {};
    bool option_parse_error = false;
    char read_cmd = 0;
    std::string input_password;
    std::vector<std::string> usable_nics;
    fd_set fds;
    iniparser_set_error_callback(iniparser_error_callback);
    setAppEnvironment();
    TakeAppPath(g_strAppPath);
    g_runLogFile = g_strAppPath + "log/run.log";
    CChangeLanguage &cinstance = CChangeLanguage::Instance();
    cinstance.SetLanguage(GetSysLanguage());
    InitLogFiles();

    while (
        (option = getopt_long(argc, argv, opt_string, long_opts, &longind)) != -1
    )
        switch (option) {
            case ':':
                fprintf(stderr, cinstance.LoadString(2029).c_str(), optopt);
                break;

            case '?':
            case 'h':
                display_usage();
                break;

            case 'I':
                ssid = optarg;
                break;

            case 'S':
                save_password_tmp = strtol(optarg, nullptr, 10);

                if (save_password_tmp == 0 || save_password_tmp == 1) {
                    save_password = save_password_tmp;
                    break;
                }

                sprintf(
                    option_parse_error_str,
                    cinstance.LoadString(2028).c_str(),
                    'S',
                    optarg
                );
                option_parse_error = true;
                break;

            case 'a':
                auth_mode = static_cast<enum AUTH_MODE>(strtol(optarg, nullptr, 10));

                if (auth_mode == AUTH_WIRED || auth_mode == AUTH_WIRELESS)
                    break;

                sprintf(
                    option_parse_error_str,
                    cinstance.LoadString(2028).c_str(),
                    'a',
                    optarg
                );
                option_parse_error = true;
                break;

            case 'd':
                dhcp_mode = static_cast<enum DHCP_MODE>(strtol(optarg, nullptr, 10));

                if (dhcp_mode == DHCP_LOCAL || dhcp_mode == DHCP_SERVER)
                    break;

                snprintf(
                    option_parse_error_str,
                    sizeof(option_parse_error_str),
                    cinstance.LoadString(2028).c_str(),
                    'd',
                    optarg
                );
                option_parse_error = true;
                break;

            case 'l':
                list_mode = true;
                break;

            case 'n':
                nic = optarg;
                break;

            case 'p':
                has_password = true;
                password = optarg;
                break;

            case 'q':
                request_exit = true;
                break;

            case 's':
                service = optarg;
                break;

            case 'u':
                username = optarg;
                break;

            case 'v':
                version = true;
                break;

            case 'w':
                scan_wlan = true;
                break;

            default:
                rj_printf_debug("default=%c\n", option);
                break;
        }

    if (option_parse_error) { // error exists
        message_info("%s", option_parse_error_str);
        return EXIT_SUCCESS;
    }

    if (argc > optind) { // extra options
        message_info(cinstance.LoadString(2030));

        while (argc > optind)
            message_info(argv[optind++]);

        message_info("\n");
        return EXIT_SUCCESS;
    }

    if (version == 1) { // requested to show version
        // ;)
        std::cout << "Ruijie Supplicant V1.31" << std::endl
                  << "written by xiaoxi-ij478" << std::endl;
        return EXIT_SUCCESS;
    }

    // make the log directory
    // "mkdir -p ${g_strAppPath}log"
    mkdir(
        (g_strAppPath + "log").c_str(),
        S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH
    );

    if (geteuid()) { // not superuser
        message_info(cinstance.LoadString(257) + '\n');
//        return EXIT_SUCCESS;
    }

    if (request_exit == 1) {
        killrjsu();
        return EXIT_SUCCESS;
    }

    {
        // it has a recursive lock, so we must use a local variable
        CSuConfigFile sucfg;
        CSuConfigFile::Lock();

        if (sucfg.Open()) {
            sucfg.GetPrivateProfileInt("PUBLIC", "SaveCheck", 0);
            sucfg.Close();
        }

        CSuConfigFile::Unlock();
    }

    updteParam = false;
    update_file_path.clear();
    update_message.clear();
    bLoadLib = false;
    hold_signals();
    set_signals();
    release_signals();
    signal(SIGCHLD, SIG_IGN);
    g_logChkRun.CreateLogFile_S(g_strAppPath + "log/chkrun.log", 1);

    if (!chkRunThred.StartThread(&start_thread_result, chk_call_back)) {
        if (start_thread_result == 1) {
            g_logChkRun.AppendText("aready run!!!!!!!!!!!!!!!!!!!!!!");
            message_info(cinstance.LoadString(199) + '\n');

        } else
            g_logChkRun.AppendText("Check run error:%d", start_thread_result);

        return do_quit();
    }

    if (load_librt()) {
        message_info(cinstance.LoadString(265) + '\n');
        return do_quit();
    }

    bLoadLib = true;
    set_msg_config("msgmni", 0x80);
    set_msg_config("msgmax", 0x6000);
    set_msg_config("msgmnb", 0xC000);
    InitAppMain();

    if (list_mode == 1) {
        switch (auth_mode) {
            case AUTH_INVALID:
                show_auth_info(false, true);
                break;

            case AUTH_WIRELESS:
                show_auth_info(true, true);
                break;

            case AUTH_WIRED:
                show_auth_info(true, false);
                break;
        }

        return do_quit();
    }

    if (scan_wlan == 1) {
        show_wlan_scan_info(nic);
        return do_quit();
    }

    switch (auth_mode) {
        case AUTH_WIRELESS:
            CtrlThread->configure_info.public_authmode = "EAPWIRELESS";
            break;

        case AUTH_WIRED:
            CtrlThread->configure_info.public_authmode = "EAPMD5";
            break;
    }

    if (dhcp_mode != DHCP_INVALID)
        CtrlThread->configure_info.public_dhcp = dhcp_mode;

    if (service)
        CtrlThread->configure_info.public_service = service;

    if (ssid && CtrlThread->configure_info.public_authmode == "EAPWIRELESS")
        CtrlThread->configure_info.public_wirelessconf = ssid;

    if (nic)
        CtrlThread->configure_info.public_adapter = nic;

    else if (
        CtrlThread->configure_info.public_adapter.empty() &&
        !CtrlThread->nic_in_use.empty()
    )
        CtrlThread->configure_info.public_adapter = CtrlThread->nic_in_use.front();

    if (username)
        CtrlThread->configure_info.last_auth_username = username;

    if (password)
        CtrlThread->configure_info.last_auth_password = password;

    if (save_password != -1)
        CtrlThread->configure_info.public_savecheck = save_password == 1;

    if (CtrlThread->configure_info.last_auth_username.empty()) {
        message_info(cinstance.LoadString(9) + '\n');
        return do_quit();
    }

    if (CtrlThread->configure_info.last_auth_username.length() >= 128) {
        message_info(cinstance.LoadString(10) + '\n');
        return do_quit();
    }

    if (CtrlThread->configure_info.last_auth_password.length() >= 128) {
        message_info(cinstance.LoadString(16) + '\n');
        return do_quit();
    }

    if (CtrlThread->configure_info.public_adapter.empty()) {
        message_info(cinstance.LoadString(20) + '\n');
        return do_quit();
    }

    if (
        std::find(
            CtrlThread->nic_in_use.cbegin(),
            CtrlThread->nic_in_use.cend(),
            CtrlThread->configure_info.public_adapter
        ) == CtrlThread->nic_in_use.cend()
    ) {
        message_info(cinstance.LoadString(1) + '\n');
        return do_quit();
    }

    if (CtrlThread->configure_info.public_dhcp == -1) {
        message_info(cinstance.LoadString(2042) + '\n');
        return do_quit();
    }

    if (CtrlThread->configure_info.public_authmode == "EAPWIRELESS") {
        get_nic_in_use(usable_nics, true);

        if (usable_nics.empty()) {
            message_info(cinstance.LoadString(142) + '\n');
            return do_quit();
        }

        if (
            std::find(
                usable_nics.cbegin(),
                usable_nics.cend(),
                CtrlThread->configure_info.public_adapter
            ) == usable_nics.cend()
        )
            CtrlThread->configure_info.public_adapter = usable_nics.front();

        if (CtrlThread->configure_info.public_wirelessconf.empty()) {
            if (
                !CtrlThread->RefreshSignal(
                    CtrlThread->configure_info.public_adapter
                )
            )
                return do_quit();

            message_info(cinstance.LoadString(2040) + '\n');
            WaitForSingleObject(&CtrlThread->scan_completed, 0);

            if (CtrlThread->configure_info.public_wirelessconf.empty()) {
                message_info(cinstance.LoadString(2041) + '\n');
                return do_quit();
            }
        }
    }

    if (
        CtrlThread->configure_info.public_service.empty() &&
        !CtrlThread->configure_info.server_names.empty()
    )
        CtrlThread->configure_info.public_service =
            CtrlThread->configure_info.server_names.front();

    if (is_run_background())
        g_background = true;

    else
        set_termios(false);

    if (CtrlThread->configure_info.last_auth_password.empty()) {
        message_info(cinstance.LoadString(2044) + ':');

        if (!std::getline(std::cin, input_password))
            return do_quit();

        if (input_password.length() >= 128) {
            message_info(cinstance.LoadString(16) + '\n');
            return do_quit();
        }

        CtrlThread->configure_info.last_auth_password = input_password;
        message_info("\n");
    }

    if (pipe(g_rwpipe) == -1)
        perror("pipe error");

    CUserConfig::SaveSupplicantConf();
    CtrlThread->PostThreadMessage(CTRLTHREAD_START_STATE_MACHINE_MTYPE, 0, 0);
    show_connect_user_info();
    SetRunModeCheckTimer();

    while (true) {
        while (true) {
            FD_ZERO(&fds);

            if (is_run_background())
                g_background = true;

            else {
                if (g_background) {
                    set_termios(false);
                    g_background = false;
                    g_uilog.AppendText("main 后台切换到前台 g_background=0");
                }

                FD_SET(STDIN_FILENO, &fds);
            }

            FD_SET(g_rwpipe[0], &fds);

            if (select(g_rwpipe[0] + 1, &fds, nullptr, nullptr, nullptr) > 0)
                break;

            g_uilog.AppendText("main select return %s", strerror(errno));
        }

        if (FD_ISSET(g_rwpipe[0], &fds)) {
            read(g_rwpipe[0], &read_cmd, 1);

            if (read_cmd == 'q')
                return do_quit();

            if (read_cmd == 'c') {
                g_uilog.AppendText("main 收到信息");
                continue;
            }

        } else
            g_uilog.AppendText("main FD_ISSET pipe false.");

        if (FD_ISSET(STDIN_FILENO, &fds)) {
            read(STDIN_FILENO, &read_cmd, 1);

            switch (dispatch_cmd(read_cmd)) {
                case 0:
                    return do_quit();

                case 2:
                    if ((GetDayTime() - time) > 50) {
                        time = GetDayTime();
                        message_info(cinstance.LoadString(2005) + '\n');
                    }

                    break;

                case 3:
                    message_info(cinstance.LoadString(2006) + '\n');
                    break;
            }

        } else
            g_uilog.AppendText("main FD_ISSET STDIN_FILENO false.");
    }

    return do_quit();
}
