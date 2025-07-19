#ifndef RXPACKETTHREAD_H_INCLUDED
#define RXPACKETTHREAD_H_INCLUDED

#include "lnxthread.h"

struct CRxPacketThread_msgids {
    int direct_msgid;
    int main_msgid;
    int proxy_msgid;
};

class CRxPacketThread : public CLnxThread
{
    public:
        CRxPacketThread();
        ~CRxPacketThread() override;

        void CloseAdapter();
        void ExitRxPacketThread();
        bool InitAdapter();
        void SetAdapterMode(unsigned adapter_mode_l);
        void SetDirectMsgID(int direct_msgid);
        void SetMainMsgID(int main_msgid);
        void SetPacketFilter(const char *filter_expr);
        void SetProxyMsgID(int proxy_msgid);
        void SetRxPacketAdapter(const char *adapter_name_l);
        int StartRecvPacketThread();
        int StopRxPacketThread();

    protected:
        void DispathMessage(struct LNXMSG *msg) override;

    private:
        DECLARE_DISPATH_MESSAGE_HANDLER(StartRecvPacket);

        static void RecvPacketCallBack(
            unsigned char *user,
            const struct pcap_pkthdr *h,
            const unsigned char *bytes
        );

        struct CRxPacketThread_msgids msgids;
        char adapter_name[100];
        bool stopped;
        pcap_t *pcap_handle;
        unsigned adapter_mode;
        WAIT_HANDLE recv_packet_waithandle;
};

#endif // RXPACKETTHREAD_H_INCLUDED
