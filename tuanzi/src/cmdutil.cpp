#include "all.h"
#include "global.h"
#include "timeutil.h"
#include "msgutil.h"
#include "changelanguage.h"
#include "cmdutil.h"

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

void fill_tc_left_char(int len, char c)
{
    if (len <= 0)
        return;

    for (int i = 0; i < len; i++)
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

    for (unsigned i = 0; i < slist.size(); i++) {
        fill_tc_left_char(12, ' ');
        std::cout << '[' << i << "] " << slist[i] << std::endl;
    }
}

bool check_quit()
{
    char c = 0;
    message_info(CChangeLanguage::Instance().LoadString(2039));
    c = std::cin.get();
    message_info("\n");
    return c == '\n';
}

void check_safe_exit(bool create_file)
{
    std::string lockfile(g_strAppPath);
    std::ofstream ofs;
    lockfile.append(".rgsusfexit");
    rj_printf_debug("%s strFile=%s\n", "check_safe_exit", lockfile.c_str());

    if (!create_file) {
        if (unlink(lockfile.c_str()) == -1)
            rj_printf_debug(
                "%s strFile=%s exist and unlink failed \n",
                "check_safe_exit",
                lockfile.c_str()
            );

        else
            rj_printf_debug(
                "%s strFile=%s exist and unlink ok \n",
                "check_safe_exit",
                lockfile.c_str()
            );

        return;
    }

    if (!access(lockfile.c_str(), F_OK)) {
        rj_printf_debug("%s strFile=%s exist\n", "check_safe_exit", lockfile.c_str());
        return;
    }

    ofs.open(lockfile.c_str());

    if (!ofs) {
        rj_printf_debug(
            "%s strFile=%s no exist and create failed \n",
            "check_safe_exit",
            lockfile.c_str()
        );
        return;
    }

    ofs.close();
    rj_printf_debug(
        "%s strFile=%s no exist and create ok 2\n",
        "check_safe_exit",
        lockfile.c_str()
    );
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

void show_version()
{
    format_tc_string(12, 0, CChangeLanguage::Instance().LoadString(2003));
    message_info(CChangeLanguage::Instance().LoadString(2004) + '\n');
}

void show_message_info()
{
    std::vector<struct tagMsgItem> msgs;
    GetMsgArray(msgs);
    print_msg_item_header();
    std::for_each(msgs.cbegin(), msgs.cend(), print_msg_item);
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

void do_modify_password()
{
    std::string old_password, new_password_first, new_password_second;

    if (
        !CPasswordModifier::GetPasswordSecurityInfo()->enable_modify_pw ||
        !theApp.IsOnline()
    )
        return;

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
}

void dispatch_cmd(char cmd)
{
    switch (cmd) {
        case '?':
        case 'h':
            display_help();
            break;

        case 'a':
            show_all_info();
            break;

        case 'm':
            show_message_info();
            break;

        case 'n':
            show_connect_net_info();
            break;

        case 'o':
            show_sso_url();
            break;

        case 'p':
            do_modify_password();
            break;

        case 'q':
            check_quit();
            break;

        case 's':
            do_switch_service();
            break;

        case 't':
            show_connect_time();
            break;

        case 'u':
            show_connect_user_info();
            break;

        case 'v':
            show_version();
            break;

        default:
            break;
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
