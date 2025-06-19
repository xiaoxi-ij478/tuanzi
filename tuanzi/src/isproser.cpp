#include "all.h"
#include "global.h"
#include "timeutil.h"
#include "util.h"
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
    clienttcp_begin(),
    clienttcp_end(),
    sertcp_begin(),
    sertcp_end()
{}

CIsProSer::~CIsProSer()
{
    while (sertcp_begin)
        DelFromHRList(sertcp_begin);

    while (clienttcp_begin)
        DelFromRHList(clienttcp_begin);
}

int CIsProSer::Detect(const unsigned char *pkg, unsigned pkglen)
{
    const struct ether_header *etherheader =
            reinterpret_cast<const struct ether_header *>(pkg);
    const struct iphdr *ipheader =
            reinterpret_cast<const struct iphdr *>(pkg + sizeof(struct ether_header));
    const struct tcphdr *tcpheader = nullptr;

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
            reinterpret_cast<const struct tcphdr *>(
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
        return false;

    *ipaddr = detected_fakeipaddr;
    *macaddr = detected_fakemac;
    return true;
}

bool CIsProSer::Start(
    const char *adapter_name_l,
    const struct ether_addr *hostmac_l,
    unsigned kind_l
)
{
    if (!adapter_name_l || !hostmac_l)
        return false;

    g_logFile_proxy.AppendText(
        "adapterName=%s hostMac=%02x:%02x:%02x:%02x:%02x:%02x kinds=%08x",
        adapter_name_l,
        hostmac_l->ether_addr_octet[0],
        hostmac_l->ether_addr_octet[1],
        hostmac_l->ether_addr_octet[2],
        hostmac_l->ether_addr_octet[3],
        hostmac_l->ether_addr_octet[4],
        hostmac_l->ether_addr_octet[5],
        kind_l
    );

    while (sertcp_begin)
        DelFromHRList(sertcp_begin);

    while (clienttcp_begin)
        DelFromRHList(clienttcp_begin);

    sertcp_begin = sertcp_end = nullptr;
    clienttcp_begin = clienttcp_end = nullptr;
    strcpy(adapter_name, adapter_name_l);
    hostmac = *hostmac_l;
    kind = kind_l;
    check_status = 0;
    detected_fakemac = {};
    last_updated_time = 0;
    host_addr_set = true;
    detected_fakeipaddr = 0;
    updated_times = 0;
    local_ips.clear();
    return true;
}

bool CIsProSer::Stop()
{
    host_addr_set = false;

    while (sertcp_begin)
        DelFromHRList(sertcp_begin);

    while (clienttcp_begin)
        DelFromRHList(clienttcp_begin);

    sertcp_begin = sertcp_end = nullptr;
    clienttcp_begin = clienttcp_end = nullptr;
    return true;
}

void CIsProSer::AddToHRList(ProxySerTcp *server)
{
    if (sertcp_begin) {
        server->next = sertcp_begin;
        sertcp_begin->prev = server;
        sertcp_begin = server;

    } else {
        sertcp_begin = server;
        sertcp_end = server;
    }
}

void CIsProSer::AddToRHList(ProxyClientTcp *client)
{
    if (clienttcp_begin) {
        client->next = clienttcp_begin;
        clienttcp_begin->prev = client;
        clienttcp_begin = client;

    } else {
        clienttcp_begin = client;
        clienttcp_end = client;
    }
}

void CIsProSer::DelFromHRList(ProxySerTcp *server)
{
    if (!server)
        return;

    if (!server->prev)
        sertcp_begin = server->next;

    else
        server->prev->next = server->next;

    if (!server->next)
        sertcp_end = server->prev;

    else
        server->next->prev = server->prev;

    if (server->bound_proxy_client_tcp) {
        if (server->bound_proxy_client_tcp->bound_proxy_ser_tcp == server) {
            server->bound_proxy_client_tcp->bound_proxy_ser_tcp = nullptr;
            DelFromRHList(server->bound_proxy_client_tcp);
        }
    }

    delete server;
}

void CIsProSer::DelFromRHList(ProxyClientTcp *client)
{
    if (!client)
        return;

    if (!client->prev)
        clienttcp_begin = client->next;

    else
        client->prev->next = client->next;

    if (!client->next)
        clienttcp_end = client->prev;

    else
        client->next->prev = client->prev;

    if (client->bound_proxy_ser_tcp) {
        if (client->bound_proxy_ser_tcp->bound_proxy_client_tcp == client) {
            client->bound_proxy_ser_tcp->bound_proxy_client_tcp = nullptr;
            DelFromHRList(client->bound_proxy_ser_tcp);
        }
    }

    delete client;
}

int CIsProSer::HandleDataPacket()
{
    int ret = 0;
    bool no_free_sertcp = false;

    for (
        ProxyClientTcp *clienttcp = clienttcp_begin;
        clienttcp;
    )
        switch (ret = clienttcp->TryDetectTCPIP(tcpip_info, sertcp_begin, kind)) {
            case -1:
                return 0;

            case -2:
                if (clienttcp_begin->reqaddr_char[0])
                    no_free_sertcp = true;

                if (!clienttcp_begin->next)
                    clienttcp = nullptr; // break outer loop

                break;

            case -3:
                clienttcp = clienttcp->next;
                break;

            default: // >= 0, assumed
                DelFromRHList(clienttcp);
                return ret;
        }

    if (!no_free_sertcp) {
        free_list_with_func(sertcp_begin, DelFromHRList);
        return 0;
    }

    for (ProxySerTcp *sertcp = sertcp_begin; sertcp; sertcp = sertcp->next)
        switch (sertcp->FindClientTcp(tcpip_info, clienttcp_begin, kind)) {
            case -2:
                break;

            case -1:
                DelFromHRList(sertcp);
                [[fallthrough]];

            default:
                return 0;
        }

    return 0;
}

int CIsProSer::HandleFinAckPacket()
{
    GetTickCount(); // ?

    for (
        ProxyClientTcp *clienttcp = clienttcp_end;
        clienttcp;
        clienttcp = clienttcp->prev
    ) {
        if (clienttcp->IsMine(tcpip_info)) {
            DelFromRHList(clienttcp);
            return 0;
        }

        if (clienttcp->IsExpired(0))
            DelFromRHList(clienttcp->prev);
    }

    for (ProxySerTcp *sertcp = sertcp_end; sertcp; sertcp = sertcp->prev) {
        if (sertcp->IsMine(tcpip_info)) {
            DelFromHRList(sertcp);
            return 0;
        }

        if (sertcp->IsExpired(0))
            DelFromHRList(sertcp->prev);
    }

    return 0;
}

int CIsProSer::HandleSynAckPacket()
{
    struct TcpInfo tcpinfo;

    if (!memcmp(tcpip_info.etherheader->ether_shost, &hostmac, sizeof(hostmac))) {
        tcpinfo.srcaddr = tcpip_info.ipheader->daddr;
        tcpinfo.srcport = ntohs(tcpip_info.tcpheader->dest);
        tcpinfo.ack_seq = ntohl(tcpip_info.tcpheader->ack_seq);
        tcpinfo.dstaddr = tcpip_info.ipheader->saddr;
        tcpinfo.dstport = ntohs(tcpip_info.tcpheader->source);
        tcpinfo.seq = ntohl(tcpip_info.tcpheader->seq) + 1;
        AddToRHList(new ProxyClientTcp(tcpinfo));
        return 0;
    }

    if (memcmp(tcpip_info.etherheader->ether_dhost, &hostmac, sizeof(hostmac))) {
        g_logFile_proxy.AppendText(
            "CIsProSer::HandleSynAckPacket receive no host's packet\r\n"
        );
        return 0;
    }

    for (
        ProxyClientTcp *clienttcp = clienttcp_end;
        clienttcp;
        clienttcp = clienttcp->prev
    ) {
        if (!clienttcp->reqaddr_char[0])
            continue;

        tcpinfo.srcaddr = tcpip_info.ipheader->saddr;
        tcpinfo.srcport = ntohs(tcpip_info.tcpheader->source);
        tcpinfo.ack_seq = ntohl(tcpip_info.tcpheader->seq) + 1;
        tcpinfo.dstaddr = tcpip_info.ipheader->daddr;
        tcpinfo.dstport = ntohs(tcpip_info.tcpheader->dest);
        tcpinfo.seq = ntohl(tcpip_info.tcpheader->ack_seq);
        AddToHRList(new ProxySerTcp(tcpinfo));
        return 0;
    }

    free_list_with_func(sertcp_begin, DelFromHRList);

    while (sertcp_begin)
        DelFromHRList(sertcp_begin);

    return 0;
}

bool CIsProSer::IsFakeMac(
    const struct ether_header *ehdr,
    const struct iphdr *iphdr
)
{
    if (
        // is the packet sent from the (reported) our machine's interface?
        memcmp(ehdr->ether_shost, &hostmac, sizeof(hostmac)) ||
        // if not, check if the ip address is in the local interfaces
        IsIPInLocalIPTable(iphdr->saddr) ||
        // if the update time exceeds 200, stop and report abuse
        updated_times < 200
    )
        return false;

    check_status = 12;
    detected_fakemac = hostmac;
    detected_fakeipaddr = iphdr->saddr;
    return true;
}

bool CIsProSer::IsIPInLocalIPTable(in_addr_t ipaddr)
{
    unsigned long cur_tick = GetTickCount();

    for (const struct tagLocalIP &localip : local_ips)
        if (ipaddr == localip.local_ip && cur_tick - localip.creation_time < 120000)
            return true;

    UpdateLocalIPTable();

    for (const struct tagLocalIP &localip : local_ips)
        if (ipaddr == localip.local_ip && cur_tick - localip.creation_time < 120000)
            return true;

    if (updated_times && cur_tick - last_updated_time < 120000)
        updated_times++;

    else {
        last_updated_time = cur_tick;
        updated_times = 1;
    }

    return false;
}

void CIsProSer::OnTimer(int, int)
{
    unsigned long cur_tick = GetTickCount();

    for (ProxySerTcp *server = sertcp_begin; server; server = server->next) {
        if (cur_tick - server->creation_time < 300000)
            continue;

        if (server == sertcp_begin)
            sertcp_begin = sertcp_end = nullptr;

        else {
            sertcp_end = server->prev;
            sertcp_end->next = nullptr;
        }

        free_list(server);
    }

    for (ProxyClientTcp *client = clienttcp_begin; client; client = client->next) {
        if (cur_tick - client->creation_time < 300000)
            continue;

        if (client == clienttcp_begin)
            clienttcp_begin = clienttcp_end = nullptr;

        else {
            clienttcp_end = client->prev;
            clienttcp_end->next = nullptr;
        }

        free_list(client);
    }
}

void CIsProSer::UpdateLocalIPTable()
{
    unsigned long cur_tick = GetTickCount();
    int fd = 0;
    struct ifaddrs *ifap = nullptr;
    bool updated = false;

    for (auto it = local_ips.begin(); it != local_ips.end(); it++)
    {
        if (cur_tick - local_ips[i].creation_time >= 120000)
            it = local_ips.erase(it) - 1;

    }
    if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) <= 0)
        return;

    if (getifaddrs(&ifap) || !ifap) {
        close(fd);
        return;
    }

    for (
        struct ifaddrs *cur_if = ifap;
        cur_if;
        updated = false, cur_if = cur_if->ifa_next
    ) {
        if (
            strcmp(cur_if->ifa_name, adapter_name) ||
            cur_if->ifa_addr->sa_family != AF_INET
        )
            continue;

        for (struct tagLocalIP &localip : local_ips) {
            if (
                localip.local_ip != reinterpret_cast<struct sockaddr_in *>
                (cur_if->ifa_addr)->sin_addr.s_addr
            )
                continue;

            localip.creation_time = cur_tick;
            updated = true;
            break;
        }

        if (updated)
            continue;

        local_ips.emplace_back(
            reinterpret_cast<struct sockaddr_in *>(cur_if->ifa_addr)->sin_addr.s_addr,
            cur_tick
        );
    }

    freeifaddrs(ifap);
    close(fd);
}
