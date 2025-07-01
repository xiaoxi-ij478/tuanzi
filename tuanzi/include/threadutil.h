#ifndef THREADUTIL_H_INCLUDED
#define THREADUTIL_H_INCLUDED

#include "waithandle.h"

extern int WaitForSingleObject(
    WAIT_HANDLE *event,
    unsigned long off_msec
);
extern int WaitForMultipleObjects(
    int event_count,
    WAIT_HANDLE *events,
    bool wait_all,
    unsigned long no_obj_waittime
);
extern void CloseHandle(WAIT_HANDLE *wait_handle);
extern void SetEvent(WAIT_HANDLE *wait_handle, bool broadcast);
extern bool TerminateThread(pthread_t thread_id);
extern bool PostThreadMessage(
    key_t thread_key,
    unsigned mtype,
    unsigned long arg1,
    unsigned long arg2
);
extern bool GPostThreadMessage(
    int msqid,
    unsigned mtype,
    unsigned long arg1,
    unsigned long arg2
);
extern bool post_command(unsigned char c);
extern void StopOcx();

#endif // THREADUTIL_H_INCLUDED
