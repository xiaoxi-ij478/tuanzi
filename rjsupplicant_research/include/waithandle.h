#ifndef WAITHANDLE_H
#define WAITHANDLE_H

// we must not #include lnxthread.h since CLnxThread include WAIT_HANDLE
// as its member, so we must be included AFTER lnxthread.h
class CLnxThread;

struct WAIT_HANDLE {
    WAIT_HANDLE();
    ~WAIT_HANDLE();
    bool finished;
    pthread_cond_t pthread_cond;
    pthread_mutex_t pthread_mutex;
};

struct WAIT_HANDLE2 : public WAIT_HANDLE {
    using WAIT_HANDLE::WAIT_HANDLE;

    bool no_need_send_msg;
    CLnxThread *calling_thread;
};

#endif // WAITHANDLE_H
