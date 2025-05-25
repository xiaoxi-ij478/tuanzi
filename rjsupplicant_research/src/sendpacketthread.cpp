#include "cmdutil.h"
#include "global.h"
#include "timeutil.h"
#include "sendpacketthread.h"

CSendPacketThread::CSendPacketThread() :
    CLnxThread(),
    pcap_handle(), started(), adapter_name()
{
    pthread_mutexattr_t mutexattr;
    pthread_mutexattr_init(&mutexattr);
    pthread_mutexattr_settype(&mutexattr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&pthread_mutex2, &mutexattr);
    SetClassName("CSendPacketThread");
}

CSendPacketThread::~CSendPacketThread()
{
    CloseAdapter();
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
                delete[] msg->buf;
                break;
            }

            if (!DoSendPacket(static_cast<unsigned char *>(msg->buf), msg->buflen))
                break;

            Sleep(1000);

            if (SetSenderAdapter(adapter_name)) {
                g_logSystem.AppendText("open nic[%s] success", adapter_name);
                DoSendPacket(static_cast<unsigned char *>(msg->buf), msg->buflen);

            } else
                g_logSystem.AppendText("open nic[%s] failed again", adapter_name);

            break;

        case CLOSE_ADAPTER_MTYPE:
            started = false;
            CloseAdapter();
            break;
    }

    // the original implementation didn't provide a return value
    // so we always return false
    return false;
}

int CSendPacketThread::DoSendPacket(const unsigned char *buf, int buflen)
{
    int ret = 0;

    if (!pcap_handle || buflen <= 0) {
        g_logSystem.AppendText(
            "CSendPacketThread :: DoSendPacket error.m_pAdapter=%x buflen=%d",
            pcap_handle,
            buflen
        );
        return -2;
    }

    if (pthread_mutex_trylock(&pthread_mutex2) == EBUSY) {
        rj_printf_debug("DoSendPacket EBUSY\n");
        pthread_mutex_lock(&pthread_mutex2);
        rj_printf_debug("after pthread_mutex_lock\n");
    }

    if (!(ret = pcap_sendpacket(pcap_handle, buf, buflen))) {
        pthread_mutex_unlock(&pthread_mutex2);
        return 0;
    }

    g_logSystem.AppendText(
        "Send Packet Error:%s code=%d",
        pcap_geterr(pcap_handle),
        ret
    );
    pthread_mutex_unlock(&pthread_mutex2);
    return -1;
}

bool CSendPacketThread::ExitSendPacketThread()
{
    rj_printf_debug("ExitSendPacketThread\n");
    return !StopSendPacketThread();
}

int CSendPacketThread::StartSendPacketThread()
{
    rj_printf_debug("StartSendPacketThread\n");
    return StartThread();
}

bool CSendPacketThread::StopSendPacketThread()
{
    rj_printf_debug("StopSendPacketThread\n");
    return PostThreadMessage(CLOSE_ADAPTER_MTYPE, 0, nullptr);
}

int CSendPacketThread::SendPacket(const unsigned char *buf, int buflen)
{
    return DoSendPacket(buf, buflen);
}
