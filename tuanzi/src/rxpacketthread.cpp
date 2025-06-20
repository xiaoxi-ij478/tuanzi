#include "all.h"
#include "cmdutil.h"
#include "netutil.h"
#include "timeutil.h"
#include "threadutil.h"
#include "udplistenthread.h" // RECV_PACKET_RETURN_MTYPE
#include "rxpacketthread.h"

CRxPacketThread::CRxPacketThread() :
    msgids(-1, -1, -1),
    adapter_name(),
    stopped(true),
    pcap_handle(),
    adapter_mode(1),
    recv_packet_waithandle()
{
    SetClassName("CRxPacketThread");
}

CRxPacketThread::~CRxPacketThread()
{
    stopped = true;

    if (pcap_handle)
        pcap_close(pcap_handle);
}

bool CRxPacketThread::DispathMessage(struct LNXMSG *msg)
{
    if (msg->mtype == START_THREAD_MTYPE)
        StartRecvPacket();
}

void CRxPacketThread::CloseAdapter()
{
    if (!pcap_handle)
        return;

    pcap_breakloop(pcap_handle);
    pcap_close(pcap_handle);
    pcap_handle = nullptr;
}

void CRxPacketThread::ExitRxPacketThread()
{
    rj_printf_debug("ExitRxPacketThread\n");
    StopRxPacketThread();
    CloseHandle(&recv_packet_waithandle);
}

bool CRxPacketThread::InitAdapter()
{
    char errbuf[256] = {};

    if (
        pcap_handle =
            pcap_open_live(adapter_name, 2000, adapter_mode, 1000, errbuf)
    ) {
        SetPacketFilter("ip or arp or ether proto 0x888e");
        return true;
    }

    g_logSystem.AppendText("m_pAdapter NULL error =%s.\n", errbuf);
    return false;
}

void CRxPacketThread::SetAdapterMode(unsigned adapter_mode_l) const
{
    adapter_mode = adapter_mode_l;
}

void CRxPacketThread::SetDirectMsgID(int direct_msgid) const
{
    msgids.direct_msgid = direct_msgid;
}

void CRxPacketThread::SetPacketFilter(const char *filter_expr) const
{
    struct bpf_program filter = {};
    rj_printf_debug("SetPacketFilter =%s\n", filter_expr);

    if (pcap_compile(pcap_handle, &filter, filter_expr, true, 0)) {
        rj_printf_debug("pcap_compile error %s\n", pcap_geterr(pcap_handle));
        pcap_close(pcap_handle);
        pthread_exit(0);
    }

    if (pcap_setfilter(pcap_handle, &filter)) {
        rj_printf_debug("pcap_setfilter error %s\n", pcap_geterr(pcap_handle));
        pcap_close(pcap_handle);
        pcap_freecode();
        pthread_exit(0);
    }

    pcap_freecode();
}

void CRxPacketThread::SetProxyMsgID(int proxy_msgid)
{
    this->msgids.proxy_msgid = a2;
}

void CRxPacketThread::SetMainMsgID(int main_msgid) const
{
    this->msgids.main_msgid = a2;
}

void CRxPacketThread::SetRxPacketAdapter(const char *adapter_name_l)
{
    memset(adapter_name, 0, sizeof(adapter_name));
    strcpy(adapter_name, adapter_name_l);
}

void CRxPacketThread::StartRecvPacket() const
{
    if (!InitAdapter())
        return;

    stopped = false;

    while (!stopped) {
        while (!pcap_handle) {
            Sleep(1000);
            InitAdapter();

            if (stopped)
                goto finish;
        }

        if (
            pcap_dispatch(
                pcap_handle,
                1,
                CRxPacketThread::RecvPacketCallBack,
                &msgids
            ) == -1
        ) {
            g_logSystem.AppendText(
                "Recv Packet Error:%s",
                pcap_geterr(pcap_handle)
            );
            CloseAdapter();
            Sleep(1000);
            InitAdapter();
        }
    }

finish:
    stopped = true;
    CloseAdapter();
    SetEvent(recv_packet_waithandle, true);
}

int CRxPacketThread::StartRecvPacketThread() const
{
    return StartThread();
}

int CRxPacketThread::StopRxPacketThread() const
{
    int ret = 0;
    rj_printf_debug("StopRxPacketThread\n");

    if (stopped)
        return ret;

    stopped = true;

    if ((ret = WaitForSingleObject(recv_packet_waithandle, 2000)) == ETIMEDOUT) {
        rj_printf_debug("StopRxPacketThread WAIT_TIMEOUT\n");
        g_logSystem.AppendText(
            "StopRxPacketThread WaitForSingleObject timeout =%d",
            ETIMEDOUT
        );
    }

    return ret;
}

void CRxPacketThread::RecvPacketCallBack(
    unsigned char *user,
    const struct pcap_pkthdr *h,
    const unsigned char *bytes
)
{
    struct CRxPacketThread_msgids *msgids =
            reinterpret_cast<struct CRxPacketThread_msgids *>(user);
    struct etherudppkg *pkg = nullptr;
    struct etherudppkg *pkg2 = nullptr;
    unsigned char *pkg_char = nullptr;
    unsigned alloc_size = 0;

    if (h->caplen < sizeof(struct ether_header))
        return;

    alloc_size = MAX(h->caplen, 1999);
    pkg = reinterpret_cast<struct etherudppkg *>
          (pkg_char = new unsigned char[alloc_size]);
    memcpy(pkg, bytes, alloc_size);

    if (pkg->etherheader.ether_type == htons(ETH_P_PAE)) {
        if (
            !IsLoopBack(
                reinterpret_cast<struct ether_addr *>(pkg->etherheader.ether_shost)
            )
        ) {
            g_Logoff.HexPrinter(pkg_char, alloc_size);

            if (
                !GPostThreadMessage(
                    msgids->main_msgid,
                    RECV_PAE_PACKET_MTYPE,
                    alloc_size,
                    pkg
                )
            )
                delete[] pkg;
        }

    } else {
        if (msgids->proxy_msgid != -1) {
            pkg2 = reinterpret_cast<struct etherudppkg *>
                   (new unsigned char[alloc_size]);
            memcpy(pkg2, bytes, alloc_size);

            if (
                !GPostThreadMessage(
                    msgids->proxy_msgid,
                    RECV_PACKET_RETURN_MTYPE,
                    alloc_size,
                    pkg2
                )
            )
                delete[] pkg2;
        }

        if (
            pkg->etherheader.ether_type == htons(ETHERTYPE_IP) &&
            pkg->ipheader.protocol == IPPROTO_UDP &&
            msgids->direct_msgid != -1 &&
            !IsLoopBack(
                reinterpret_cast<struct ether_addr *>(pkg->etherheader.ether_shost)
            ) &&
            !IsHostDstMac(
                reinterpret_cast<struct ether_addr *>(pkg->etherheader.ether_dhost)
            )
        )
            if (
                !GPostThreadMessage(
                    msgids->direct_msgid,
                    RECV_PACKET_RETURN_MTYPE,
                    alloc_size,
                    pkg
                )
            )
                delete[] pkg;
    }
}
