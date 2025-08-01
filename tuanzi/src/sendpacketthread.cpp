#include "all.h"
#include "cmdutil.h"
#include "global.h"
#include "timeutil.h"
#include "mtypes.h"
#include "sendpacketthread.h"

CSendPacketThread::CSendPacketThread() :
    adapter_name(),
    pcap_handle(),
    started()
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

bool CSendPacketThread::SetSenderAdapter(const char *name)
{
    char ebuf[PCAP_ERRBUF_SIZE] = {};
    strcpy(adapter_name, name);
    rj_printf_debug("CSendPacketThread :: SetSenderAdapter()\n");

    if (!adapter_name[0]) {
        g_logSystem.AppendText("m_szAdapterName == NULL");
        return false;
    }

    if ((pcap_handle = pcap_open_live(adapter_name, 2000, 1, -1, ebuf)))
        return started = true;

    else
        g_logSystem.AppendText("pcap_open_live(): %s", ebuf);

    return false;
}

void CSendPacketThread::DispathMessage(struct LNXMSG *msg)
{
    switch (msg->mtype) {
        case SEND_MESSAGE_MTYPE:
            if (!started) {
                g_logSystem.AppendText("Send Packet before open nic");
                delete[] reinterpret_cast<char *>(msg->arg2);
                break;
            }

            if (
                !DoSendPacket(
                    reinterpret_cast<char *>(msg->arg2),
                    msg->arg1
                )
            )
                break;

            Sleep(1000);

            if (SetSenderAdapter(adapter_name)) {
                g_logSystem.AppendText("open nic[%s] success", adapter_name);
                DoSendPacket(
                    reinterpret_cast<char *>(msg->arg2),
                    msg->arg1
                );

            } else
                g_logSystem.AppendText("open nic[%s] failed again", adapter_name);

            break;

        case CLOSE_ADAPTER_MTYPE:
            started = false;
            CloseAdapter();
            break;
    }
}

int CSendPacketThread::DoSendPacket(const char *buf, unsigned buflen)
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

    if (
        !(
            ret = pcap_sendpacket(
                      pcap_handle,
                      reinterpret_cast<const unsigned char *>(buf),
                      buflen
                  )
        )
    ) {
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

bool CSendPacketThread::ExitSendPacketThread() const
{
    rj_printf_debug("ExitSendPacketThread\n");
    return !StopSendPacketThread();
}

int CSendPacketThread::StartSendPacketThread()
{
    rj_printf_debug("StartSendPacketThread\n");
    return StartThread();
}

bool CSendPacketThread::StopSendPacketThread() const
{
    rj_printf_debug("StopSendPacketThread\n");
    return PostThreadMessage(CLOSE_ADAPTER_MTYPE, 0, 0);
}

int CSendPacketThread::SendPacket(const char *buf, unsigned buflen)
{
    return DoSendPacket(buf, buflen);
}
