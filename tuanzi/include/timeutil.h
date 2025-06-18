#ifndef TIMEUTIL_H_INCLUDED
#define TIMEUTIL_H_INCLUDED

extern void GetAbsTime(struct timespec *ts, unsigned long off_msec);
extern unsigned long GetTickCount();
// why would they even want to create such a function...
extern void Sleep(int msec);
// result's orig name: pCurUTC
extern bool CreateCurrentUTC(
    long long plus_sec,
    long long off_msec,
    long long *result
);
extern void GetCurDataAndTime(char *dst);
extern void GetCurTime(char *dst);
extern void PrintCurTime();
extern unsigned long GetDayTime();
extern unsigned long GetElapseMiliSec(struct timeval tvp);

#endif // TIMEUTIL_H_INCLUDED
