#include "all.h"
#include "cmdutil.h"
#include "timeutil.h"
#include "util.h"
#include "waithandle.h"
#include "global.h"
#include "miscdefs.h"
#include "mtypes.h"
#include "threadutil.h"

int WaitForSingleObject(
    WAIT_HANDLE *wait_handle,
    unsigned long off_msec
)
{
    int ret = 0;
    struct timespec ts = {};

    if (wait_handle->signal) {
        wait_handle->signal = false;
        return 0;
    }

    ret = pthread_mutex_trylock(&wait_handle->pthread_mutex);

    if (ret == EBUSY)
        return ret;

    if (ret) {
        rj_printf_debug("pthread_mutex_lock error,code =%s\n", strerror(errno));
        return ret;
    }

    if (off_msec) {
        GetAbsTime(&ts, off_msec);
        ret = 1;

        while (!wait_handle->signal && ret)
            ret = pthread_cond_timedwait(
                      &wait_handle->pthread_cond,
                      &wait_handle->pthread_mutex,
                      &ts
                  );

    } else
        while (!wait_handle->signal)
            ret = pthread_cond_wait(
                      &wait_handle->pthread_cond,
                      &wait_handle->pthread_mutex
                  );

    wait_handle->signal = false;

    if (pthread_mutex_unlock(&wait_handle->pthread_mutex))
        rj_printf_debug("pthread_mutex_unlock error,code =%s\n", strerror(errno));

    return ret;
}

int WaitForMultipleObjects(
    [[maybe_unused]] int event_count,
    [[maybe_unused]] WAIT_HANDLE *events,
    [[maybe_unused]] bool wait_all,
    [[maybe_unused]] unsigned long no_obj_waittime
)
{
    // It is not used anyway, and I don't want to figure what the heck
    // it does any more. So just left here for now.
    return 0;
//    unsigned long TickCount = 0;
//    unsigned i = 0, j = 0;
//    int wait_result = -1;
//    bool *wait_flags = nullptr;
//    WAIT_HANDLE *cevents = events;
//    assert(events);
//    assert(event_count < 0); // if event_count < 0 then new will throw error
//
//    if (no_obj_waittime) {
//        if (!wait_all) {
//            TickCount = GetTickCount();
//
//            do
//                if (no_obj_waittime <= GetTickCount() - TickCount)
//                    return 110;
//
//            while (!event_count);
//
//            cevents = events;
//            i = 0;
//
//            while (WaitForSingleObject(*cevents++, 5)) {
//                if (++i != event_count)
//                    continue;
//
//                if (no_obj_waittime > GetTickCount() - TickCount)
//                    do
//                        if (no_obj_waittime <= GetTickCount() - TickCount)
//                            break;
//
//                    while (!event_count);
//
//                return 110;
//            }
//
//            return i;
//
//        } else {
//            wait_flags = new bool[event_count];
//
//            if (event_count)
//                memset(wait_flags, 0, sizeof(bool)*event_count);
//
//            for (i = 0; i <= event_count; ++i)
//                *wait_flags++ = 0;
//
//            wait_result = -1;
//            TickCount = GetTickCount();
//            i = 0;
//
//            do
//                {
//                    if (no_obj_waittime <= GetTickCount() - TickCount) {
//                        delete[] wait_flags;
//                        return 110;
//                    }
//                }
//
//            while (!event_count);
//
//            cevents = events;
//            j = 0;
//
//            while (1) {
//                if (!*wait_flags++ && !WaitForSingleObject(*cevents++, 5)) {
//                    i++;
//                    *wait_flags = 1;
//                }
//
//                if (i == event_count)
//                    break;
//
//                if (++j == event_count)
//                    do
//                        if (no_obj_waittime <= GetTickCount() - TickCount) {
//                            delete[] wait_flags;
//                            return 110;
//                        }
//
//                    while (!event_count);
//            }
//
//            delete[] wait_flags;
//        }
//
//    } else { // !no_obj_waittime
//        if (!wait_all) {
//            while (!event_count)
//                ;
//
//            cevents = events;
//            i = 0;
//
//            while (WaitForSingleObject(*cevents++, 5))
//                if (++i == event_count)
//                    while (!event_count)
//                        ;
//
//            return i;
//        }
//
//        wait_flags = new bool[event_count];
//
//        if (event_count)
//            memset(wait_flags, 0, sizeof(bool)*event_count);
//
//        wait_result = -1;
//        i = 0;
//
//        while (!event_count)
//            ;
//
//        cevents = events;
//
//        while (!*wait_flags) {
//            if (WaitForSingleObject(*cevents++, 5)) {
//                if (i == event_count)
//                    break;
//
//            } else {
//                *wait_flags++ = 1;
//
//                if (++i == event_count)
//                    break;
//
//                if (++j == event_count)
//                    while (!event_count)
//                        ;
//            }
//        }
//
//        delete[] wait_flags;
//    }
//
//    return wait_result;
}

void CloseHandle(WAIT_HANDLE *wait_handle)
{
    int ret = 0;

    if (pthread_mutex_trylock(&wait_handle->pthread_mutex) == EBUSY) {
        rj_printf_debug("pthread_mutex_trylock busy\n");

        if ((ret = pthread_cond_broadcast(&wait_handle->pthread_cond)))
            rj_printf_debug("pthread_cond_signal error %d\n", ret);

        wait_handle->signal = true;

    } else {
        if ((ret = pthread_cond_broadcast(&wait_handle->pthread_cond)))
            rj_printf_debug("pthread_cond_signal error %d\n", ret);

        wait_handle->signal = true;

        if ((ret = pthread_mutex_unlock(&wait_handle->pthread_mutex)))
            rj_printf_debug("pthread_mutex_unlock nResult=%d\n", ret);
    }

    pthread_mutex_destroy(&wait_handle->pthread_mutex);
    pthread_cond_destroy(&wait_handle->pthread_cond);
}

void SetEvent(WAIT_HANDLE *wait_handle, bool broadcast)
{
    int ret = 0;
    pthread_mutex_lock(&wait_handle->pthread_mutex);

    if (broadcast)
        ret = pthread_cond_broadcast(&wait_handle->pthread_cond);

    else
        ret = pthread_cond_signal(&wait_handle->pthread_cond);

    if (ret)
        rj_printf_debug("pthread_cond_signal error %d\n", ret);

    wait_handle->signal = true;
    pthread_mutex_unlock(&wait_handle->pthread_mutex);
}

bool TerminateThread(pthread_t thread_id)
{
    int retval = 0;
    int msqid = 0;
    void *thread_return = nullptr;

    if ((retval = pthread_cancel(thread_id))) {
        g_logSystem.AppendText("pthread_cancel Error:%d\n", retval);
        return retval;
    }

    if ((retval = pthread_join(thread_id, &thread_return))) {
        g_logSystem.AppendText("pthread_join ret:%d error:%s", retval, strerror(errno));
        return retval;
    }

    msqid =
        msgget(
            thread_id,
            S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH
        );

    if (msqid == -1)
        return retval;

    if (msgctl(msqid, IPC_RMID, nullptr))
        g_logSystem.AppendText("msgctl error:%s", strerror(errno));

    return retval;
}

bool PostThreadMessage(
    key_t thread_key,
    long mtype,
    unsigned long arg1,
    unsigned long arg2
)
{
    int msqid =
        msgget(
            thread_key,
            S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH
        );
    g_logFile_start.AppendText(
        "::PostThreadMessage idThread = %d,Msg=%d",
        thread_key,
        mtype
    );
    return msqid >= 0 ? GPostThreadMessage(msqid, mtype, arg1, arg2) : false;
}

bool GPostThreadMessage(
    int msqid,
    long mtype,
    unsigned long arg1,
    unsigned long arg2
)
{
    struct LNXMSG msg = { mtype, arg1, arg2 };
    int ret = 0;

    if (msqid < 0) {
        rj_printf_debug("message id is null,return\n");
        return false;
    }

    ret = msgsnd(msqid, &msg, LNXMSG_MSGSZ, IPC_NOWAIT);

    if (ret == -1)
        rj_printf_debug("msgsnd Error:%s\n", strerror(errno));

    return ret != -1;
}

void StopOcx()
{
    PostThreadMessage(theApp.thread_key, STOP_OCX_MTYPE, 0, 0);
}
