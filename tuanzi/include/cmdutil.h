#ifndef CMDUTIL_H_INCLUDED
#define CMDUTIL_H_INCLUDED

extern void exec_cmd(const char *cmd, char *buf, unsigned buflen);
[[gnu::format(printf, 1, 2)]]
extern void message_info(const char *format, ...);
extern void message_info(const std::string &str);
[[noreturn]] extern void display_usage();
extern unsigned short get_tc_width();
[[gnu::format(printf, 1, 2)]]
extern void rj_printf_debug(const char *format, ...);
extern void format_tc_string(
    unsigned short tc_width,
    unsigned indent_len,
    const std::string &str
);
extern void fill_tc_left_char(unsigned len, char c);
extern void print_separator(const char *s, unsigned len, bool print_crlf);
extern void print_string_list(
    const char *prefix,
    const std::vector<std::string> &slist
);
extern bool check_quit();
extern bool check_safe_exit(bool create_file);
extern bool is_run_background();
extern int set_termios(bool set_echo_icanon);
extern void shownotify(
    const std::string &content,
    const std::string &header,
    unsigned timeout
);
extern void show_url(
    const std::string &content,
    const std::string &header
);
extern void show_version();
extern void show_message_info();
extern void show_login_url();
extern void do_modify_password();
extern void dispatch_cmd(char cmd);
extern void display_help();
extern int modify_password_timeout(bool reset);
extern void show_sso_url();

#endif // CMDUTIL_H_INCLUDED
