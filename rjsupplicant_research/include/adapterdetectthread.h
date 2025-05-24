#ifndef ADAPTERDETECTTHREAD_H_INCLUDED
#define ADAPTERDETECTTHREAD_H_INCLUDED

#include "global.h"
#include "lnxthread.h"

#define START_DETECT_MTYPE 0x1
#define STOP_DETECT_MTYPE 0x2
#define ON_TIMER_MTYPE 0x1E61

#define PROXY_DETECT_TIMER_MTYPE 0x71
#define NIC_STATE_DETECT_TIMER_MTYPE 0x72

#define MAC_CHANGED_MTYPE 0x1E
#define IP_CHANGED_MTYPE 0x1F
#define MULTIPLE_ADAPTER_MTYPE 0x14
#define MULTIPLE_IP_MTYPE 0x15

#define STOP_PROXY_DETECT_TIMER_FLAG 0x1
#define STOP_NIC_STATE_DETECT_TIMER_FLAG 0x2

#define MAX_NIC_NAME_LEN 0x200

#define ADAPTER_UP_REPORT_MTYPE ADAPTER_UP
#define ADAPTER_DOWN_REPORT_MTYPE ADAPTER_DOWN
#define ADAPTER_DISABLE_REPORT_MTYPE ADAPTER_DISABLE
#define ADAPTER_ENABLE_REPORT_MTYPE ADAPTER_ENABLE
#define ADAPTER_ERROR_REPORT_MTYPE ADAPTER_ERROR

struct DetectNICInfo {
    char nic_name[MAX_NIC_NAME_LEN];
    struct in_addr ipaddr;
    unsigned char macaddr[6];
    pthread_t thread_key;
    int msgid;
    bool disallow_multi_nic_ip;
};

class CAdapterDetectThread : public CLnxThread
{
    public:
        CAdapterDetectThread();
        virtual ~CAdapterDetectThread() override;

        bool StartDetect(
            const char *nic_name,
            const unsigned char macaddr[6],
            struct in_addr ipaddr,
            pthread_t thread_key,
            int msgid,
            bool disallow_multi_nic_ip,
            char *errmsg
        ) const;
        bool StopDetect(unsigned flag) const;

    protected:
        bool InitInstance() override;
        bool DispathMessage(struct LNXMSG *msg) override;
        void OnTimer(int tflag) const override;
        bool ExitInstance() override;

    private:
        void MultipleAdaptesOrIPCheck() const;
        void OnStartDetect(void *arg);
        void OnStopDetect(void *arg);
        void OnTimer(void *arg); // called by DispathMessage
        void adapter_state_check();

        char nic_name[MAX_NIC_NAME_LEN];
        pthread_t control_thread_key;
        int control_thread_msgid;
        struct in_addr ipaddr;
        unsigned char macaddr[6];
        bool disallow_multi_nic_ip;
        timer_t proxy_detect_timerid;
        timer_t nic_state_detect_timerid;
        int socket_fd;
        enum ADAPTER_STATUS status;
};

#endif // ADAPTERDETECTTHREAD_H_INCLUDED
