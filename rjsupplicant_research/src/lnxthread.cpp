#include "global.h"
#include "timeutil.h"
#include "threadutil.h"
#include "util.h"
#include "lnxthread.h"

CLnxThread::CLnxThread(void *(*thread_func)(void *), void *thread_func_arg) :
    dont_know_always_false(),
    thread_running(),
    me(),
    thread_key(),
    msgid(-1),
    no_need_send_msg(),
    thread_func_arg(thread_func_arg),
    thread_func(thread_func),
    wait_handle1(),
    wait_handle2(),
    cur_msg(),
    timers(),
    pthread_mutex()
{
    pthread_mutex_init(&pthread_mutex, nullptr);
    memset(classname, 0, sizeof(classname));
}

CLnxThread::CLnxThread() : CLnxThread(nullptr, nullptr)
{}

CLnxThread::~CLnxThread()
{
    pthread_mutex_destroy(&pthread_mutex);
    CloseHandle(&wait_handle1);
}

int CLnxThread::CreateThread(
    pthread_attr_t *pthread_attr,
    bool no_need_send_msg
)
{
    int retval = 0;
    WAIT_HANDLE2 wait_handle;

    if (me)
        return -1;

    this->no_need_send_msg = no_need_send_msg;
    me = this;
    wait_handle.calling_thread = this;
    wait_handle.no_need_send_msg = no_need_send_msg;

    if ((retval = pthread_create(
                      &thread_key,
                      pthread_attr,
                      CLnxThread::_LnxThreadEntry,
                      &wait_handle
                  ))) {
        g_logSystem.AppendText("pthread_create error. retrun = %d", retval);
        return -1;
    }

    if ((retval = WaitForSingleObject(&wait_handle, 0))) {
        g_logSystem.AppendText("WaitForSingleObject error. retrun = %d", retval);
        return -1;
    }

    CloseHandle(&wait_handle);
    return 1;
}

int CLnxThread::GetMessageID() const
{
    return msgid;
}

bool CLnxThread::PostThreadMessage(long mtype, void *buf,
                                   unsigned long buflen) const
{
    struct LNXMSG msg = { mtype, buf, buflen };

    if (mtype < 0) {
        g_logSystem.AppendText(
            "%s PostThreadMessage error: msgid < 0", classname
        );
        return false;
    }

    if (msgsnd(msgid, &msg, LNXMSG_MSGSZ, IPC_NOWAIT) == -1) {
        g_logSystem.AppendText(
            "%s msgsnd %d Error:%s",
            classname, mtype, strerror(errno)
        );
        return false;
    }

    return true;
}

int CLnxThread::StartThread()
{
    if (no_need_send_msg)
        return -1;

    if (!PostThreadMessage(START_THREAD_MTYPE, nullptr, 0)) {
        g_logSystem.AppendText(
            "CLnxThread::StartThread() PostThreadMessage failed\n");
        return -1;
    }

    return WaitForSingleObject(&wait_handle2, 0);
}

int CLnxThread::StopThread()
{
    thread_running = false;

    if (no_need_send_msg)
        return 1;

    if (PostThreadMessage(STOP_THREAD_MTYPE, nullptr, 0))
        return 1;

    g_logSystem.AppendText(
        "CLnxThread::StopThread PostThreadMessage(%d) error",
        msgid
    );
    return 0;
}

// void CLnxThread::CommonConstruct()
// {
//    dont_know_always_false = false;
//    me = nullptr;
//    pthread = nullptr;
//    msgid = -1;
//    no_need_send_msg = false;
//    thread_running = false;
//    pthread_mutex_init(&pthread_mutex, nullptr);
//    memset(classname, 0, sizeof(classname));
// }

void CLnxThread::SetClassName(const char *name)
{
    strcpy(classname, name);
}

timer_t CLnxThread::SetTimer(int tflag, int off_msec)
{
    struct sigevent sev = {};
    struct TIMERPARAM *timer = nullptr;
    struct itimerspec new_time = {
        { off_msec / 1000, off_msec % 1000 },
        { off_msec / 1000, off_msec % 1000 }
    };
    timer_t timerid = nullptr;

    if (off_msec <= 0) {
        g_logSystem.AppendText("CLnxThread::SetTimernElapse <= 0\n");
        return nullptr;
    }

    timer = new struct TIMERPARAM;
    sev.sigev_notify = SIGEV_THREAD;
    sev.sigev_notify_function = &_OnTimerEntry;
    sev.sigev_value.sival_ptr = timer;
    timer->calling_thread = me;
    timer->tflag = tflag;
    timer->msqid = msgid;
    pthread_mutex_init(&timer->pthread_mutex, nullptr);

    if (my_timer_create(CLOCK_REALTIME, &sev, &timerid)) {
        g_logSystem.AppendText("timer_creat error.\n");
        return nullptr;
    }

    timer->ti = timerid;
    timers.push_back(timer);

    if (my_timer_settime(timerid, 0, &new_time, nullptr)) {
        g_logSystem.AppendText("timer_settime error.\n");
        return nullptr;
    }

    g_logFile_start.AppendText(
        "[%s] SetTimer(%d) ti = %d,msgid =%d",
        classname, tflag, timerid, msgid
    );
    return timerid;
}

timer_t CLnxThread::SetTimer(
    void *,
    int tflag,
    int off_msec,
    void (*)(union sigval)
)
{
    return SetTimer(tflag, off_msec);
}

bool CLnxThread::InitInstance()
{
    return true;
}

bool CLnxThread::Run()
{
    bool start_process = false;

    if (no_need_send_msg)
        return true;

    while (true) {
        if (msgrcv(msgid, &cur_msg, LNXMSG_MSGSZ, 0, MSG_NOERROR) == -1) {
            perror("msgrcv error");
            ExitInstance();
            return false;
        }

        switch (cur_msg.mtype) {
            case STOP_THREAD_MTYPE:
                cur_msg.mtype = 0;
                cur_msg.buf = nullptr;
                cur_msg.buflen = 0;
                g_logSystem.AppendText("CLnxThread::Run %s \tLEAVE", classname);
                return false;

            case START_THREAD_MTYPE:
                start_process = true;
                SetEvent(&wait_handle2, true);
                g_logFile_start.AppendText(
                    "(%s)msgrcv m_msgid =%d m_msgCur.message=%d ",
                    classname, msgid, cur_msg.mtype
                );
                DispathMessage(&cur_msg);
                break;

            default:
                g_logFile_start.AppendText(
                    "(%s)msgrcv m_msgid =%d m_msgCur.message=%d ",
                    classname, msgid, cur_msg.mtype
                );

                if (start_process)
                    DispathMessage(&cur_msg);

                break;
        }
    }

    return true;
}

bool CLnxThread::DispathMessage(struct LNXMSG * /* msg */)
{
    return true;
}

void CLnxThread::OnTimer(int tflag) const
{
    g_logSystem.AppendText(
        "CLnxThread::OnTimer pid =%d,timerFlag=%d\n",
        pthread_self(), tflag
    );
}

bool CLnxThread::OnTimerEnter(int tflag) const
{
    bool found = false;
    bool success = false;

    if (timers.empty()) {
        g_logFile_start.AppendText("[%s] OnTimerEnter no found.", classname);
        return false;
    }

    for (TIMERPARAM *timer : timers) {
        int retval = 0;

        if (timer->tflag != tflag)
            continue;

        found = true;
        retval = pthread_mutex_trylock(&timer->pthread_mutex);

        if (retval == EBUSY || retval == EDEADLOCK)
            g_logFile_start.AppendText("[%s] OnTimerEnter busy", classname);

        else
            success = true;
    }

    if (!found) {
        g_logFile_start.AppendText("[%s] OnTimerEnter no found.", classname);
        return false;
    }

    return success;
}

void CLnxThread::OnTimerLeave(int tflag) const
{
    for (TIMERPARAM *timer : timers) {
        if (timer->tflag != tflag)
            return;

        pthread_mutex_unlock(&timer->pthread_mutex);
    }
}

bool CLnxThread::ExitInstance()
{
    return no_need_send_msg ? 0 : cur_msg.buf;
}

void CLnxThread::KillTimer(timer_t &timerid)
{
    g_logFile_start.AppendText(
        "CLnxThread(%s)::KillTimer() %u ,pid =%u\n",
        classname, timerid, pthread_self()
    );

    if (!timerid)
        return;

    pthread_mutex_lock(&pthread_mutex);

    if (timers.empty())
        return;

    for (TIMERPARAM *timer : timers) {
        if (timer->ti != timerid)
            return;

        g_logFile_start.AppendText(
            "[%s] KillTimer(%d) ti = %d,tflag =%d",
            classname, timer->tflag, timerid, timer->tflag
        );

        if (my_timer_delete(timerid) == -1) {
            g_logFile_start.AppendText(
                "Error-CLnxThread::KillTimer %s,errno=%d,EINVAL=%d",
                strerror(errno), errno, EINVAL
            );
            Sleep(100);

            if (my_timer_delete(timerid) == -1 && errno == EINVAL) {
                timer->ti = 0;
                timerid = 0;
            }

        } else {
            timer->ti = 0;
            timerid = 0;
            g_logFile_start.AppendText("CLnxThread(%s)::KillTimer() OK", classname);
        }
    }

    pthread_mutex_unlock(&pthread_mutex);
}

void CLnxThread::KillAllTimer()
{
    g_logFile_start.AppendText(
        "CLnxThread(%s)::KillAllTimer() size=%d",
        classname, timers.size()
    );
    pthread_mutex_lock(&pthread_mutex);

    for (TIMERPARAM *timer : timers) {
        g_logFile_start.AppendText(
            "[%s] KillAllTimer(%d) ti = %d,tflag =%d",
            classname, timer->tflag, timer->ti, timer->tflag
        );
        pthread_mutex_destroy(&timer->pthread_mutex);

        if (timer->ti)
            my_timer_delete(timer->ti);

        delete timer;
    }

    timers.clear();
    pthread_mutex_unlock(&pthread_mutex);
}

void CLnxThread::LnxEndThread()
{
    KillAllTimer();

    if (msgid >= 0 && msgctl(msgid, IPC_RMID, nullptr))
        perror("msgctl error ");

    thread_running = false;
    CloseHandle(&wait_handle2);
    g_logFile_start.AppendText(
        "CLnxThread::LnxEndThread %s \tCloseHandle (m_exitEvent)",
        classname);
    SetEvent(&wait_handle1, 1);

    if (false) // if (dont_know_always_false)
        this->~CLnxThread();
}

void CLnxThread::SafeExitThread(unsigned int off_msec)
{
    int retval = 0;

    //    if (!this) // ???????
    //        return;

    if (!StopThread()) {
        g_logSystem.AppendText("CLnxThread::StopThread %s failed.", classname);

        if (!TerminateThread(thread_key))
            this->~CLnxThread();

    } else { //if (!dont_know_always_false) { // == if (1)
        if (!(retval = WaitForSingleObject(&wait_handle1, off_msec)))
            this->~CLnxThread();

        g_logSystem.AppendText(
            "CLnxThread::StopThread %s WaitForSingleObject error = %d",
            classname, retval
        );

        if (!TerminateThread(thread_key))
            this->~CLnxThread();
    }
}

void *CLnxThread::_LnxThreadEntry(void *arg)
{
    int msgid = 0;
    WAIT_HANDLE2 *wait_handle = static_cast<WAIT_HANDLE2 *>(arg);
    CLnxThread *calling_thread = wait_handle->calling_thread;

    if (pthread_setcanceltype(PTHREAD_CANCEL_DISABLE, nullptr))
        g_logSystem.AppendText("CLnxThread::_LnxThreadEntry pthread_setcanceltype error.");

    if (wait_handle->no_need_send_msg)
        goto start_exec;

    msgid = msgget(calling_thread->thread_key, 0666 | IPC_CREAT | IPC_EXCL);

    if (msgid != -1)
        goto start_exec;

    if (errno != EEXIST) {
        g_logSystem.AppendText(
            "ENOSPC=%d,ENOMEM=%d,EACCES=%d,EEXIST=%d,ENOENT=%dn errno=%d\n",
            ENOSPC, ENOMEM, EACCES, EEXIST, ENOENT, errno
        );

        if (errno == ENOSPC) {
            g_logSystem.AppendText(
                "limit for the maximum number of message queues (MSGMNI) would be exceeded\n");

        } else if (errno == ENOMEM) {
            g_logSystem.AppendText(
                " the system does not have enough memory for the new data structure.\n");
        }

        g_logSystem.AppendText("msgget error:%s", strerror(errno));
        SetEvent(wait_handle, false);
        calling_thread->LnxEndThread();
        g_logSystem.AppendText(
            "CLnxThread::_LnxThreadEntry Create Thread Error!");
        return nullptr;
    }

    g_logSystem.AppendText("--------------------------creat msg id error,del it first\n");
    msgid = msgget(calling_thread->thread_key, 0666);
    calling_thread->msgid = msgid;

    if (msgid == -1 ||
            msgctl(msgid, IPC_RMID, nullptr) == -1 ||
            (
                calling_thread->msgid =
                    msgget(calling_thread->thread_key, 0666 | IPC_CREAT | IPC_EXCL)
            ) == -1
       ) {
        g_logSystem.AppendText("msgget error:%s", strerror(errno));
        SetEvent(wait_handle, false);
        calling_thread->LnxEndThread();
        g_logSystem.AppendText(
            "CLnxThread::_LnxThreadEntry Create Thread Error!");
        return nullptr;
    }

start_exec:
    SetEvent(wait_handle, false);
    calling_thread->thread_running = true;

    if (calling_thread->thread_func)
        calling_thread->thread_func(calling_thread->thread_func_arg);

    else if (calling_thread->InitInstance())
        calling_thread->Run();

    else
        calling_thread->ExitInstance();

    calling_thread->LnxEndThread();
    return nullptr;
}

void CLnxThread::_OnTimerEntry(union sigval arg)
{
    TIMERPARAM *timer = static_cast<TIMERPARAM *>(arg.sival_ptr);
    CLnxThread *calling_thread = timer->calling_thread;

    if (!calling_thread)
        return g_logFile_start.AppendText(
                   "[%s] _OnTimerEntry_,timerParam1=%p,return",
                   "_OnTimerEntry",
                   timer);

    g_logFile_start.AppendText(
        "[%s] OnTimer(%d)",
        calling_thread->classname,
        timer->tflag
    );

    if (timer->tflag)
        calling_thread->OnTimer(timer->tflag);
}
