#include "all.h"
#include "psutil.h"
#include "cmdutil.h"
#include "timeutil.h"
#include "fileutil.h"
#include "util.h"
#include "sysutil.h"
#include "stdpkgs.h"
#include "global.h"
#include "netutil.h"

int sockets_open()
{
#define TRY_CREATE_AND_RETURN(domain, type) \
    do { \
        int result = socket((domain), (type), 0); \
        if (result != -1) \
            return result; \
    } while (0)
    TRY_CREATE_AND_RETURN(AF_INET, SOCK_DGRAM);
    TRY_CREATE_AND_RETURN(AF_IPX, SOCK_DGRAM);
    TRY_CREATE_AND_RETURN(AF_AX25, SOCK_DGRAM);
    TRY_CREATE_AND_RETURN(AF_APPLETALK, SOCK_DGRAM);
    return -1;
#undef TRY_CREATE_AND_RETURN
}

enum ADAPTER_TYPE get_nic_type(const char *ifname)
{
    int fd = sockets_open();
    struct iwreq iwr = {};

    if (fd < 0) {
        perror("get_nic_type __ sockets_open");
        return ADAPTER_WIRELESS;
    }

    strncpy(iwr.ifr_name, ifname, IFNAMSIZ - 1);
    return ioctl(fd, SIOCGIWNAME, &iwr) >= 0 ? ADAPTER_WIRELESS : ADAPTER_WIRED;
}

unsigned short ComputeTcpPseudoHeaderChecksum(
    const struct iphdr *ipheader,
    struct tcphdr *tcpheader,
    const char *databuf,
    int length
)
{
    struct tcp_checksum_hdr header = {};
#define SET_PSEUDO_HEADER_INFO(name) header.pseudo_header.name = ipheader->name
    SET_PSEUDO_HEADER_INFO(saddr);
    SET_PSEUDO_HEADER_INFO(daddr);
    SET_PSEUDO_HEADER_INFO(protocol);
    header.pseudo_header.tcp_length = htons(length + sizeof(header.real_header));
    header.real_header = *tcpheader;
#undef SET_PSEUDO_HEADER_INFO
    memcpy(header.data, databuf, length);
    return tcpheader->check =
               checksum(
                   reinterpret_cast<unsigned short *>(&header),
                   length + sizeof(header.real_header) + sizeof(header.pseudo_header)
               );
}

unsigned short ComputeUdpPseudoHeaderChecksumV4(
    const struct iphdr *ipheader,
    struct udphdr *udpheader,
    const char *databuf,
    int length
)
{
    struct udp_checksum_hdr header = {};
#define SET_PSEUDO_HEADER_INFO(name) header.pseudo_header.name = ipheader->name
    SET_PSEUDO_HEADER_INFO(saddr);
    SET_PSEUDO_HEADER_INFO(daddr);
    SET_PSEUDO_HEADER_INFO(protocol);
    header.pseudo_header.udp_length = htons(length + sizeof(header.real_header));
    header.real_header = *udpheader;
#undef SET_PSEUDO_HEADER_INFO
    memcpy(header.data, databuf, length);
    return udpheader->check =
               checksum(
                   reinterpret_cast<unsigned short *>(&header),
                   length + sizeof(header.real_header) + sizeof(header.pseudo_header)
               );
}

unsigned short checksum(const unsigned short *data, unsigned len)
{
    unsigned checksum = 0;

    // add up all the complete words
    for (unsigned i = 0; i < len >> 1; i++)
        checksum += *data++;

    // add the additional one padding to word if exists
    if (len & 1)
        checksum += *reinterpret_cast<const char *>(data);

    // repeatedly take the high 16 bit and add to the low 16 bit
    // until the high 16 bit is 0
    while (checksum >> 16)
        checksum = (checksum & 0xffff) + (checksum >> 16);

    return htons(~checksum);
}

struct NICINFO *get_nics_info(const char *ifname)
{
    struct NICINFO *info = nullptr;
    struct NICINFO *cur_info = nullptr;
    struct NICINFO *tmp_info = nullptr;
    struct ifaddrs *ifap = nullptr;
    struct NICINFO::IPAddrNode *tmp_ipnode = nullptr, *tmp2_ipnode = nullptr;
    struct NICINFO::IP6AddrNode *tmp_ip6node = nullptr, *tmp2_ip6node = nullptr;
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    bool interface_added = false;
    struct ifreq ifr = {};
    in_addr_t dns_addr = 0;

    if (fd == -1)
        return nullptr;

    if (getifaddrs(&ifap) == -1 || !ifap) {
        close(fd);
        return nullptr;
    }

    if (get_dns(&dns_addr)) {}

//        swap32(static_cast<char *>(&dns_addr.s_addr));

    for (struct ifaddrs *cur_if = ifap; cur_if; cur_if = cur_if->ifa_next) {
        interface_added = false;

        if (cur_if->ifa_flags & IFF_LOOPBACK)
            continue;

        if (ifname && strcmp(ifname, cur_if->ifa_name))
            continue;

        for (struct NICINFO *c = info; c; c = c->next)
            if (!strcmp(c->ifname, cur_if->ifa_name)) {
                interface_added = true;
                break;
            }

        if (!info || !interface_added) {
            cur_info = new struct NICINFO;

            if (!cur_info)
                continue;

            if (!info)
                info = cur_info;

            if (!interface_added) {
                tmp_info = info;

                while (tmp_info->next)
                    tmp_info = tmp_info->next;

                tmp_info->next = cur_info;
            }

            strncpy(cur_info->ifname, cur_if->ifa_name, IFNAMSIZ - 1);
            strncpy(ifr.ifr_name, cur_if->ifa_name, IFNAMSIZ - 1);

            if (ioctl(fd, SIOCGIFHWADDR, &ifr))
                memset(&cur_info->hwaddr, 0, sizeof(cur_info->hwaddr));

            else
                memcpy(
                    &cur_info->hwaddr,
                    &ifr.ifr_hwaddr.sa_data,
                    sizeof(cur_info->hwaddr)
                );

            cur_info->use_dhcp = true;

            if (get_nic_type(cur_if->ifa_name) == ADAPTER_WIRELESS) {
                cur_info->is_wireless = true;
                cur_info->speed = get_speed_wl(fd, cur_if->ifa_name);

            } else {
                cur_info->is_wireless = false;
                cur_info->speed = get_speed(fd, cur_if->ifa_name);
            }

            if (get_gateway(&cur_info->gateway, cur_if->ifa_name)) {
                memset(&cur_info->gateway_mac, 0, sizeof(cur_info->gateway_mac));
                cur_info->gateway = ntohl(cur_info->gateway);

            } else
                memset(&cur_info->gateway, 0, sizeof(cur_info->gateway));

            cur_info->dns = dns_addr;
        }

        switch (cur_if->ifa_addr->sa_family) {
            case AF_INET:
                tmp_ipnode = new struct NICINFO::IPAddrNode;

                if (!tmp_ipnode)
                    continue;

                cur_info->ipaddr_count++;
                tmp_ipnode->ipaddr =
                    reinterpret_cast<struct sockaddr_in *>
                    (cur_if->ifa_addr)->sin_addr.s_addr;
//                swap32(reinterpret_cast<char *>(&tmp_ipnode->ipaddr.s_addr));
                tmp_ipnode->netmask =
                    reinterpret_cast<struct sockaddr_in *>
                    (cur_if->ifa_netmask)->sin_addr.s_addr;

//                swap32(reinterpret_cast<char *>(&tmp_ipnode->netmask.s_addr));
                if (!cur_info->ipaddrs) {
                    cur_info->ipaddrs = tmp_ipnode;
                    cur_info->use_dhcp =
                        check_dhcp(
                            cur_if->ifa_name,
                            inet_ntoa({ tmp_ipnode->ipaddr })
                        );

                } else {
                    tmp2_ipnode = cur_info->ipaddrs;

                    while (tmp2_ipnode->next)
                        tmp2_ipnode = tmp2_ipnode->next;

                    tmp2_ipnode->next = tmp_ipnode;
                }

                break;

            case AF_INET6:
                tmp_ip6node = new struct NICINFO::IP6AddrNode;

                if (!tmp_ip6node)
                    continue;

                cur_info->ipaddr6_count++;
                tmp_ip6node->ipaddr =
                    reinterpret_cast<struct sockaddr_in6 *>
                    (cur_if->ifa_addr)->sin6_addr;
//                swap128(reinterpret_cast<char *>(tmp_ip6node->ipaddr));
                tmp_ip6node->netmask =
                    reinterpret_cast<struct sockaddr_in6 *>
                    (cur_if->ifa_netmask)->sin6_addr;

//                swap128(reinterpret_cast<char *>(tmp_ip6node->netmask));

                if (!cur_info->ip6addrs)
                    cur_info->ip6addrs = tmp_ip6node;

                else {
                    tmp2_ip6node = cur_info->ip6addrs;

                    while (tmp2_ip6node->next)
                        tmp2_ip6node = tmp2_ip6node->next;

                    tmp2_ip6node->next = tmp_ip6node;
                }

                break;

            default:
                break;
        }
    }

    return info;
}

void free_nics_info(struct NICINFO *info)
{
    free_list_with_func(info, [](struct NICINFO *nicinfo) { // *NOPAD*
        free_list(nicinfo->ipaddrs);
        free_list(nicinfo->ip6addrs);
        delete nicinfo;
    });
}

bool get_dns(in_addr_t *dst)
{
    // the original implementation uses shell
    // cat /etc/resolv.conf 2>&- |awk '{if ($1=="nameserver") {print $2;exit}}'
    std::ifstream ifs("/etc/resolv.conf");
    std::vector<std::string> val;
    std::string line;
    bool found = false;

    if (!ifs)
        return false;

    while (!found && std::getline(ifs, line)) {
        ParseString(line, ' ', val);

        if (val[0] == "nameserver")
            break;
    }

    ifs.close();

    if (!found)
        return false;

    inet_pton(AF_INET, val[1].c_str(), &dst);
    return true;
}

bool get_alternate_dns(char *dest, unsigned &counts)
{
    // cat /etc/resolv.conf 2>&- |awk '{if ($1==\"nameserver\") {print $2}}' |awk 'NR>1'
    std::ifstream ifs("/etc/resolv.conf");
    std::string line;
    std::vector<std::string> val;
    unsigned c = 0;

    if (!ifs) {
        counts = 0;
        return false;
    }

    while (std::getline(ifs, line)) {
        ParseString(line, ' ', val);

        if (val[0] != "nameserver")
            continue;

        if (!c++)
            continue;

        val[1].copy(dest + strlen(dest), val[1].length());
        strcat(dest, ";");
        counts++;
    }

    dest[strlen(dest) - 1] = 0;
    return true;
}

bool get_gateway(in_addr_t *result, const char *ifname)
{
    // cat /proc/net/route 2>&- |awk '{if ($1~/$ifname/ && $2~/00000000/) print $3}'
    std::ifstream ifs("/proc/net/route");
    std::string line;
    std::vector<std::string> arr;

    if (!ifs)
        return false;

    while (std::getline(ifs, line)) {
        ParseString(line, '\t', arr);

        if (arr[0] == ifname && arr[1] == "00000000") {
            *result = std::stoi(arr[2], nullptr, 16);
            ifs.close();
            return true;
        }
    }

    ifs.close();
    return false;
}

unsigned short get_speed_wl(int fd, char *ifname)
{
    struct iwreq iwr = {};
    strncpy(iwr.ifr_name, ifname, IFNAMSIZ - 1);
    return ioctl(fd, SIOCGIWRATE, &iwr) ? 0 : iwr.u.bitrate.value / 1000000;
}

unsigned short get_speed(int fd, char *ifname)
{
    struct ethtool_cmd ecmd = {};
    struct ifreq ifr = {};
    ecmd.cmd = ETHTOOL_GSET;
    strncpy(ifr.ifr_name, ifname, IFNAMSIZ - 1);
    ifr.ifr_data = reinterpret_cast<caddr_t>(&ecmd);
    return ioctl(fd, SIOCETHTOOL, &ifr) ? 0 : ethtool_cmd_speed(&ecmd);
}

bool check_manualip_indirectory(
    const char *ipaddr,
    const char *dir,
    bool incl_subdir
)
{
    // if $incl_subdir
    // then grep $ipaddr $dir/*/* 2>&-
    // else grep $ipaddr $dir/* 2>&-
    // fi
    DIR *rootdir = opendir(dir);
    DIR *subdir = nullptr;
    std::ifstream ifs;
    std::string line;
    std::string sdir(dir);

    if (!rootdir)
        return false;

    for (struct dirent *dent = readdir(rootdir); dent; dent = readdir(rootdir)) {
        if (dent->d_type != DT_REG)
            continue;

        if (incl_subdir && dent->d_type == DT_DIR) {
            subdir = opendir((sdir + dent->d_name).c_str());

            if (!subdir)
                continue;

            for (
                struct dirent *sdent = readdir(subdir);
                sdent;
                sdent = readdir(subdir)
            ) {
                if (sdent->d_type != DT_REG)
                    continue;

                ifs.open(sdent->d_name);

                if (!ifs)
                    continue;

                while (std::getline(ifs, line))
                    if (line.find(ipaddr) != std::string::npos) {
                        ifs.close();
                        closedir(subdir);
                        closedir(rootdir);
                        return true;
                    }

                ifs.close();
            }

            closedir(subdir);
            subdir = nullptr;
            continue;
        }

        ifs.open(dent->d_name);

        if (!ifs)
            continue;

        while (std::getline(ifs, line))
            if (line.find(ipaddr) != std::string::npos) {
                ifs.close();
                closedir(rootdir);
                return true;
            }

        ifs.close();
    }

    closedir(rootdir);
    return false;
}

bool check_manualip_infile(const char *ipaddr, const char *file)
{
    std::ifstream ifs(file);
    std::string line;

    if (!ifs)
        return false;

    while (std::getline(ifs, line))
        if (line.find(ipaddr) != std::string::npos) {
            ifs.close();
            return true;
        }

    ifs.close();
    return false;
}

bool check_dhcp([[maybe_unused]] const char *ifname, const char *ipaddr)
{
    if (!ipaddr || !strcmp(ipaddr, "0.0.0.0"))
        return true;

    return
        !check_manualip_indirectory(
            ipaddr,
            "/etc/sysconfig/networking/",
            true
        ) &&
        !check_manualip_indirectory(
            ipaddr,
            "/etc/sysconfig/network-scripts/",
            false
        ) &&
        !check_manualip_infile(
            ipaddr,
            "/etc/network/interfaces"
        ) &&
        !check_manualip_indirectory(
            ipaddr,
            "/etc/NetworkManager/system-connections/",
            false
        );
}

bool get_ip_mac(in_addr_t ipaddr, struct ether_addr *macaddr)
{
    std::string empty_mac("00:00:00:00:00:00"), full_mac("ff:ff:ff:ff:ff:ff");
    std::string ip, mac;
    std::string line;
    std::ifstream ifs("/proc/net/arp");
    std::string::iterator b, e;
    std::string testip(inet_ntoa({ ipaddr }));
    std::vector<std::string> val;
    // minimal-ping related
    int fd = 0;
    struct icmppkg package = {}, recv_package = {};
    struct sockaddr_in dest_addr = { AF_INET, 0, { ipaddr } };
    struct timeval wait_sec = { 3, 0 };
    unsigned dest_addrlen = sizeof(dest_addr);
    unsigned short orig_checksum = 0;
    fd_set listen_fds;

    if (!ifs)
        return false;

    while (std::getline(ifs, line)) {
        ParseString(line, ' ', val);

        if (val[0] != testip || val[3] == empty_mac || val[3] == full_mac)
            continue;

        sscanf(
            val[3].c_str(),
            "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
            &macaddr->ether_addr_octet[0],
            &macaddr->ether_addr_octet[1],
            &macaddr->ether_addr_octet[2],
            &macaddr->ether_addr_octet[3],
            &macaddr->ether_addr_octet[4],
            &macaddr->ether_addr_octet[5]
        );
        ifs.close();
        return true;
    }

    ifs.close();
    // minimal ping implementation
    // yes, I copied them from inetutils
    fd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    FD_ZERO(&listen_fds);
    FD_SET(fd, &listen_fds);
    package.hdr.type = ICMP_ECHO;
    package.hdr.code = 0;
    package.hdr.checksum = 0;
    package.hdr.un.echo.sequence = htons(1);
    package.hdr.un.echo.id = htons(9527);
    memcpy(
        package.data,
        "tuanzituanzituanzituanzituanzituanzituanzituanzi",
        sizeof(package.data)
    );
    package.hdr.checksum =
        checksum(
            reinterpret_cast<unsigned short *>(&package),
            sizeof(package)
        );
    sendto(
        fd,
        &package,
        sizeof(package),
        0,
        reinterpret_cast<struct sockaddr *>(&dest_addr),
        dest_addrlen
    );

    // wait up to 3 seconds
    switch (select(fd + 1, &listen_fds, nullptr, nullptr, &wait_sec)) {
        case -1: // some error
        case 0: // the host may not be online
            return false;

        case 1: // got response
            break;
    }

//    if (!FD_ISSET(fd, &listen_fds))
//        return false;
    recvfrom(
        fd,
        &recv_package,
        sizeof(recv_package),
        0,
        reinterpret_cast<struct sockaddr *>(&dest_addr),
        &dest_addrlen
    );
    close(fd);
    orig_checksum = recv_package.hdr.checksum;
    recv_package.hdr.checksum = 0;

    if (
        checksum(
            reinterpret_cast<unsigned short *>(&recv_package),
            sizeof(recv_package)
        ) != orig_checksum
    )
        return false;

    if (
        recv_package.hdr.type != ICMP_ECHOREPLY ||
        recv_package.hdr.un.echo.sequence != htons(1) ||
        recv_package.hdr.un.echo.id != htons(9527) ||
        memcmp(
            recv_package.data,
            "tuanzituanzituanzituanzituanzituanzituanzituanzi",
            sizeof(recv_package.data)
        )
    )
        return false;

//    system(std::string("ping -c 1").insert(5, inet_ntoa(ipaddr)).c_str());
    ifs.open("/proc/net/arp");

    if (!ifs)
        return false;

    while (std::getline(ifs, line)) {
        ParseString(line, ' ', val);

        if (val[0] != testip || val[3] == empty_mac || val[3] == full_mac)
            continue;

        sscanf(
            val[3].c_str(),
            "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
            &macaddr->ether_addr_octet[0],
            &macaddr->ether_addr_octet[1],
            &macaddr->ether_addr_octet[2],
            &macaddr->ether_addr_octet[3],
            &macaddr->ether_addr_octet[4],
            &macaddr->ether_addr_octet[5]
        );
        ifs.close();
        return true;
    }

    ifs.close();
    return false;
}

int check_nic_status(const char *ifname)
{
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    struct ifreq ifr = {};
//    struct ethtool_value evalue;

    if (fd == -1)
        return ADAPTER_INVALID;

    strncpy(ifr.ifr_name, ifname, IFNAMSIZ - 1);
//    evalue.cmd = ETHTOOL_GLINK;
//    ifr.ifr_data = &evalue;

    if (ioctl(fd, SIOCGIFFLAGS, &ifr) < 0) {
        close(fd);
        return ADAPTER_INVALID;
    }

    close(fd);
    return ifr.ifr_flags & IFF_UP ? ADAPTER_UP : ADAPTER_DOWN;
}

bool get_nic_in_use(std::vector<std::string> &nic_list, bool wireless_only)
{
    struct ifaddrs *ifap = nullptr;
    bool in_list = false;
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    nic_list.clear();

    if (fd == -1)
        return false;

    if (getifaddrs(&ifap) == -1 || !ifap) {
        freeifaddrs(ifap);
        close(fd);
        return false;
    }

    for (struct ifaddrs *cur_if = ifap; cur_if; cur_if = cur_if->ifa_next) {
        if (cur_if->ifa_flags & IFF_LOOPBACK)
            continue;

        for (const std::string &nic_name : nic_list)
            if (nic_name == cur_if->ifa_name) {
                in_list = true;
                break;
            }

        if (in_list)
            continue;

        if (check_nic_status(cur_if->ifa_name) == ADAPTER_DOWN)
            rj_printf_debug("%s disable,skip.", cur_if->ifa_name);

        if (wireless_only && get_nic_type(cur_if->ifa_name) != ADAPTER_WIRELESS)
            continue;

        nic_list.push_back(cur_if->ifa_name);
    }

    freeifaddrs(ifap);
    close(fd);
    return true;
}

// ????????
// lshal 2>&- |grep net.interface | awk -F\"'\" '{print$2}'
int get_nic_list(std::vector<std::string>)
{
    return 0;
}

bool get_nic_speed(char *dst, const char *ifname)
{
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    struct ifaddrs *ifap = nullptr;
    unsigned short speed = 0;

    if (fd == -1)
        return false;

    if (getifaddrs(&ifap) == -1 || !ifap) {
        freeifaddrs(ifap);
        close(fd);
        return false;
    }

    for (struct ifaddrs *cur_if = ifap; cur_if; cur_if = cur_if->ifa_next) {
        if (cur_if->ifa_flags & IFF_LOOPBACK)
            continue;

        if (ifname && strcmp(ifname, cur_if->ifa_name))
            continue;

        switch (get_nic_type(cur_if->ifa_name)) {
            case ADAPTER_WIRELESS:
                speed = get_speed_wl(fd, cur_if->ifa_name);
                break;

            case ADAPTER_WIRED:
                speed = get_speed(fd, cur_if->ifa_name);
                break;
        }
    }

    freeifaddrs(ifap);
    close(fd);

    if (speed)
        sprintf(dst, "%d.0", speed);

    return speed;
}

bool GetNICInUse(std::vector<std::string> &nic_list, bool wireless_only)
{
    nic_list.clear();
    return get_nic_in_use(nic_list, wireless_only);
}

unsigned InitIpv4Header(
    struct iphdr *header,
    const char *srcaddr,
    const char *dstaddr,
    unsigned datalen
)
{
    header->version = 4;
    header->ihl = 5;
    header->tos = 0;
    header->tot_len =
        htons(datalen + sizeof(struct iphdr) + sizeof(struct udphdr));
    header->id = 0;
    header->frag_off = 0;
    header->ttl = 0x80;
    header->protocol = IPPROTO_UDP;
    header->check = 0;
    header->saddr = inet_addr(srcaddr);
    header->daddr = inet_addr(dstaddr);
    header->check =
        checksum(
            reinterpret_cast<unsigned short *>(header),
            sizeof(struct iphdr)
        );
    return sizeof(struct iphdr);
}

unsigned InitUdpHeader(
    struct udphdr *header,
    unsigned srcport,
    unsigned dstport,
    unsigned datalen
)
{
    header->source = htons(srcport);
    header->dest = htons(dstport);
    header->len = htons(datalen + sizeof(struct udphdr));
    header->check = 0;
    return sizeof(struct udphdr);
}

bool Is8021xGroupAddr(struct ether_addr *macaddr)
{
    // see https://en.wikipedia.org/wiki/IEEE_802.1X#Typical_authentication_progression
    // 01:80:C2:00:00:03
    struct ether_addr group_addr = { 0x01, 0x80, 0xC2, 0x00, 0x00, 0x03 };
    return !memcmp(macaddr, &group_addr, sizeof(group_addr));
}

bool IsEqualIP(in_addr_t *ipaddr1, in_addr_t *ipaddr2)
{
    return *ipaddr1 == *ipaddr2;
}

bool IsEqualMac(struct ether_addr *macaddr1, struct ether_addr *macaddr2)
{
    return !memcmp(macaddr1, macaddr2, sizeof(struct ether_addr));
}

bool IsGetDhcpIpp(in_addr_t *ip)
{
    // !169.254.xxx.xxx ||
    // 0.xxx.xxx.xxx
    return (*ip >> 24) ?
           (*ip >> 24) != 169 || (*ip >> 24) != 254 :
           (*ip >> 16 & 0xff) || (*ip >> 8 & 0xff) || (*ip & 0xff);
}

bool IsHostDstMac(struct ether_addr *macaddr1, struct ether_addr *macaddr2)
{
    return !memcmp(macaddr1, macaddr2, sizeof(struct ether_addr));
}

bool IsHostDstMac(struct ether_addr *macaddr)
{
    struct ether_addr host_macaddr = {};
    CtrlThread->GetAdapterMac(&host_macaddr);
    return !memcmp(&host_macaddr, macaddr, sizeof(struct ether_addr));
}

bool IsMulDstMac(struct ether_addr *macaddr)
{
    // FF:FF:FF:FF:FF:FF
    struct ether_addr multicast_mac = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
    return !memcmp(macaddr, &multicast_mac, sizeof(multicast_mac));
}

bool IsStarGroupDstMac(struct ether_addr *macaddr)
{
    // 00:D0:F8:00:00:03
    struct ether_addr star_group_addr = { 0x00, 0xD0, 0xF8, 0x00, 0x00, 0x03 };
    return !memcmp(macaddr, &star_group_addr, sizeof(star_group_addr));
}

bool check_nic_isok(char *ifname)
{
    char errbuf[PCAP_ERRBUF_SIZE] = {};
    pcap_t *handle = nullptr;

    if (!ifname)
        return false;

    if (!(handle = pcap_open_live(ifname, 2000, true, -1, errbuf)))
        return 0;

    pcap_close(handle);
    return true;
}

void createUdpBindSocket(unsigned short port)
{
    int fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    struct sockaddr_in addr = { AF_INET, htons(port) };

    if (fd == -1) {
        logFile_debug.AppendText("socket error.");
        return;
    }

    if (bind(fd, reinterpret_cast<struct sockaddr *>(&addr), sizeof(addr)) == -1)
        logFile_debug.AppendText("bind error");

    else
        logFile_debug.AppendText("bind udp socket(port=%d) success.", port);
}

bool isNoChangeIP(in_addr_t *ipaddr1, in_addr_t *ipaddr2)
{
    return *ipaddr1 == *ipaddr2;
}

unsigned long htonLONGLONG(unsigned long val)
{
    return bswap_64(val);
}

void stop_dhclient_asyn()
{
    killProcess("dhclient");
}

bool dhclient_asyn(const char *ipaddr, sem_t *semaphore)
{
    struct DHClientThreadStruct *arg = new struct DHClientThreadStruct;
    pthread_t thread_id;
    stop_dhclient_asyn();
    Sleep(1000);
    strcpy(arg->ipaddr, ipaddr);
    arg->semaphore = semaphore;

    if (pthread_create(&thread_id, 0, dhclient_thread, arg)) {
        delete arg;
        return false;
    }

    return true;
}

void *dhclient_thread(void *varg)
{
    struct DHClientThreadStruct *arg =
            static_cast<struct DHClientThreadStruct *>(varg);

    if (get_os_type() != OS_FEDORA || isFileExist("/sbin/dhclient-script")) {
        if (get_os_type() != OS_FEDORA)
            g_log_Wireless.AppendText("%s file is no exist.", "/sbin/dhclient-script");

        g_log_Wireless.AppendText("sfFile NULL");
        system(
            std::string("dhclient ")
            .append(arg->ipaddr)
            /*.append(" 2>&-")*/
            .c_str()
        );
        sem_post(arg->semaphore);
        delete arg;
        return nullptr;
    }

    chmod("/sbin/dhclient-script", 0755);
    chmod("/sbin/dhclient-script", 0751);
    addStringOnLineHead(
        "/sbin/dhclient-script",
        "/sbin/rjsu-dhclient-script",
        "ip link set ${interface} down",
        "#"
    );
    addStringOnLineHead(
        "/sbin/dhclient-script",
        "/sbin/rjsu-dhclient-script",
        "ip link set $interface down",
        "#"
    );
    addStringOnLineHead(
        "/sbin/dhclient-script",
        "/sbin/rjsu-dhclient-script",
        "ifconfig $interface inet 0 down",
        "#"
    );
    addStringOnLineHead(
        "/sbin/dhclient-script",
        "/sbin/rjsu-dhclient-script",
        "ifconfig ${interface} inet 0 down",
        "#"
    );
    g_log_Wireless.AppendText("sfFile:%s", "/sbin/rjsu-dhclient-script");
    system(
        std::string("dhclient -sf ")
        .append("/sbin/rjsu-dhclient-script")
        .append(arg->ipaddr)
        /*.append(" 2>&-")*/
        .c_str()
    );
    sem_post(arg->semaphore);
    delete arg;
    return nullptr;
}

void dhclient_exit()
{
    system("dhclient -x");
}

void disable_enable_nic(const char *ifname)
{
    // ifconfig $ifname down
    // ifconfig $ifname up
    struct ifreq ifr = {};
    strncpy(ifr.ifr_name, ifname, IFNAMSIZ);
    int fd = socket(AF_INET, SOCK_DGRAM, 0);

    if (fd == -1)
        return;

#define EXIT_ON_FAIL(expr) do { if (expr) { close(fd); return; } } while(0)
    EXIT_ON_FAIL(ioctl(fd, SIOCGIFFLAGS, &ifr));
    ifr.ifr_flags &= ~IFF_UP;
    EXIT_ON_FAIL(ioctl(fd, SIOCSIFFLAGS, &ifr));
    ifr.ifr_flags |= IFF_UP;
    EXIT_ON_FAIL(ioctl(fd, SIOCSIFFLAGS, &ifr));
#undef EXIT_ON_FAIL
    close(fd);
}

void get_all_nics_statu(std::vector<struct NICsStatus> &dest)
{
    bool in_list = false;
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    struct ifaddrs *ifap = nullptr;

    if (fd == -1)
        return;

    if (getifaddrs(&ifap) == -1 || !ifap) {
        freeifaddrs(ifap);
        close(fd);
        return;
    }

    for (
        struct ifaddrs *cur_if = ifap;
        cur_if;
        cur_if = cur_if->ifa_next, in_list = false
    ) {
        for (const struct NICsStatus &nic : dest) {
            if (!strcmp(nic.nic_name, cur_if->ifa_name))
                in_list = true;

            break;
        }

        if (in_list)
            continue;

        switch (check_nic_status(cur_if->ifa_name)) {
            case ADAPTER_INVALID:
                rj_printf_debug("%s check_nic_status error", cur_if->ifa_name);
                dest.emplace_back(cur_if->ifa_name, true);
                break;

            case ADAPTER_UP:
                rj_printf_debug("%s check_nic_status enable", cur_if->ifa_name);
                dest.emplace_back(cur_if->ifa_name, true);
                break;

            case ADAPTER_DOWN:
                rj_printf_debug("%s check_nic_status disable", cur_if->ifa_name);
                dest.emplace_back(cur_if->ifa_name, true);
                break;
        }
    }

    freeifaddrs(ifap);
    close(fd);
}

void get_and_set_gateway(in_addr_t *gatewayd, const char *ifname)
{
    // yes, the codes are from check_manualip_indirectory
    // original code:
    // [ ! -d /etc/sysconfig/networking/devices/ ] && exit
    // grep DEVICE=$ifname /etc/sysconfig/networking/*/* 2>&-
    // split with ':', get the first field
    // and for each do
    // ipaddr=$(cat $first_field|awk  -F = '{if ($1~/IPADDR/) print $2}')
    // if not empty then
    // netmask=$(cat $first_field|awk  -F = '{if ($1~/NETMASK/) print $2}')
    // if not empty then
    // ifconfig $ifname $ipaddr netmask $netmask
    // printf "____%s [set ip mask]%s________\n" "get_gateway_from_file" $
    // "ifconfig $ifname $ipaddr netmask $netmask"
    // gateway=$(cat $first_field|awk  -F = '{if ($1~/GATEWAY/) print $2}')
    // if not empty then
    // printf "%s GATEWAY:%d:%d:%d:%d;\n" "get_gateway_from_file" $
    // $gateway_splited
    // route add default gw $gateway 2>&-
    DIR *rootdir = opendir("/etc/sysconfig/networking");
    DIR *subdir = nullptr;
    std::ifstream ifs;
    std::string line;
    std::string device_line("DEVICE=");
    std::string ipaddr, netmask, gateway;
    std::string sdir("/etc/sysconfig/networking");
    std::vector<std::string> val;
    struct ifreq ifr = {};
    struct rtentry route = {};
    bool exit_ok = false;
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    device_line.append(ifname);

    if (access("/etc/sysconfig/networking/devices/", F_OK) == -1)
        return;

    if (!rootdir)
        return;

    for (struct dirent *dent = readdir(rootdir); dent; dent = readdir(rootdir)) {
        if (dent->d_type != DT_DIR)
            continue;

        subdir = opendir((sdir + dent->d_name).c_str());

        if (!subdir)
            continue;

        for (struct dirent *sdent = readdir(subdir); sdent; sdent = readdir(subdir)) {
            if (sdent->d_type != DT_REG)
                continue;

            ifs.open(sdent->d_name);

            if (!ifs)
                continue;

            while (std::getline(ifs, line)) {
                if (line.compare(device_line))
                    break;

                ParseString(line, '=', val);

                if (val[0] == "IPADDR")
                    ipaddr = val[1];

                if (val[1] == "NETMASK")
                    netmask = val[1];

                if (val[1] == "GATEWAY")
                    gateway = val[1];
            }

            if (!ipaddr.empty() && !netmask.empty()) {
                strcpy(ifr.ifr_name, ifname);
                reinterpret_cast<struct sockaddr_in *>
                (&ifr.ifr_addr)->sin_addr.s_addr = inet_addr(ipaddr.c_str());
                reinterpret_cast<struct sockaddr_in *>
                (&ifr.ifr_netmask)->sin_addr.s_addr = inet_addr(netmask.c_str());
                // may be this is a inline function's name
                std::cout << "____" << "get_gateway_from_file"
                          << " [set ip mask]" << "ifconfig " << ifname << " "
                          << ipaddr << " netmask " << netmask
                          << " ________" << std::endl;
                exit_ok = true;
            }

            if (!gateway.empty()) {
                reinterpret_cast<struct sockaddr_in *>
                (&route.rt_gateway)->sin_addr.s_addr = *gatewayd =
                        inet_addr(gateway.c_str());
                std::cout << "get_gateway_from_file GATEWAY:"
                          << reinterpret_cast<char *>(gatewayd)[0]
                          << ':' << reinterpret_cast<char *>(gatewayd)[1]
                          << ':' << reinterpret_cast<char *>(gatewayd)[2]
                          << ':' << reinterpret_cast<char *>(gatewayd)[3]
                          << ';' << std::endl;
                ioctl(fd, SIOCADDRT, &route);
                exit_ok = true;
            }

            if (exit_ok) {
                close(fd);
                return;
            }

            ifs.close();
        }

        closedir(subdir);
        subdir = nullptr;
        continue;
    }

    close(fd);
    closedir(rootdir);
    return;
}

bool IsEqualDhcpInfo(
    const struct DHCPIPInfo &info1,
    const struct DHCPIPInfo &info2
)
{
    return
        info1.dhcp == info2.dhcp &&
        info1.ip4_ipaddr == info2.ip4_ipaddr &&
        info1.ip4_netmask == info2.ip4_netmask;
}

void InitDHCPIPInfo(struct DHCPIPInfo &info)
{
    info.dns = 0;
    info.gateway = 0;
    info.ip4_ipaddr = 0;
    info.ip4_netmask = 0;
    info.dhcp = true;
}

void InitDhcpIpInfo(struct DHCPIPInfo &info)
{
    info.dns = 0;
    info.gateway = 0;
    info.ip4_ipaddr = 0;
    info.ip4_netmask = 0;
    info.field_24 = {};
    info.ip6_link_local_ipaddr = {};
    info.ip6_ipaddr = {};
    info.dhcp = true;
    info.ipaddr6_count = 0;
    info.adapter_mac = {};
}

bool GetDHCPIPInfo(struct DHCPIPInfo &info, bool)
{
    struct NICINFO *nic_info = nullptr;
    InitDhcpIpInfo(info);
    g_dhcpDug.AppendText("Adapter name:%s", CtrlThread->field_240.field_38);
    nic_info = get_nics_info(CtrlThread->field_240.field_38);

    if (!nic_info)
        return false;

    info.dhcp = CtrlThread->field_240.field_54;
    info.adapter_mac = nic_info->hwaddr;
    info.dns = htonl(nic_info->dns);
    info.gateway = htonl(nic_info->gateway);
    info.gateway_mac = nic_info->gateway_mac;

    if (nic_info->ipaddrs->ipaddr) {
        info.ip4_ipaddr = nic_info->ipaddrs->ipaddr;
        info.ip4_netmask = nic_info->ipaddrs->netmask;
    }

    info.ipaddr6_count = nic_info->ipaddr6_count;

    for (
        struct NICINFO::IP6AddrNode *cip = nic_info->ip6addrs;
        cip;
        cip = cip->next
    ) {
        swapipv6(&cip->ipaddr);
        swapipv6(&cip->netmask);
        info.ip6_netmask = cip->netmask;

        if (cip->ipaddr.s6_addr[0] == 0xFE) {
            if (
                cip->ipaddr.s6_addr[1] & 0xC0 == 0x80 &&
                !info.ip6_link_local_ipaddr.s6_addr[0] &&
                !info.ip6_link_local_ipaddr.s6_addr[1]
            )
                info.ip6_link_local_ipaddr = cip->ipaddr;

            else if (
                cip->ipaddr.s6_addr[1] & 0xC0 == 0xC0 &&
                !info.ip6_ipaddr.s6_addr[0] &&
                !info.ip6_ipaddr.s6_addr[1]
            )
                info.ip6_ipaddr = cip->ipaddr;

        } else if (
            cip->ipaddr.s6_addr[0] & 0xE0 == 0x20 &&
            !info.ip6_ipaddr.s6_addr[0] &&
            !info.ip6_ipaddr.s6_addr[1]
        )
            info.ip6_ipaddr = cip->ipaddr;
    }

    free_nics_info(nic_info);
    return true;
}

void repair_ip_gateway(
    const struct DHCPIPInfo &info,
    const std::string &adapter_name
)
{
    // ifconfig $adapter_name $info.ip4_ipaddr netmask $info.ip4_netmask
    // route add default gw info.gateway 2>&-
    struct ifreq ifr = {};
    strncpy(ifr.ifr_name, adapter_name.c_str(), IFNAMSIZ);
    struct rtentry route = {};
    int fd = socket(AF_INET, SOCK_DGRAM, 0);

    if (fd == -1)
        return;

#define EXIT_ON_FAIL(expr) do { if (expr) { close(fd); return; } } while(0)
    reinterpret_cast<struct sockaddr_in *>
    (&ifr.ifr_addr)->sin_addr.s_addr = htonl(info.ip4_ipaddr);
    EXIT_ON_FAIL(ioctl(fd, SIOCSIFADDR, &ifr));
    reinterpret_cast<struct sockaddr_in *>
    (&ifr.ifr_addr)->sin_addr.s_addr = htonl(info.ip4_netmask);
    EXIT_ON_FAIL(ioctl(fd, SIOCSIFNETMASK, &ifr));
    reinterpret_cast<struct sockaddr_in *>
    (&route.rt_gateway)->sin_addr.s_addr = htonl(info.gateway);
    EXIT_ON_FAIL(ioctl(fd, SIOCADDRT, &route));
#undef EXIT_ON_FAIL
    close(fd);
}

void swapipv6(struct in6_addr *addr)
{
    swap128(static_cast<char *>(addr->s6_addr));
}

void del_default_gateway()
{
    // route del default 2>&-
    struct rtentry route = {};
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
#define EXIT_ON_FAIL(expr) do { if (expr) { close(fd); return; } } while(0)
    EXIT_ON_FAIL(ioctl(fd, SIOCDELRT, &route));
#undef EXIT_ON_FAIL
    close(fd);
}
