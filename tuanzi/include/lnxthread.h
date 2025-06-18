#ifndef LNXTHREAD_H_INCLUDED
#define LNXTHREAD_H_INCLUDED

#include "waithandle.h"

#define STOP_THREAD_MTYPE 0x456
#define START_THREAD_MTYPE 0x457
#define ON_TIMER_MTYPE 0x1E61

class CLnxThread;

struct LNXMSG {
    unsigned long mtype;
    unsigned long buflen;
    void *buf;
};

#define LNXMSG_MSGSZ (sizeof(struct LNXMSG) - offsetof(struct LNXMSG, buflen))

struct TIMERPARAM {
    int tflag;
    int msqid;
    timer_t ti;
    pthread_mutex_t pthread_mutex;
    CLnxThread *calling_thread;
};

class CLnxThread
{
    public:
        CLnxThread(void *(*thread_func)(void *), void *thread_func_arg);
        CLnxThread();
        virtual ~CLnxThread();

        int CreateThread(pthread_attr_t *pthread_attr, bool no_need_send_msg_l);
        int GetMessageID() const;
        bool PostThreadMessage(
            unsigned long mtype,
            unsigned long buflen,
            void *buf
        ) const;
        int StartThread();
        int StopThread();

    protected:
        // void CommonConstruct();
        void SetClassName(const char *name);
        timer_t SetTimer(int tflag, int off_msec);
        timer_t SetTimer(
            void *,
            int tflag,
            int off_msec,
            void (*)(union sigval)
        );
        void SafeExitThread(unsigned off_msec);

        virtual bool InitInstance();
        virtual bool Run();
        virtual bool DispathMessage(struct LNXMSG *msg);
        virtual bool OnTimerEnter(int tflag) const;
        virtual void OnTimerLeave(int tflag) const;
        virtual void OnTimer(int tflag) const;
        virtual bool ExitInstance();
        virtual bool KillTimer(timer_t &timerid);

        bool dont_know_always_false;
        pthread_t thread_id;

    private:
        void KillAllTimer();
        // the original implementation set it as a static method with
        // an object pointer argument
        // but here we set it as a member function
        // static void LnxEndThread(CLnxThread *objp);
        void LnxEndThread();

        static void *_LnxThreadEntry(void * /* WAIT_HANDLE2 * */ arg);
        static void _OnTimerEntry(union sigval /* TIMERPARAM @ sival_ptr */ arg);

        bool thread_running;
        CLnxThread *me;
        WAIT_HANDLE wait_handle1;
        char classname[128];
        int msgid; // or msqid... ?
        bool no_need_send_msg;
        WAIT_HANDLE wait_handle2;
        void *thread_func_arg;
        void *(*thread_func)(void *);
        struct LNXMSG cur_msg;
        std::vector<struct TIMERPARAM *> timers;
        pthread_mutex_t pthread_mutex;
};

#endif // LNXTHREAD_H_INCLUDED
