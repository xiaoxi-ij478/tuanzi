#include "all.h"
#include "global.h"
#include "timeutil.h"

unsigned long GetTickCount()
{
    struct sysinfo sinfo = {};

    for (int i = 0; i < 3; i++)
        if (!sysinfo(&sinfo))
            return 1000 * sinfo.uptime;

    return 0;
}

void Sleep(int msec)
{
    usleep(1000 * msec);
}

void GetAbsTime(struct timespec *ts, unsigned long off_msec)
{
    // the original implementation uses gettimeofday & timeval
    // we use clock_gettime & timespec
    // struct timeval tv;
    for (int i = 0; i < 3; i++) {
//      if (!gettimeofday(&tv, nullptr)) {
//          ts->tv_sec = tv.tv_sec + off_msec / 1000;
//          ts->tv_nsec = tv.tv_usec * 1000 + off_msec % 1000;
//      }
        if (clock_gettime(CLOCK_REALTIME, ts))
            continue;

        ts->tv_sec += off_msec / 1000;
        ts->tv_nsec += 1000000 * (off_msec % 1000);
        return;
    }

    g_logSystem.AppendText("GetAbsTime failed\n");
}

bool CreateCurrentUTC(
    unsigned long plus_sec,
    unsigned long off_msec,
    unsigned long *result
)
{
    assert(result);
    *result = plus_sec + (GetTickCount() - off_msec) / 1000;
    return true;
}

void GetCurDataAndTime(char *dst)
{
    time_t times =
        std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    strftime(dst, 64, "%T ", localtime(&times));
}

void PrintCurTime()
{
    char dst[64] = {};
    GetCurDataAndTime(dst);
    std::cout << dst;
}

void GetCurTime(char *dst)
{
    time_t times =
        std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    strftime(dst, 64, "%T ", localtime(&times));
}

long GetDayTime()
{
    return
        std::chrono::milliseconds(
            std::chrono::system_clock::now().time_since_epoch()
        ).count();
}

unsigned long GetElapseMiliSec(struct timeval tvp)
{
    struct timeval tvn = {};
    gettimeofday(&tvn, nullptr);
    return (tvn.tv_sec - tvp.tv_sec) * 1000 + (tvn.tv_usec - tvp.tv_usec) / 1000;
}
