#ifndef THREADUTIL_H_INCLUDED
#define THREADUTIL_H_INCLUDED

class WAIT_HANDLE;

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
    long mtype,
    unsigned long arg1,
    unsigned long arg2
);
extern bool GPostThreadMessage(
    int msqid,
    long mtype,
    unsigned long arg1,
    unsigned long arg2
);
extern void StopOcx();

#endif // THREADUTIL_H_INCLUDED
