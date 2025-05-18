#include "global.h"
#include "changelanguage.h"
#include "cmdutil.h"

[[maybe_unused]] void message_info(const char *format, ...)
{
    va_list arg;
    va_start(arg, format);
    vprintf(format, arg);
    va_end(arg);
}

[[maybe_unused]] void message_info(std::string str)
{
    std::cout << str;
}

[[noreturn]] void display_usage()
{
#define PRINT_USAGE(head, help_str_id) \
    do { \
        std::cout << head; \
        format_tc_string(tc_width, 24, cinstance.LoadString(help_str_id)); \
        std::cout << std::endl; \
    } while(0)
    unsigned short tc_width = get_tc_width();
    char str2[2048] = {};
    CChangeLanguage &cinstance = CChangeLanguage::Instance();
    std::cout << cinstance.LoadString(2007) << std::endl;
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
    std::cout << "\t   --comments\t";
    std::string log_path = g_strAppPath + "log/run.log";
    snprintf(
        str2, sizeof(str2),
        cinstance.LoadString(2045).c_str(),
        log_path.c_str()
    );
    format_tc_string(tc_width, 24, str2);
#undef PRINT_USAGE
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
    unsigned int indent_len,
    const std::string &str
)
{
    // i wrote this function myself since I
    // can't read what the heck they are...
    unsigned int actual_line_width = tc_width - indent_len;
    unsigned int remain = str.length();
    unsigned int actual_write = 0;
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
    format_tc_string(12, 0, std::string(prefix));
    std::cout << CChangeLanguage::Instance().LoadString(2059)
              << '[' << slist.size() << ']' << std::endl;

    for (unsigned int i = 0; i < slist.size(); i++) {
        fill_tc_left_char(12, ' ');
        std::cout << '[' << i << "] " << slist[i] << std::endl;
    }
}

bool check_quit()
{
    char c = 0;
    std::cout << CChangeLanguage::Instance().LoadString(2039);
    c = std::cin.get();
    std::cout << std::endl;
    return c == '\n';
}

void check_safe_exit(bool create_file)
{
    std::string lockfile(g_strAppPath);
    std::ofstream ofs;
    lockfile.append(".rgsusfexit");
    rj_printf_debug("%s strFile=%s\n", __func__, lockfile.c_str());

    if (!create_file) {
        if (unlink(lockfile.c_str()) == -1)
            rj_printf_debug(
                "%s strFile=%s exist and unlink failed \n",
                __func__,
                lockfile.c_str()
            );

        else
            rj_printf_debug(
                "%s strFile=%s exist and unlink ok \n",
                __func__,
                lockfile.c_str()
            );

        return;
    }

    if (!access(lockfile.c_str(), F_OK)) {
        rj_printf_debug("%s strFile=%s exist\n", lockfile.c_str());
        return;
    }

    ofs.open(lockfile.c_str());

    if (!ofs) {
        rj_printf_debug(
            "%s strFile=%s no exist and create failed \n",
            __func__,
            lockfile.c_str()
        );
        return;
    }

    ofs.close();
    rj_printf_debug(
        "%s strFile=%s no exist and create ok 2\n",
        __func__,
        lockfile.c_str()
    );
}

bool is_run_background()
{
    int forep = tcgetpgrp(0);
    int myp = getpgrp();

    if (forep == -1) {
        rj_printf_debug("tcgetpgrp STDIN_FILENO(%d) error:%s", 0, strerror(errno));

        if ((forep = tcgetpgrp(1)) == -1) {
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

    if (tcgetattr(0, &term) == -1) {
        rj_printf_debug("set_termios: tcgetattr error:%s", strerror(errno));
        return -1;
    }

    if (set_echo_icanon)
        term.c_lflag |= ICANON | ECHO;

    else
        term.c_lflag &= ~(ICANON | ECHO);

    if (tcsetattr(0, TCSANOW, &term) == -1)
        rj_printf_debug("tcsetattr error:%s", strerror(errno));

    return 0;
}
