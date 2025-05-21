#ifndef THREADUTIL_H_INCLUDED
#define THREADUTIL_H_INCLUDED

#include "waithandle.h"

extern int WaitForSingleObject(
    WAIT_HANDLE *event,
    unsigned long off_msec
);
[[maybe_unused]] extern int WaitForMultipleObjects(
    [[maybe_unused]] int event_count,
    [[maybe_unused]] WAIT_HANDLE *events,
    [[maybe_unused]] bool wait_all,
    [[maybe_unused]] unsigned long no_obj_waittime
);
extern void CloseHandle(WAIT_HANDLE *wait_handle);
extern void SetEvent(WAIT_HANDLE *wait_handle, bool broadcast);
extern bool TerminateThread(pthread_t thread_key);
extern bool PostThreadMessage(
    pthread_t thread_key,
    unsigned mtype,
    void *buf,
    unsigned long buflen
);
extern bool GPostThreadMessage(
    int msqid,
    unsigned mtype,
    void *buf,
    unsigned long buflen
);

#endif // THREADUTIL_H_INCLUDED
