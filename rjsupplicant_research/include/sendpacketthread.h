#ifndef SENDPACKETTHREAD_H_INCLUDED
#define SENDPACKETTHREAD_H_INCLUDED

#define SEND_MESSAGE_MTYPE 0x7D2
#define CLOSE_ADAPTER_MTYPE 0x7D3

#include "lnxthread.h"
#include "pcap.h"

class CSendPacketThread : public CLnxThread
{
    public:
        CSendPacketThread();
        virtual ~CSendPacketThread() override;

        void CloseAdapter();
        bool SetSenderAdapter(char *name);

    protected:
        bool DispathMessage(struct LNXMSG *msg) override;

    private:
        int DoSendPacket(const unsigned char *buf, int buflen);
        bool ExitSendPacketThread();
        int StartSendPacketThread();
        bool StopSendPacketThread();
        int SendPacket(const unsigned char *buf, int buflen);

        char adapter_name[100];
        pcap_t *pcap_handle;
        pthread_mutex_t pthread_mutex2;
        bool started;
};

#endif // SENDPACKETTHREAD_H_INCLUDED
