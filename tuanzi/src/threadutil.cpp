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
    struct WAIT_HANDLE &wait_handle,
    unsigned long off_msec
)
{
    int ret = 0;

    if (wait_handle.signal) {
        wait_handle.signal = false;
        return 0;
    }

    std::unique_lock l(wait_handle.mutex, std::try_to_lock);

    if (!l)
        return 1;

    if (off_msec)
        while (
            !wait_handle.signal &&
            wait_handle.condition.wait_for(
                l,
                std::chrono::milliseconds(off_msec)
            ) == std::cv_status::timeout
        );

    else
        while (!wait_handle.signal)
            wait_handle.condition.wait(l);

    wait_handle.signal = false;
    return ret;
}

int WaitForMultipleObjects(
    [[maybe_unused]] int event_count,
    [[maybe_unused]] struct WAIT_HANDLE *events,
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
//    struct WAIT_HANDLE *cevents = events;
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

void CloseHandle(struct WAIT_HANDLE &wait_handle)
{
    std::unique_lock l(wait_handle.mutex, std::try_to_lock);
    wait_handle.condition.notify_all();
    wait_handle.signal = true;
}

void SetEvent(struct WAIT_HANDLE &wait_handle, bool broadcast)
{
    std::lock_guard l(wait_handle.mutex);

    if (broadcast)
        wait_handle.condition.notify_all();

    else
        wait_handle.condition.notify_one();

    wait_handle.signal = true;
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
