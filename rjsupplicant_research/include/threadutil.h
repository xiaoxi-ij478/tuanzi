#ifndef THREADUTIL_H_INCLUDED
#define THREADUTIL_H_INCLUDED


#include "waithandle.h"

int WaitForSingleObject(
    WAIT_HANDLE *event,
    unsigned long off_msec
);
[[maybe_unused]] int WaitForMultipleObjects(
    int /* event_count */,
    WAIT_HANDLE * /* events */,
    bool /* wait_all */,
    unsigned long /* no_obj_waittime */
);
void CloseHandle(WAIT_HANDLE *wait_handle);
void SetEvent(WAIT_HANDLE *wait_handle, bool broadcast);
bool TerminateThread(pthread_t thread_key);
bool PostThreadMessage(pthread_t thread_key, unsigned int mtype, void *buf,
                       unsigned long buflen);
bool GPostThreadMessage(int msqid, unsigned int mtype, void *buf,
                        unsigned long buflen);

#endif // THREADUTIL_H_INCLUDED
