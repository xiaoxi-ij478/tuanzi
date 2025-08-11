#ifndef TIMEUTIL_H_INCLUDED
#define TIMEUTIL_H_INCLUDED

extern void GetAbsTime(struct timespec *ts, unsigned long off_msec);
extern unsigned long GetTickCount();
// why would they even want to create such a function...
extern void Sleep(int msec);
extern bool CreateCurrentUTC(
    unsigned long plus_sec,
    unsigned long off_msec,
    unsigned long *result // pCurUTC
);
extern void GetCurDataAndTime(char *dst);
extern void GetCurTime(char *dst);
extern void PrintCurTime();
extern long GetDayTime();
extern unsigned long GetElapseMiliSec(struct timeval tvp);

#endif // TIMEUTIL_H_INCLUDED
