#include <cassert>
#include <ctime>
#include <unistd.h>
#include <sys/time.h>
#include <sys/sysinfo.h>

#include "global.h"
#include "timeutil.h"

long GetTickCount()
{
    struct sysinfo sinfo = { 0 };

    for (int i = 0; i < 3; i++)
        if (!sysinfo(&sinfo))
            return 1000 * sinfo.uptime;

    return 0;
}

void Sleep(int msec)
{
    usleep(1000 * msec);
}

void GetAbsTime(struct timespec *tsp, unsigned long off_msec)
{
    // the original implementation uses gettimeofday & timeval
    // we use clock_gettime & timespec
    // struct timeval tv;
    for (int i = 0; i < 3; i++) {
        //      if (!gettimeofday(&tv, NULL)) {
        //          ts->tv_sec = tv.tv_sec + off_msec / 1000;
        //          ts->tv_nsec = tv.tv_usec * 1000 + off_msec % 1000;
        //      }
        if (!clock_gettime(CLOCK_REALTIME, tsp)) {
            tsp->tv_sec += off_msec / 1000;
            tsp->tv_nsec += 1000000 * (off_msec % 1000);
            return;
        }
    }

    g_logSystem.AppendText("GetAbsTime failed\n");
}

bool CreateCurrentUTC(long plus_sec, long off_sec, long long *result)
{
    assert(result);
    *result = plus_sec + (GetTickCount() - off_sec);
    return true;
}

void GetCurDataAndTime(char *dst)
{
    struct tm *timet = nullptr;
    time_t times = 0;
//    time(&times);
    times = time(nullptr);
    timet = localtime(&times);
    strftime(dst, 64, "%F %T ", timet);
}

void GetCurTime(char *dst)
{
    struct tm *timet = nullptr;
    time_t times = 0;
//    time(&times);
    times = time(nullptr);
    timet = localtime(&times);
    strftime(dst, 64, "%T ", timet);
}

unsigned long GetDayTime()
{
    struct timeval tv = { 0 };

    for (int i = 0; i < 3; i++)
        if (!gettimeofday(&tv, nullptr))
            return tv.tv_sec * 1000 + tv.tv_usec / 1000;

    return 0;
}

unsigned long GetElapseMiliSec(struct timeval tvp)
{
    struct timeval tvn = { 0 };
    gettimeofday(&tvn, nullptr);
    return (tvn.tv_sec - tvp.tv_sec) * 1000 + (tvn.tv_usec - tvp.tv_usec) / 1000;
}
