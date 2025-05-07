#ifndef TIMEUTIL_H_INCLUDED
#define TIMEUTIL_H_INCLUDED


void GetAbsTime(struct timespec *ts, unsigned long off_msec);
long GetTickCount();
// why would they even want to create such a function...
void Sleep(int msec);
// result's orig name: pCurUTC
bool CreateCurrentUTC(long plus_sec, long off_sec, long long *result);
void GetCurDataAndTime(char *dst);
void GetCurTime(char *dst);
void PrintCurTime();
unsigned long GetDayTime();
unsigned long GetElapseMiliSec(struct timeval tvp);

#endif // TIMEUTIL_H_INCLUDED
