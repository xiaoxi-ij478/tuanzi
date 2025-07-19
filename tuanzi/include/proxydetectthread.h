#ifndef PROXYDETECTTHREAD_H_INCLUDED
#define PROXYDETECTTHREAD_H_INCLUDED

#include "lnxthread.h"
#include "isproser.h"

class CProxyDetectThread : public CLnxThread
{
    public:
        CProxyDetectThread();
        ~CProxyDetectThread() override;

        bool StartDetect(
            const char *adapter_name_l,
            key_t thread_key_l,
            unsigned mtype_l,
            int kind_l,
            char *errbuf
        );
        bool StopDetect();

    protected:
        bool InitInstance() override;
        void DispathMessage(struct LNXMSG *msg) override;
        void OnTimer(int tflag) override;
        bool ExitInstance() override;

    private:
        bool GetFakeInfo(in_addr_t *ipaddr, struct ether_addr *macaddr) const;

        CIsProSer isproser;
        char adapter_name[512];
        struct ether_addr hostmac;
        in_addr_t ipaddr;
        key_t thread_key;
        unsigned mtype;
        unsigned kind;
        bool started;
        timer_t timerid;
};

#endif // PROXYDETECTTHREAD_H_INCLUDED
