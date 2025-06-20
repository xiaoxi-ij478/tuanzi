#ifndef WAITHANDLE_H_INCLUDED
#define WAITHANDLE_H_INCLUDED

// we must not include lnxthread.h since CLnxThread include WAIT_HANDLE
// as its member, so we must be included AFTER lnxthread.h
class CLnxThread;

class WAIT_HANDLE
{
    public:
        WAIT_HANDLE();
        ~WAIT_HANDLE();

        bool signal;
        pthread_cond_t pthread_cond;
        pthread_mutex_t pthread_mutex;
};

class WAIT_HANDLE2 : public WAIT_HANDLE
{
    public:
        WAIT_HANDLE2();

        bool no_need_send_msg;
        CLnxThread *calling_thread;
};

#endif // WAITHANDLE_H_INCLUDED
