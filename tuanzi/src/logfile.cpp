#include "all.h"
#include "global.h"
#include "logfile.h"

CLogFile::CLogFile() : log_filename(), prio()
{}

void CLogFile::CreateLogFile_S(const std::string &filename, int prio_l)
{
    CreateLogFile(filename.c_str(), prio_l);
}

void CLogFile::CreateLogFile(const char *filename, int prio_l)
{
    strcpy(log_filename, filename);
    prio = prio_l;
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

void CLogFile::HexPrinter(const char *arr, unsigned len)
{
    char *s = new char[len * 2];

    for (unsigned i = 0; i < len; i++)
        sprintf(s + i * 2, "%02x", arr[i]);

    WriteString(s);
    delete[] s;
}

void CLogFile::LogToFile(
    const char *log_msg,
    const char *filename,
    bool print_time,
    bool print_crlf
)
{
    char time[304] = {};

    if (!log_msg || !*log_msg || !filename || !*filename)
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
    char s[2048] = {};
    vsnprintf(s, sizeof(s), format, va);
    WriteString(s);
}

void CLogFile::WriteString(const char *str)
{
    LogToFile(str, log_filename, true, true);
}
