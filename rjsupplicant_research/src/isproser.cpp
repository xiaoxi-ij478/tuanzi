#include "global.h"
#include "stdpkgs.h"
#include "isproser.h"

CIsProSer::CIsProSer() :
    kind(),
    host_addr_set(),
    adapter_name(),
    hostmac(),
    check_status(),
    detected_fakemac(),
    detected_fakeipaddr(),
    updated_times(),
    last_updated_time(),
    tcpip_info(),
    clienttcp_next(),
    clienttcp_prev(),
    sertcp_next(),
    sertcp_prev()
{}

CIsProSer::~CIsProSer()
{
    while (sertcp_next)
        DelFromHRList(sertcp_next);

    while (clienttcp_next)
        DelFromRHList(clienttcp_next);
}

int CIsProSer::Detect(unsigned char *pkg, unsigned int pkglen)
{
    struct ether_header *etherheader =
            reinterpret_cast<struct ether_header *>(pkg);
    struct iphdr *ipheader =
            reinterpret_cast<struct iphdr *>(pkg + sizeof(struct ether_header));
    struct tcphdr *tcpheader = nullptr;

    if (!host_addr_set) {
        g_logFile_proxy.AppendText("CIsProSer::Detect no set host addr\r\n");
        return -1;
    }

    if (
        !pkg ||
        pkglen <= sizeof(struct ether_header) + sizeof(struct iphdr) ||
        ntohs(etherheader->ether_type) != ETHERTYPE_IP
    )
        return 0;

    tcpip_info.etherheader = etherheader;
    tcpip_info.ipheader = ipheader;

    if (ipheader->ihl << 2 < sizeof(struct iphdr)) {
        g_logFile_proxy.AppendText("IP Head Len min than 20\r\n");
        return 0;
    }

    if (kind & 1 && IsFakeMac(etherheader, ipheader))
        return 12;

    if (tcpip_info.ipheader->protocol != IPPROTO_TCP)
        return 0;

    if (
        ntohs(tcpip_info.ipheader->tot_len) <
        (ipheader->ihl << 2) + sizeof(struct tcphdr)
    ) {
        g_logFile_proxy.AppendText(
            "nIPTotalLen:%d is too small(realIPHdrLen:%d),it's ip headr error",
            ntohs(tcpip_info.ipheader->tot_len),
            ipheader->ihl << 2
        );
        return 0;
    }

    if (pkglen < ntohs(tcpip_info.ipheader->tot_len) + sizeof(struct ether_addr)) {
        g_logFile_proxy.AppendText(
            "CIsProSer::Detect bufLen:%d,ETHdrLen+nIPTotalLen:%d\r\n",
            pkglen,
            ntohs(tcpip_info.ipheader->tot_len) + sizeof(struct ether_addr)
        );
        return 0;
    }

    tcpip_info.tcpheader =
        tcpheader =
            reinterpret_cast<struct tcphdr *>(
                pkg +
                sizeof(struct ether_header) +
                (ipheader->ihl << 2)
            );

    if (
        ntohs(tcpip_info.ipheader->tot_len) ==
        tcpheader->doff + (ipheader->ihl << 2) &&
        tcpheader->th_flags != (TH_SYN | TH_ACK) &&
        tcpheader->th_flags != (TH_FIN | TH_ACK) &&
        !(tcpheader->th_flags & TH_RST)
    )
        return 0;

    tcpip_info.content_length =
        ntohs(tcpip_info.ipheader->tot_len) -
        (tcpheader->doff + (ipheader->ihl << 2));

    if (tcpheader->th_flags == (TH_SYN | TH_ACK))
        return check_status = HandleSynAckPacket();

    if (tcpheader->th_flags == (TH_FIN | TH_ACK) || tcpheader->th_flags & TH_RST)
        return check_status = HandleFinAckPacket();

    return check_status = HandleDataPacket();
}

bool CIsProSer::GetFakeMacInfo(
    in_addr_t *ipaddr,
    struct ether_addr *macaddr
) const
{
    if (check_status != 12)
        return 0;

    *ipaddr = detected_fakeipaddr;
    *macaddr = detected_fakemac;
}

bool CIsProSer::Start(
    const char *adapter_name,
    const struct ether_addr *host_mac,
    unsigned int kind
) const
{
}

bool CIsProSer::Stop() const
{
}

void CIsProSer::AddToHRList(ProxySerTcp *server) const
{
}

void CIsProSer::AddToRHList(ProxyClientTcp *client) const
{
}

void CIsProSer::DelFromHRList(ProxySerTcp *server) const
{
}

void CIsProSer::DelFromRHList(ProxyClientTcp *client) const
{
}

int CIsProSer::HandleDataPacket() const
{
}

int CIsProSer::HandleFinAckPacket() const
{
}

int CIsProSer::HandleSynAckPacket() const
{
}

bool CIsProSer::IsFakeMac(
    const struct ether_header *ehdr,
    const struct iphdr *iphdr
) const
{
}

bool CIsProSer::IsIPInLocalIPTable(in_addr_t ipaddr) const
{
}

void CIsProSer::OnTimer(int a2, int a3) const
{
}

void CIsProSer::UpdateLocalIPTable() const
{
}
