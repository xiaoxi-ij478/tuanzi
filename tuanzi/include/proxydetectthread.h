#ifndef PROXYDETECTTHREAD_H
#define PROXYDETECTTHREAD_H

#include "lnxthread.h"
#include "isproser.h"

class CProxyDetectThread : public CLnxThread
{
    public:
        CProxyDetectThread();
        ~CProxyDetectThread() override;

    protected:
        bool InitInstance() override;
        bool DispathMessage(struct LNXMSG *msg) override;
        void OnTimer(int tflag) const override;
        bool ExitInstance() override;

    private:
        bool GetFakeInfo(in_addr_t *ipaddr, struct ether_addr *macaddr) const;
        bool StartDetect(
            const char *adapter_name_l,
            key_t thread_key_l,
            unsigned mtype_l,
            int kind_l,
            char *errbuf
        );
        bool StopDetect();

        CIsProSer isproser;
        char adapter_name[512];
        struct ether_addr hostmac;
        unsigned field_568;
        key_t thread_key;
        unsigned mtype;
        unsigned kind;
        bool started;
        timer_t timerid;
};

#endif // PROXYDETECTTHREAD_H
