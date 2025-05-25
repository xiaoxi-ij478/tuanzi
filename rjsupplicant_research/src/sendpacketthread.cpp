#include "cmdutil.h"
#include "global.h"
#include "sendpacketthread.h"

CSendPacketThread::CSendPacketThread() :
    CLnxThread(),
    pcap_handle(), started(), adapter_name()
{
    pthread_mutexattr_t mutexattr;
    pthread_mutexattr_init(&mutexattr);
    pthread_mutexattr_settype(&mutexattr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&pthread_mutex, &mutexattr);
    SetClassName("CSendPacketThread");
}

CSendPacketThread::~CSendPacketThread()
{
    if (pcap_handle)
        pcap_close(pcap_handle);
}

void CSendPacketThread::CloseAdapter()
{
    if (!pcap_handle)
        return;

    pcap_close(pcap_handle);
    pcap_handle = nullptr;
}

bool CSendPacketThread::SetSenderAdapter(char *name)
{
    char ebuf[256] = {};
    strcpy(adapter_name, name);
    rj_printf_debug("CSendPacketThread :: SetSenderAdapter()\n");

    if (!adapter_name[0]) {
        g_logSystem.AppendText("m_szAdapterName == NULL");
        return 0;
    }

    if ((pcap_handle = pcap_open_live(adapter_name, 2000, 1, 0xFFFFFFFF, ebuf)))
        return started = true;

    else
        g_logSystem.AppendText("pcap_open_live(): %s", ebuf);

    return 0;
}

bool CSendPacketThread::DispathMessage(struct LNXMSG *msg)
{
    switch (msg->mtype) {
        case SEND_MESSAGE_MTYPE:
            if (!started) {
                g_logSystem.AppendText("Send Packet before open nic");

                if (msg->buf)
                    delete[] msg->buf;
            }
    }
}

int CSendPacketThread::DoSendPacket(const unsigned char *buf, int buflen)
{
}

bool CSendPacketThread::ExitSendPacketThread()
{
}

int CSendPacketThread::StartSendPacketThread()
{
}

bool CSendPacketThread::StopSendPacketThread()
{
}

int CSendPacketThread::SendPacket(const unsigned char *buf, int buflen)
{
}
