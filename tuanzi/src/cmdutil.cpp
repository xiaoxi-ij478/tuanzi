#include "all.h"
#include "global.h"
#include "timeutil.h"
#include "msgutil.h"
#include "libutil.h"
#include "fileutil.h"
#include "sysutil.h"
#include "netutil.h"
#include "util.h"
#include "mtypes.h"
#include "changelanguage.h"
#include "passwordmodifier.h"
#include "contextcontrolthread.h"
#include "cmdutil.h"

void exec_cmd(const char *cmd, char *buf, unsigned buflen)
{
    FILE *fp = popen(cmd, "r");
    unsigned read_len = 0;

    if (!fp) {
        rj_printf_debug("exec_cmd popen null:%s\n", strerror(errno));
        return;
    }

    if ((read_len = fread(buf, 1, buflen - 1, fp)) <= 0) {
        rj_printf_debug("exec_cmd fread size <= 0\n");

        if (buflen)
            *buf = 0;

    } else
        buf[read_len] = 0;

    pclose(fp);
}

void message_info(const char *format, ...)
{
    va_list arg;
    va_start(arg, format);
    vprintf(format, arg);
    va_end(arg);
}

void message_info(const std::string &str)
{
    std::cout << str;
}

void display_usage()
{
    unsigned short tc_width = get_tc_width();
    char str2[2048] = {};
    std::string log_path(g_strAppPath + "log/run.log");
    CChangeLanguage &cinstance = CChangeLanguage::Instance();
    std::cout << cinstance.LoadString(2007) << std::endl;
#define PRINT_USAGE(head, help_str_id) \
    do { \
        std::cout << (head); \
        format_tc_string(tc_width, 24, cinstance.LoadString(help_str_id)); \
        std::cout << std::endl; \
    } while(0)
    PRINT_USAGE("\t-a --auth\t", 2008);
    PRINT_USAGE("\t-d --dhcp\t", 2043);
    PRINT_USAGE("\t-n --nic\t", 2009);
    PRINT_USAGE("\t-s --service\t", 2010);
    PRINT_USAGE("\t-I --ssid\t", 2011);
    PRINT_USAGE("\t-w --wlan\t", 2061);
    PRINT_USAGE("\t-u --user\t", 2012);
    PRINT_USAGE("\t-p --password\t", 2013);
    PRINT_USAGE("\t-S --save\t", 2014);
    PRINT_USAGE("\t-q --quit\t", 2046);
#undef PRINT_USAGE
    std::cout << "\t   --comments\t";
    sprintf(str2, cinstance.LoadString(2045).c_str(), log_path.c_str());
    format_tc_string(tc_width, 24, str2);
    exit(1);
}

unsigned short get_tc_width()
{
    unsigned short def = 80;
    struct winsize wsz = {};

    if (ioctl(0, TIOCGWINSZ, &wsz) == -1) {
        rj_printf_debug("get_tc_width ioctl=%s\n", strerror(errno));
        return def;

    } else {
        if (wsz.ws_col)
            return wsz.ws_col;

        return def;
    }
}

void rj_printf_debug(const char *format, ...)
{
    va_list arg;
    va_start(arg, format);
    vprintf(format, arg);
    va_end(arg);
}

void format_tc_string(
    unsigned short tc_width,
    unsigned indent_len,
    const std::string &str
)
{
    // i wrote this function myself since I
    // can't read what the heck they are...
    unsigned actual_line_width = tc_width - indent_len;
    unsigned remain = str.length();
    unsigned actual_write = 0;
    char *spaces = new char[indent_len + 1];
    const char *rstr = str.c_str();
    memset(spaces, ' ', sizeof(char) * indent_len);
    spaces[indent_len] = 0;

    while (true) {
        actual_write = remain > actual_line_width ? actual_line_width : remain;
        std::cout.write(rstr, actual_write);
        rstr += actual_write;
        remain -= actual_write;

        if (!remain)
            break;

        std::cout << std::endl << spaces;
    }

    delete[] spaces;
    spaces = nullptr;
}

void fill_tc_left_char(unsigned len, char c)
{
    if (len <= 0)
        return;

    for (unsigned i = 0; i < len; i++)
        std::cout << c;

    std::cout.flush();
}

void print_separator(const char *s, int len, bool print_crlf)
{
    if (len < 0) {
        if (print_crlf)
            std::cout << std::endl;

        return;
    }

    if (!len)
        len = 10;

    for (int i = 0; i < len; i++)
        std::cout << s;

    if (print_crlf)
        std::cout << std::endl;
}

void print_string_list(
    const char *prefix,
    const std::vector<std::string> &slist
)
{
    format_tc_string(12, 0, prefix);
    std::cout << CChangeLanguage::Instance().LoadString(2059)
              << '[' << slist.size() << ']' << std::endl;

    for (auto it = slist.cbegin(); it != slist.cend(); it++) {
        fill_tc_left_char(12, ' ');
        std::cout << '['
                  << std::distance(slist.cbegin(), it)
                  << "] " << *it << std::endl;
    }
}

bool check_quit()
{
    char c = 0;
    fd_set fds;
    message_info(CChangeLanguage::Instance().LoadString(2039));
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds);

    if (select(32, &fds, nullptr, nullptr, nullptr) <= 0 || !FD_ISSET(1, &fds))
        return true;

    read(STDIN_FILENO, &c, 1);
    message_info("\n");
    return c == '\n';
}

bool check_safe_exit(bool create_file)
{
    std::string lockfile(g_strAppPath);
    std::ofstream ofs;
    lockfile.append(".rgsusfexit");
    rj_printf_debug("%s strFile=%s\n", "check_safe_exit", lockfile.c_str());

    if (!create_file) {
        if (unlink(lockfile.c_str()) == -1) {
            rj_printf_debug(
                "%s strFile=%s exist and unlink failed \n",
                "check_safe_exit",
                lockfile.c_str()
            );
            return false;
        }

        rj_printf_debug(
            "%s strFile=%s exist and unlink ok \n",
            "check_safe_exit",
            lockfile.c_str()
        );
        return true;
    }

    if (!access(lockfile.c_str(), F_OK)) {
        rj_printf_debug("%s strFile=%s exist\n", "check_safe_exit", lockfile.c_str());
        return false;
    }

    ofs.open(lockfile.c_str());

    if (!ofs) {
        rj_printf_debug(
            "%s strFile=%s no exist and create failed \n",
            "check_safe_exit",
            lockfile.c_str()
        );
        return false;
    }

    ofs.close();
    rj_printf_debug(
        "%s strFile=%s no exist and create ok 2\n",
        "check_safe_exit",
        lockfile.c_str()
    );
    return true;
}

bool is_run_background()
{
    int forep = tcgetpgrp(STDIN_FILENO);
    int myp = getpgrp();

    if (forep == -1) {
        rj_printf_debug(
            "tcgetpgrp STDIN_FILENO(%d) error:%s",
            STDIN_FILENO,
            strerror(errno)
        );

        if ((forep = tcgetpgrp(STDOUT_FILENO)) == -1) {
            rj_printf_debug("tcgetpgrp STDOUT_FILENO error:%s", strerror(errno));
            return false;
        }
    }

    if (myp == -1) {
        rj_printf_debug("getpgrp error:%s", strerror(errno));
        return false;
    }

    return forep != myp;
}

int set_termios(bool set_echo_icanon)
{
    struct termios term = {};

    if (tcgetattr(STDIN_FILENO, &term) == -1) {
        rj_printf_debug("set_termios: tcgetattr error:%s", strerror(errno));
        return -1;
    }

    if (set_echo_icanon)
        term.c_lflag |= ICANON | ECHO;

    else
        term.c_lflag &= ~(ICANON | ECHO);

    if (tcsetattr(STDIN_FILENO, TCSANOW, &term) == -1)
        rj_printf_debug("tcsetattr error:%s", strerror(errno));

    return 0;
}

void shownotify(
    const std::string &content,
    const std::string &header,
    [[maybe_unused]] unsigned timeout
)
{
    char cur_date[64] = {};

    if (content != g_strNotify)
        return;

    g_strNotify = content;
    GetCurDataAndTime(cur_date);
    message_info(cur_date + header);
    format_tc_string(get_tc_width(), 20, "\n" + content + "\n");
    CLogFile::LogToFile(
        (header + ' ' + content).c_str(),
        g_runLogFile.c_str(),
        true,
        true
    );
}

void show_url(const std::string &header, const std::string &content)
{
    if (content.empty())
        return;

    message_info(header + ':');
    format_tc_string(get_tc_width(), 20, content + '\n');
}

unsigned show_version()
{
    format_tc_string(12, 0, CChangeLanguage::Instance().LoadString(2003));
    message_info(CChangeLanguage::Instance().LoadString(2004) + '\n');
    return 1;
}

unsigned show_message_info()
{
    std::vector<struct tagMsgItem> msgs;
    GetMsgArray(msgs);
    print_msg_item_header();
    std::for_each(msgs.cbegin(), msgs.cend(), print_msg_item);
    return 1;
}

void show_login_url()
{
    if (!theApp.IsOnline())
        return;

    show_url(
        CChangeLanguage::Instance().LoadString(2062),
        CtrlThread->private_properties.user_login_url
    );
}

unsigned do_modify_password()
{
    std::string old_password, new_password_first, new_password_second;

    if (
        !CPasswordModifier::GetPasswordSecurityInfo()->enable_modify_pw ||
        !theApp.IsOnline()
    )
        return 2;

    message_info(CChangeLanguage::Instance().LoadString(2053) + '\n');

    while (true) {
        message_info(CChangeLanguage::Instance().LoadString(2054) + ':');

        if (!std::getline(std::cin, old_password) || old_password.empty())
            continue;

        message_info(
            std::string("\n") + CChangeLanguage::Instance().LoadString(2055) + ':'
        );

        if (!std::getline(std::cin, new_password_first) || new_password_first.empty())
            continue;

        message_info(
            std::string("\n") + CChangeLanguage::Instance().LoadString(2056) + ':'
        );

        if (!std::getline(std::cin, new_password_second) || new_password_second.empty())
            continue;

        if (new_password_first == new_password_second)
            break;

        message_info(CChangeLanguage::Instance().LoadString(272) + ':');
    }

    g_strNotify.clear();
    CPasswordModifier::SetSubmitedNewPassword(new_password_first);

    if (!CPasswordModifier::SendModifyPWRequest(old_password, new_password_first))
        g_uilog.AppendText(
            "%s>SendModifyPWRequest fail.",
            // something undefined???
            // we assume it's the function's name
            "do_modify_password"
        );

    g_llmodifypwdstart = GetTickCount();
    g_bmodifypwdstart = true;
    message_info("\n");
    return 1;
}

unsigned dispatch_cmd(char cmd)
{
    switch (cmd) {
        case '?':
        case 'h':
            display_help();
            return 1;

        case 'a':
            return show_all_info();

        case 'm':
            return show_message_info();

        case 'n':
            return show_connect_net_info();

        case 'o':
            return show_sso_url();

        case 'p':
            return do_modify_password();

        case 'q':
            return !check_quit();

        case 's':
            return do_switch_service();

        case 't':
            return show_connect_time();

        case 'u':
            return show_connect_user_info();

        case 'v':
            return show_version();

        default:
            return 2;
    }
}

void display_help()
{
    message_info(CChangeLanguage::Instance().LoadString(2016) + '\n');

    if (
        theApp.IsOnline() &&
        !CtrlThread->configure_info.server_names.empty() &&
        CtrlThread->GetRadiusServer() == 9
    ) {
        message_info("\ts\t");
        format_tc_string(
            get_tc_width(),
            24,
            CChangeLanguage::Instance().LoadString(2017) + '\n'
        );
    }

    if (
        theApp.IsOnline() &&
        CPasswordModifier::GetPasswordSecurityInfo()->enable_modify_pw
    ) {
        message_info("\ts\t");
        format_tc_string(
            get_tc_width(),
            24,
            CChangeLanguage::Instance().LoadString(2053) + '\n'
        );
    }

    if (
        theApp.IsOnline() &&
        !CtrlThread->private_properties.utrust_url.empty()
    ) {
        message_info("\to\t");
        format_tc_string(
            get_tc_width(),
            24,
            CChangeLanguage::Instance().LoadString(2057) + '\n'
        );
    }

    message_info("\ta\t");
    format_tc_string(
        get_tc_width(),
        24,
        CChangeLanguage::Instance().LoadString(2018) + '\n'
    );
    message_info("\tt\t");
    format_tc_string(
        get_tc_width(),
        24,
        CChangeLanguage::Instance().LoadString(2019) + '\n'
    );
    message_info("\tu\t");
    format_tc_string(
        get_tc_width(),
        24,
        CChangeLanguage::Instance().LoadString(2020) + '\n'
    );
    message_info("\tn\t");
    format_tc_string(
        get_tc_width(),
        24,
        CChangeLanguage::Instance().LoadString(2021) + '\n'
    );
    message_info("\tv\t");
    format_tc_string(
        get_tc_width(),
        24,
        CChangeLanguage::Instance().LoadString(2022) + '\n'
    );
    message_info("\tq\t");
    format_tc_string(
        get_tc_width(),
        24,
        CChangeLanguage::Instance().LoadString(2023) + '\n'
    );
    message_info("\th,?\t");
    format_tc_string(
        get_tc_width(),
        24,
        CChangeLanguage::Instance().LoadString(2024) + '\n'
    );
}

int modify_password_timeout(bool reset)
{
    if (!g_bmodifypwdstart)
        return -1;

    if (reset)
        g_bmodifypwdstart = false;

    return
        reset ?
        0 :
        (GetTickCount() - g_llmodifypwdstart) >
        (1000 * CPasswordModifier::GetPasswordSecurityInfo()->timeout);
}

unsigned show_sso_url()
{
    if (!theApp.IsOnline())
        return 2;

    show_url(
        CChangeLanguage::Instance().LoadString(2058),
        CtrlThread->private_properties.utrust_url
    );
    return 1;
}

bool post_command(char c)
{
    for (int i = 0; i < 3; i++) {
        if (write(g_rwpipe[1], &c, 1) != -1)
            return true;

        perror("post_command write pipe");
    }

    return false;
}

int do_quit()
{
    static unsigned i = 0;
    struct updateArg_t update_arg = { updteParam, update_file_path, update_message };

    if (i)
        return -1;

    i++;
    KillRunModeCheckTimer();

    if (is_run_background())
        rj_printf_debug("It is back ground.");

    else {
        rj_printf_debug("It is not back ground.");
        set_termios(true);
    }

    if (CtrlThread) {
        CtrlThread->PostThreadMessage(CONNECT_NOTIFY_MTYPE, 0, 0);

        if (CtrlThread->IS_WIRED(RFC_EAP_NONE))
            check_safe_exit(false);

        CtrlThread->SafeExitThread(10000);
        CtrlThread = nullptr;
    }

    chkRunThred.StopThread();

    if (bLoadLib)
        free_librt();

    do_update(&update_arg);
    rj_printf_debug("exit all\n");
    return 0;
}

unsigned do_switch_service()
{
    char tmpbuf[1024] = {};
    int service_index = -1;
    std::string line;

    if (
        CtrlThread->configure_info.server_names.empty() ||
        !theApp.IsOnline() ||
        CtrlThread->GetRadiusServer() != 9
    )
        return 2;

    if (CtrlThread->configure_info.server_names.size() != 1) {
        print_service_list(true);
        set_termios(true);

        if (std::getline(std::cin, line)) {
            service_index = std::stoi(line);

            if (
                service_index < 0 ||
                service_index > CtrlThread->configure_info.server_names.size()
            ) {
                rj_printf_debug(
                    "input error:sid=%s,id=%d\n",
                    line.c_str(),
                    service_index
                );
                set_termios(false);
                return 1;
            }
        }

        set_termios(false);

        if (
            CtrlThread->configure_info.server_names[service_index] ==
            CtrlThread->configure_info.public_service
        ) {
            sprintf(
                tmpbuf,
                CChangeLanguage::Instance().LoadString(255).c_str(),
                CtrlThread->configure_info.public_service.c_str()
            );
            message_info("%s\n", tmpbuf);

        } else {
            g_strNotify.clear();
            ServiceSwitch(CtrlThread->configure_info.server_names[service_index]);
        }
    }

    print_service_list(false);
    message_info(CChangeLanguage::Instance().LoadString(2032) + '\n');
    return 1;
}

void print_service_list(bool do_switch)
{
    if (CtrlThread->configure_info.server_names.empty())
        return;

    if (do_switch)
        message_info(CChangeLanguage::Instance().LoadString(2027) + '\n');

    else
        format_tc_string(12, 0, CChangeLanguage::Instance().LoadString(2026));

    for (
        auto it = CtrlThread->configure_info.server_names.cbegin();
        it != CtrlThread->configure_info.server_names.cend();
        it++
    ) {
        if (
            it != CtrlThread->configure_info.server_names.cbegin() ||
            do_switch
        )
            fill_tc_left_char(12, ' ');

        if (*it == CtrlThread->configure_info.public_service)
            message_info(
                "[%d]* %s\n",
                std::distance(CtrlThread->configure_info.server_names.cbegin(), it),
                it->c_str()
            );

        else
            message_info(
                "[%d]  %s\n",
                std::distance(CtrlThread->configure_info.server_names.cbegin(), it),
                it->c_str()
            );
    }
}

void do_update(struct updateArg_t *update_arg)
{
    const char *exe = nullptr;

    if (!update_arg->update || update_arg->update_file_path.empty())
        return;

    removeFileOrDir("/tmp/rjsupplicant/");
    decompressFile(update_arg->update_file_path.c_str(), "/tmp/");

    if (!update_arg->update_message.empty())
        message_info(update_arg->update_message + '\n');

    if (Is64BIT()) {
        // chmod +x /tmp/rjsupplicant/x64/updateproduct
        exe = "/tmp/rjsupplicant/x64/updateproduct";
        chmod(exe, S_IRWXU | S_IRWXG | S_IRWXO);

    } else {
        // chmod +x /tmp/rjsupplicant/x86/updateproduct
        exe = "/tmp/rjsupplicant/x86/updateproduct";
        chmod(exe, S_IRWXU | S_IRWXG | S_IRWXO);
    }

    // chmod +x /tmp/rjsupplicant/rjsupplicant.sh
    chmod("/tmp/rjsupplicant/rjsupplicant.sh", S_IRWXU | S_IRWXG | S_IRWXO);

    switch (GetSysLanguage()) {
        case LANG_ENGLISH:
            execl(exe, exe, "-p", g_strAppPath.c_str(), "-e", nullptr);
            break;

        case LANG_CHINESE:
            execl(exe, exe, "-p", g_strAppPath.c_str(), nullptr);
            break;
    }
}

unsigned show_all_info()
{
    show_connect_time();
    show_connect_user_info();
    print_separator("-", 40, 1);
    show_connect_net_info();
    print_separator("-", 40, 1);
    show_version();
    return 1;
}

void show_auth_info(bool use_default, bool wireless_only)
{
    show_version();

    if (use_default) {
        show_connect_user_info();
        print_separator("-", 40, 1);
        print_service_list(false);
        print_nic_list(wireless_only);

    } else {
        show_connect_user_info();
        print_separator("-", 40, 1);
        print_service_list(false);
        print_nic_list(CtrlThread->configure_info.public_authmode != "EAPMD5");
    }
}

unsigned show_connect_net_info()
{
    struct DHCPIPInfo dhcp_ipinfo = {};
    CChangeLanguage &cinstance = CChangeLanguage::Instance();
    CtrlThread->GetDHCPInfoParam(dhcp_ipinfo);
    format_tc_string(12, 0, cinstance.LoadString(153));
    message_info("%s\n", inet_ntoa({ dhcp_ipinfo.ip4_ipaddr }));
    format_tc_string(12, 0, cinstance.LoadString(154));
    message_info("%s\n", inet_ntoa({ dhcp_ipinfo.ip4_netmask }));
    format_tc_string(12, 0, cinstance.LoadString(155));
    message_info("%s\n", inet_ntoa({ dhcp_ipinfo.gateway }));
    format_tc_string(12, 0, cinstance.LoadString(160));
    message_info("%s\n", inet_ntoa({ dhcp_ipinfo.dns }));
    format_tc_string(12, 0, cinstance.LoadString(156));
    message_info(
        "%.2X-%.2X-%.2X-%.2X-%.2X-%.2X\n",
        dhcp_ipinfo.adapter_mac.ether_addr_octet[0],
        dhcp_ipinfo.adapter_mac.ether_addr_octet[1],
        dhcp_ipinfo.adapter_mac.ether_addr_octet[2],
        dhcp_ipinfo.adapter_mac.ether_addr_octet[3],
        dhcp_ipinfo.adapter_mac.ether_addr_octet[4],
        dhcp_ipinfo.adapter_mac.ether_addr_octet[5]
    );
    return 1;
}

unsigned show_connect_time()
{
    unsigned long success_time = 0;
    success_time =
        theApp.success_time ?
        0 :
        (GetTickCount() - theApp.success_time) / 1000;
    format_tc_string(12, 0, CChangeLanguage::Instance().LoadString(27));
    message_info(
        "%02lld:%02lld:%02lld\n",
        success_time / 3600,
        success_time % 3600 / 60,
        success_time % 60
    );
    return 1;
}

unsigned show_connect_user_info()
{
    std::string logfile_path = g_strAppPath + "log/run.log";
    CChangeLanguage &cinstance = CChangeLanguage::Instance();
    format_tc_string(12, 0, CChangeLanguage::Instance().LoadString(2000));

    if (CtrlThread->configure_info.public_authmode == "EAPMD5") {
        message_info(cinstance.LoadString(164) + '\n');
        CLogFile::LogToFile(
            (cinstance.LoadString(2000) + "  " + cinstance.LoadString(164)).c_str(),
            logfile_path.c_str(),
            true,
            true
        );

    } else {
        message_info(cinstance.LoadString(191) + '\n');
        CLogFile::LogToFile(
            (cinstance.LoadString(2000) + "  " + cinstance.LoadString(191)).c_str(),
            logfile_path.c_str(),
            true,
            true
        );
        format_tc_string(12, 0, cinstance.LoadString(2036));
        message_info(CtrlThread->configure_info.public_wirelessconf + '\n');
        CLogFile::LogToFile(
            (
                cinstance.LoadString(2036) + "  " +
                CtrlThread->configure_info.public_wirelessconf
            ).c_str(),
            logfile_path.c_str(),
            true,
            true
        );
    }

    format_tc_string(12, 0, cinstance.LoadString(2001));
    message_info(CtrlThread->configure_info.public_adapter + '\n');
    CLogFile::LogToFile(
        (
            cinstance.LoadString(2001) + "  " +
            CtrlThread->configure_info.public_adapter
        ).c_str(),
        logfile_path.c_str(),
        true,
        true
    );
    format_tc_string(12, 0, cinstance.LoadString(2002));
    message_info(CtrlThread->configure_info.last_auth_username + '\n');
    CLogFile::LogToFile(
        (
            cinstance.LoadString(2002) + "  " +
            CtrlThread->configure_info.last_auth_username
        ).c_str(),
        logfile_path.c_str(),
        true,
        true
    );

    if (!CtrlThread->configure_info.public_service.empty()) {
        format_tc_string(12, 0, cinstance.LoadString(2037));
        message_info(CtrlThread->configure_info.public_service + '\n');
        CLogFile::LogToFile(
            (
                cinstance.LoadString(2037) + "  " +
                CtrlThread->configure_info.public_service
            ).c_str(),
            logfile_path.c_str(),
            true,
            true
        );
    }

    return 1;
}

void show_wlan_scan_info(const char *adapter_name)
{
    CChangeLanguage &cinstance = CChangeLanguage::Instance();
    bool adapter_usable = false;
    char tmpbuf[64] = {};
    std::vector<std::string> nics;
    std::vector<std::string> signals_strlist;
    std::vector<struct tagWirelessSignal> signals;
    get_nic_in_use(nics, true);

    if (adapter_name) {
        if (std::find(nics.cbegin(), nics.cend(), adapter_name) == nics.cend())
            message_info(
                "%s %s\n",
                adapter_name,
                cinstance.LoadString(2060).c_str()
            );

    } else
        print_string_list(
            cinstance.LoadString(2025).c_str(),
            nics
        );

    adapter_usable =
        std::find(nics.cbegin(), nics.cend(), adapter_name) != nics.cend();
    message_info("%s\n", cinstance.LoadString(2040).c_str());
    get_ssid_list(adapter_usable ? adapter_name : nics.front().c_str(), signals);

    for (const struct tagWirelessSignal &signal : signals) {
        sprintf(tmpbuf, "(%d%%)", signal.qual);
        signals_strlist.push_back(std::string(signal.ssid).append(tmpbuf));
    }

    print_string_list(cinstance.LoadString(2038).c_str(), signals_strlist);
}

void print_nic_list(bool wireless_only)
{
    std::vector<std::string> nics;
    get_nic_in_use(nics, wireless_only);
    format_tc_string(12, 0, CChangeLanguage::Instance().LoadString(2025));

    if (nics.empty()) {
        message_info(CChangeLanguage::Instance().LoadString(112) + '\n');
        return;
    }

    for (auto it = nics.cbegin(); it != nics.cend(); it++) {
        if (it != nics.cbegin())
            fill_tc_left_char(12, ' ');

        if (*it == CtrlThread->configure_info.public_adapter)
            message_info(
                "[%d]* %s\n",
                std::distance(nics.cbegin(), it),
                it->c_str()
            );

        else
            message_info(
                "[%d]  %s\n",
                std::distance(nics.cbegin(), it),
                it->c_str()
            );
    }
}
