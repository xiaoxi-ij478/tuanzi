#ifndef CMDUTIL_H_INCLUDED
#define CMDUTIL_H_INCLUDED

#include <string>

//void message_info(const char *format, ...);
[[noreturn]] void display_usage();
unsigned short get_tc_width();
void rj_printf_debug(const char *format, ...);
void format_tc_string(
    unsigned short tc_width,
    unsigned int indent_len,
    const std::string &str
);
void fill_tc_left_char(int len, char c);

#endif // CMDUTIL_H_INCLUDED
