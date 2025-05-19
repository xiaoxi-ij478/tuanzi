#ifndef TIMEUTIL_H_INCLUDED
#define TIMEUTIL_H_INCLUDED

extern void GetAbsTime(struct timespec *ts, unsigned long off_msec);
extern long GetTickCount();
// why would they even want to create such a function...
extern void Sleep(int msec);
// result's orig name: pCurUTC
extern bool CreateCurrentUTC(long plus_sec, long off_sec, long long *result);
extern void GetCurDataAndTime(char *dst);
extern void GetCurTime(char *dst);
extern void PrintCurTime();
extern unsigned long GetDayTime();
extern unsigned long GetElapseMiliSec(struct timeval tvp);

#endif // TIMEUTIL_H_INCLUDED
