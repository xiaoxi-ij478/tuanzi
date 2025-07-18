#ifndef SENDPACKETTHREAD_H_INCLUDED
#define SENDPACKETTHREAD_H_INCLUDED

#include "lnxthread.h"

class CSendPacketThread : public CLnxThread
{
    public:
        CSendPacketThread();
        ~CSendPacketThread() override;

        void CloseAdapter();
        bool SetSenderAdapter(char *name);

    protected:
        void DispathMessage(struct LNXMSG *msg) override;

    private:
        int DoSendPacket(unsigned buflen, char *buf);
        bool ExitSendPacketThread() const;
        int StartSendPacketThread();
        bool StopSendPacketThread() const;
        int SendPacket(unsigned buflen, char *buf);

        char adapter_name[100];
        pcap_t *pcap_handle;
        pthread_mutex_t pthread_mutex2;
        bool started;
};

#endif // SENDPACKETTHREAD_H_INCLUDED
