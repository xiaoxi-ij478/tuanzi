#ifndef SENDPACKETTHREAD_H_INCLUDED
#define SENDPACKETTHREAD_H_INCLUDED

#include "lnxthread.h"

class CSendPacketThread : public CLnxThread
{
    public:
        CSendPacketThread();
        ~CSendPacketThread() override;

        void CloseAdapter();
        int DoSendPacket(const char *buf, unsigned buflen);
        bool SetSenderAdapter(const char *name);
        int SendPacket(const char *buf, unsigned buflen);
        bool StopSendPacketThread() const;

    protected:
        void DispathMessage(struct LNXMSG *msg) override;

    private:
        bool ExitSendPacketThread() const;
        int StartSendPacketThread();

        char adapter_name[100];
        pcap_t *pcap_handle;
        pthread_mutex_t pthread_mutex2;
        bool started;
};

#endif // SENDPACKETTHREAD_H_INCLUDED
