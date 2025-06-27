#ifndef ADAPTERDETECTTHREAD_H_INCLUDED
#define ADAPTERDETECTTHREAD_H_INCLUDED

#include "global.h"
#include "lnxthread.h"

#define MAX_NIC_NAME_LEN 0x200

struct DetectNICInfo {
    char nic_name[MAX_NIC_NAME_LEN];
    in_addr_t ipaddr;
    struct ether_addr macaddr;
    key_t thread_key;
    int msgid;
    bool disallow_multi_nic_ip;
};

class CAdapterDetectThread : public CLnxThread
{
    public:
        CAdapterDetectThread();
        ~CAdapterDetectThread() override;

        bool StartDetect(
            const char *nic_name_l,
            const struct ether_addr *macaddr_l,
            in_addr_t ipaddr_l,
            key_t thread_key_l,
            int msgid_l,
            bool disallow_multi_nic_ip_l,
            char *errmsg
        ) const;
        bool StopDetect(unsigned flag) const;

    protected:
        bool InitInstance() override;
        void DispathMessage(struct LNXMSG *msg) override;
        void OnTimer(int tflag) const override;
        bool ExitInstance() override;

    private:
        void MultipleAdaptesOrIPCheck() const;
        DECLARE_DISPATH_MESSAGE_HANDLER(OnStartDetect);
        DECLARE_DISPATH_MESSAGE_HANDLER(OnStopDetect);
        DECLARE_DISPATH_MESSAGE_HANDLER(OnTimer); // called by DispathMessage
        void adapter_state_check();

        char nic_name[MAX_NIC_NAME_LEN];
        key_t control_thread_key;
        int control_thread_msgid;
        in_addr_t ipaddr;
        struct ether_addr macaddr;
        bool disallow_multi_nic_ip;
        timer_t proxy_detect_timerid;
        timer_t nic_state_detect_timerid;
        int socket_fd;
        enum ADAPTER_STATUS status;
};

#endif // ADAPTERDETECTTHREAD_H_INCLUDED
