#ifndef CMDUTIL_H_INCLUDED
#define CMDUTIL_H_INCLUDED

[[maybe_unused]] void message_info(const char *format, ...);
[[maybe_unused]] void message_info(std::string str);
[[noreturn]] void display_usage();
unsigned short get_tc_width();
void rj_printf_debug(const char *format, ...);
void format_tc_string(
    unsigned short tc_width,
    unsigned int indent_len,
    const std::string &str
);
void fill_tc_left_char(int len, char c);
void print_separator(const char *s, int len, bool print_crlf);
void print_string_list(
    const char *prefix,
    const std::vector<std::string> &slist
);

#endif // CMDUTIL_H_INCLUDED
