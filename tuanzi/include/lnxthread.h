#ifndef LNXTHREAD_H_INCLUDED
#define LNXTHREAD_H_INCLUDED

#include "waithandle.h"
#include "miscdefs.h"
#include "timer.h"

struct TIMERPARAM {
    int tflag;
    timer t;
    std::mutex mutex;
    CLnxThread *calling_thread;
};

struct WAIT_HANDLE2 : WAIT_HANDLE {
    bool no_need_send_msg;
    CLnxThread *calling_thread;
};

// helper macro for writing DispathMessage() handlers
#define HANDLE_MTYPE(mtype, func) \
    case (mtype): (func)(msg->arg1, msg->arg2); break

#define DECLARE_DISPATH_MESSAGE_HANDLER(name) \
    void name(unsigned long arg1, unsigned long arg2)

#define DEFINE_DISPATH_MESSAGE_HANDLER(name, class_name) \
    void class_name::name(unsigned long arg1, unsigned long arg2)

class CLnxThread
{
    public:
        CLnxThread();
        virtual ~CLnxThread();

        int CreateThread(pthread_attr_t *pthread_attr, bool no_need_send_msg_l);
        int GetMessageID() const;
        bool PostThreadMessage(
            long mtype,
            unsigned long arg1,
            unsigned long arg2
        ) const;
        void SafeExitThread(unsigned off_msec);
        int StartThread();
        int StopThread();

    protected:
        void SetClassName(const char *name);
        timer_t SetTimer(int tflag, int off_msec);

        virtual bool InitInstance() = 0;
        virtual bool Run();
        virtual void DispathMessage() = 0;
        virtual bool OnTimerEnter(int tflag) const;
        virtual void OnTimerLeave(int tflag) const;
        virtual void OnTimer(int tflag);
        virtual bool ExitInstance() = 0;
        virtual bool KillTimer(timer_t &timerid);

    private:
        void KillAllTimer();
        // the original implementation set it as a static method with
        // an object pointer argument
        // but here we set it as a member function
        // static void LnxEndThread(CLnxThread *objp);
        void LnxEndThread();

        static void *_LnxThreadEntry(void * /* WAIT_HANDLE2 * */ arg);
        static void _OnTimerEntry(union sigval /* TIMERPARAM @ sival_ptr */ arg);


    public:
        bool doing_upgrade;
        std::thread thread_obj;

    protected:
        message_queue msg_queue;

    private:
        bool thread_inited;
        bool thread_running;
        struct WAIT_HANDLE thread_end_wait_handle;
        std::string classname;
        bool no_need_send_msg;
        struct WAIT_HANDLE start_thread_wait_handle;
        std::vector<struct TIMERPARAM> timers;
        std::mutex mutex_obj;
};

#endif // LNXTHREAD_H_INCLUDED
