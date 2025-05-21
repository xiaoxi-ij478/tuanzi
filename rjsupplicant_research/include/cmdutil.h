#ifndef CMDUTIL_H_INCLUDED
#define CMDUTIL_H_INCLUDED

[[maybe_unused, gnu::format(printf, 1, 2)]]
extern void message_info(const char *format, ...);
[[maybe_unused]] extern void message_info(std::string str);
[[noreturn]] extern void display_usage();
extern unsigned short get_tc_width();
[[gnu::format(printf, 1, 2)]]
extern void rj_printf_debug(const char *format, ...);
extern void format_tc_string(
    unsigned short tc_width,
    unsigned indent_len,
    const std::string &str
);
extern void fill_tc_left_char(int len, char c);
extern void print_separator(const char *s, int len, bool print_crlf);
extern void print_string_list(
    const char *prefix,
    const std::vector<std::string> &slist
);
extern bool check_quit();
extern void check_safe_exit(bool create_file);
extern bool is_run_background();

#endif // CMDUTIL_H_INCLUDED
