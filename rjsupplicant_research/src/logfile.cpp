#include <algorithm>
#include <fstream>
#include <string>
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <sys/time.h>

#include "global.h"
#include "logfile.h"

void CLogFile::CreateLogFile_S(const std::string &filename, int prio)
{
    CreateLogFile(filename.c_str(), prio);
}

void CLogFile::CreateLogFile(const char *filename, int prio)
{
    ofs.open(filename, std::ios::app);
    this->prio = prio;
}

void CLogFile::GetTimeString(char *dst)
{
    struct tm *times = nullptr;
    time_t timer = 0;

    if (time(&timer) == -1)
        strcpy(dst, "00-00-00 00:00:00 ");

    else {
        times = localtime(&timer);
        strftime(dst, 64, "%F %T ", times);
    }
}

void CLogFile::HexPrinter(const unsigned char *arr, unsigned int len)
{
    std::ios::fmtflags orig_f = ofs.flags(std::ios::hex);
    std::streamsize orig_p = ofs.precision(2);
    std::for_each(arr, arr + len, [this](const unsigned char i) {
        ofs << i;
    });
    ofs.flags(orig_f);
    ofs.precision(orig_p);
}

void CLogFile::LogToFile(
    const char *log_msg,
    const char *filename,
    bool print_time,
    bool print_crlf
)
{
    char time[304] = { 0 };

    if (!(log_msg && *log_msg && filename && *filename))
        return;

    std::ofstream ofs(filename, std::ios::app);

    if (!ofs)
        return;

    CLogFile::GetTimeString(time);

    if (print_time)
        ofs << time;

    ofs << log_msg;

    if (print_crlf)
        ofs << std::endl;

    ofs.flush();
    ofs.close();
}

void CLogFile::AppendText(const char *format, ...)
{
    va_list va;
    va_start(va, format);
    AppendText_V(format, va);
    va_end(va);
}

void CLogFile::AppendText_V(const char *format, va_list va)
{
    char s[2048] = { 0 };
    vsnprintf(s, sizeof(s), format, va);
    ofs << s;
}

void CLogFile::WriteString(const char *str)
{
    ofs << str;
}
