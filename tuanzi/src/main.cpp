#include "psutil.h"
#include "checkrunthread.h"
#include "signals.h"
#include "suconfigfile.h"
#include "changelanguage.h"
#include "libutil.h"
#include "util.h"
#include "cmdutil.h"
#include "global.h"

enum AUTH_MODE {
    AUTH_INVALID,
    AUTH_WIRED,
    AUTH_WIRELESS
};

enum DHCP_MODE {
    DHCP_INVALID,
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

int main(int argc, char **argv)
{
    int option = 0;
    int longind = 0;
    bool has_password = false;
    bool save_password = false;
    bool list_mode = false;
    bool request_exit = false;
    bool show_version = false;
    bool scan_wlan = false;
    enum AUTH_MODE auth_method = AUTH_INVALID;
    enum DHCP_MODE dhcp_mode = DHCP_INVALID;
    char *nic = nullptr;
    char *password = nullptr;
    char *ssid = nullptr;
    char *service = nullptr;
    char *username = nullptr;
    char option_parse_error_str[2048] = {};
    bool option_parse_error = false;
    int save_password_tmp = 0;
    int start_thread_result = 0;
    const struct option longopt[] = {
        { "auth",    1, nullptr, 'a' }, { "dhcp",     1, nullptr, 'd' },
        { "nic",     1, nullptr, 'n' }, { "ssid",     1, nullptr, 'I' },
        { "wlan",    0, nullptr, 'w' }, { "service",  1, nullptr, 's' },
        { "user",    1, nullptr, 'u' }, { "password", 1, nullptr, 'p' },
        { "save",    1, nullptr, 'S' }, { "quit",     0, nullptr, 'q' },
        { "list",    0, nullptr, 'l' }, { "help",     0, nullptr, 'h' },
        { "version", 0, nullptr, 'v' }, { }
    };
    iniparser_set_error_callback(iniparser_error_callback);
    setAppEnvironment();
    TakeAppPath(g_strAppPath);
    g_runLogFile = g_strAppPath + "log/run.log";
    CChangeLanguage &cinstance = CChangeLanguage::Instance();
    cinstance.SetLanguage(GetSysLanguage());
    InitLogFiles();

    while ((option = getopt_long(
                         argc,
                         argv,
                         "qlh?wa:d:u:p:s:n:S:I:",
                         longopt,
                         &longind
                     )) != -1) {
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
                    continue;
                }

                snprintf(
                    option_parse_error_str, sizeof(option_parse_error_str),
                    cinstance.LoadString(2028).c_str(), 'S',
                    optarg
                );
                option_parse_error = true;
                break;

            case 'a':
                auth_method = static_cast<enum AUTH_MODE>(strtol(optarg, nullptr, 10));

                if (auth_method == AUTH_WIRED || auth_method == AUTH_WIRELESS)
                    continue;

                snprintf(
                    option_parse_error_str,
                    sizeof(option_parse_error_str),
                    cinstance.LoadString(2028).c_str(),
                    'a',
                    optarg
                );
                option_parse_error = true;
                break;

            case 'd':
                dhcp_mode = static_cast<enum DHCP_MODE>(strtol(optarg, nullptr, 10));

                if (dhcp_mode == DHCP_LOCAL || dhcp_mode == DHCP_SERVER)
                    continue;

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
                show_version = true;
                break;

            case 'w':
                scan_wlan = true;
                break;

            default:
                rj_printf_debug("default=%c\n", option);
                break;
        }
    }

    if (option_parse_error) { // error exists
        std::cerr << option_parse_error_str << std::endl;
        return EXIT_SUCCESS;
    }

    if (argc > optind) { // extra options
        fprintf(stderr, cinstance.LoadString(2030).c_str());

        while (argc > optind)
            std::cerr << argv[optind++];

        std::cerr << std::endl;
        return EXIT_SUCCESS;
    }

    if (show_version) { // requested to show version
        // ;)
        std::cout << "Ruijie Supplicant V1.31" << std::endl
                  << "transcribed by xiaoxi-ij478" << std::endl;
    }

    // make the log directory
    // original "mkdir -p ${g_strAppPath}log"
    if (
        mkdir((g_strAppPath + "log").c_str(), 0755) == -1 &&
        errno != EEXIST
    ) {
        perror("Could not make log directory");
        return EXIT_FAILURE;
    }

    if (geteuid()) { // not superuser
        std::cerr << cinstance.LoadString(257) << std::endl;
        return EXIT_SUCCESS;
    }

    if (request_exit == 1) {
        killrjsu();
        return EXIT_SUCCESS;
    }

    {
        CSuConfigFile sucfg;
        CSuConfigFile::Lock();

        if (sucfg.Open()) {
            sucfg.GetPrivateProfileInt("PUBLIC", "SaveCheck", 0);
            sucfg.Close();
        }

        CSuConfigFile::Unlock();
    }

    updteParam = false;
    update_message.clear();
    alt_update_message.clear();
    bLoadLib = false;
    hold_signals();
    set_signals();
    release_signals();
    signal(SIGCHLD, SIG_IGN);
    g_logChkRun.CreateLogFile_S(g_strAppPath + "log/chkrun.log", 1);

    if (!chkRunThread.StartThread(
                &start_thread_result,
                chk_call_back
            )) {
        if (start_thread_result == 1) {
            g_logChkRun.AppendText("aready run!!!!!!!!!!!!!!!!!!!!!!");
            std::cerr << cinstance.LoadString(199) << std::endl;

        } else
            g_logChkRun.AppendText("Check run error:%d", start_thread_result);

        return do_quit();
    }

    if (load_librt()) {
        std::cerr << cinstance.LoadString(265) << std::endl;
        return do_quit();
    }

    bLoadLib = true;
    set_msg_config("msgmni", 0x80);
    set_msg_config("msgmax", 0x6000);
    set_msg_config("msgmnb", 0xC000);
}
