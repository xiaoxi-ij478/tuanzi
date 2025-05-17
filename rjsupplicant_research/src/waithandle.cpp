#include "global.h"
#include "waithandle.h"

WAIT_HANDLE::WAIT_HANDLE() : finished(), pthread_cond(), pthread_mutex()
{
    pthread_mutexattr_t t;
    int val;

    if ((val = pthread_cond_init(&pthread_cond, NULL)))
        g_logSystem.AppendText("pthread_cond_init error. retrun = %d", val);

    else {
        pthread_mutexattr_init(&t);
        pthread_mutexattr_settype(&t, PTHREAD_MUTEX_RECURSIVE);

        if ((val = pthread_mutex_init(&pthread_mutex, &t)))
            g_logSystem.AppendText("pthread_mutex_init error. retrun = %d", val);

        else
            finished = false;
    }
}

WAIT_HANDLE::~WAIT_HANDLE()
{
    finished = false;
}

WAIT_HANDLE2::WAIT_HANDLE2() : no_need_send_msg(), calling_thread()
{}
