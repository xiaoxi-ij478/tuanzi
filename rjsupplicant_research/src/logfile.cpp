#include "global.h"
#include "logfile.h"

CLogFile::CLogFile() : ofs(), prio(), ofs_orig_precision(ofs.precision())
{}

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

void CLogFile::HexPrinter(const unsigned char *arr, unsigned len)
{
    ofs << std::setprecision(2) << std::hex;
    std::for_each(arr, arr + len, [this](const unsigned char i) {
        ofs << i;
    });
    ofs << std::setprecision(ofs_orig_precision) << std::dec;
}

void CLogFile::LogToFile(
    const char *log_msg,
    const char *filename,
    bool print_time,
    bool print_crlf
)
{
    char time[304] = {};

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
    char s[2048] = {};
    vsnprintf(s, sizeof(s), format, va);
    ofs << s;
}

void CLogFile::WriteString(const char *str)
{
    ofs << str;
}
