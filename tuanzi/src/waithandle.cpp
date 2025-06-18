#include "all.h"
#include "global.h"
#include "waithandle.h"

WAIT_HANDLE::WAIT_HANDLE() : signal(), pthread_cond(), pthread_mutex()
{
    pthread_mutexattr_t mutexattr;
    int val;

    if ((val = pthread_cond_init(&pthread_cond, NULL)))
        g_logSystem.AppendText("pthread_cond_init error. retrun = %d", val);

    else {
        pthread_mutexattr_init(&mutexattr);
        pthread_mutexattr_settype(&mutexattr, PTHREAD_MUTEX_RECURSIVE);

        if ((val = pthread_mutex_init(&pthread_mutex, &mutexattr)))
            g_logSystem.AppendText("pthread_mutex_init error. retrun = %d", val);

        else
            signal = false;
    }
}

WAIT_HANDLE::~WAIT_HANDLE()
{
    signal = false;
}

WAIT_HANDLE2::WAIT_HANDLE2() : no_need_send_msg(), calling_thread()
{}
