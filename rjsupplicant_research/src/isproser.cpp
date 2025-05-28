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

int CIsProSer::Detect(struct etherpkg *pkg, unsigned int pkglen) const
{
    if (!host_addr_set) {
        g_logFile_proxy.AppendText("CIsProSer::Detect no set host addr\r\n");
        return -1;
    }

    if (
        !pkg ||
        pkglen <= offsetof(struct EtherPkg, tcpheader) ||
        htons(pkg->etherheader.ether_type) != ETHERTYPE_IP
    )
        return 0;

    if (pkg->ipheader.ihl << 2 < 20) {
        g_logFile_proxy.AppendText("IP Head Len min than 20\r\n");
        return 0;
    }
}

bool CIsProSer::GetFakeMacInfo(
    in_addr_t *ipaddr,
    struct ether_addr *macaddr
) const
{
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

void CIsProSer::HandleFinAckPacket() const
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
