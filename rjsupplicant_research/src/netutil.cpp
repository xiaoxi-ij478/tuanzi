#include "global.h"
#include "cmdutil.h"
#include "util.h"
#include "netutil.h"

int sockets_open()
{
#define TRY_CREATE_AND_RETURN(domain, type) \
    do { \
        int result; \
        if ((result = socket(domain, type, 0)) != -1) \
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
    struct iwreq iwr = { 0 };

    if (fd < 0) {
        perror("get_nic_type __ sockets_open");
        return ADAPTER_WIRELESS;
    }

    strncpy(iwr.ifr_ifrn.ifrn_name, ifname, IFNAMSIZ - 1);
    return ioctl(fd, SIOCGIWNAME, &iwr) >= 0 ? ADAPTER_WIRELESS : ADAPTER_WIRED;
}

unsigned short ComputeTcpPseudoHeaderChecksum(
    const struct _IPHeader *ipheader,
    const struct _TCPHeader *tcpheader,
    const unsigned char *databuf,
    int length
)
{
    struct _TCPChecksumHeader header = { 0 };
    memset(&header, 0, sizeof(header));
#define SET_PSEUDO_HEADER_INFO(name) header.pseudo_header.name = ipheader->name
    SET_PSEUDO_HEADER_INFO(srcaddr);
    SET_PSEUDO_HEADER_INFO(dstaddr);
    SET_PSEUDO_HEADER_INFO(protocol);
    header.pseudo_header.tcp_length = length + sizeof(header.header);
#define SET_HEADER_INFO(name) header.header.name = tcpheader->name
    SET_HEADER_INFO(srcport);
    SET_HEADER_INFO(dstport);
    SET_HEADER_INFO(seq);
    SET_HEADER_INFO(ack);
    SET_HEADER_INFO(offset);
    SET_HEADER_INFO(reserved);
    SET_HEADER_INFO(flags);
    SET_HEADER_INFO(window);
    SET_HEADER_INFO(urgent_pointer);
#undef SET_HEADER_INFO
#undef SET_PSEUDO_HEADER_INFO
    memcpy(header.data, databuf, length);
    return checksum(
               reinterpret_cast<unsigned short *>(&header),
               length + sizeof(header.header) + sizeof(header.pseudo_header)
           );
}

unsigned short ComputeUdpPseudoHeaderChecksumV4(
    const struct _IPHeader *ipheader,
    const struct udp_hdr *udpheader,
    const unsigned char *databuf,
    int length
)
{
    udp_checksum_hdr header;
    memset(&header, 0, sizeof(header));
#define SET_PSEUDO_HEADER_INFO(name) header.pseudo_hdr.name = ipheader->name
    SET_PSEUDO_HEADER_INFO(srcaddr);
    SET_PSEUDO_HEADER_INFO(dstaddr);
    SET_PSEUDO_HEADER_INFO(protocol);
    header.pseudo_hdr.srcaddr = length + sizeof(header.hdr);
#define SET_HEADER_INFO(name) header.hdr.name = udpheader->name
    SET_HEADER_INFO(srcport);
    SET_HEADER_INFO(dstport);
    SET_HEADER_INFO(length);
#undef SET_HEADER_INFO
#undef SET_PSEUDO_HEADER_INFO
    memcpy(header.data, databuf, length);
    return checksum(
               reinterpret_cast<unsigned short *>(&header),
               length + sizeof(header.hdr) + sizeof(header.pseudo_hdr)
           );
}

unsigned short checksum(unsigned short *data, unsigned int len)
{
    unsigned int checksum = 0;

    // add up all the complete words
    for (unsigned int i = 0; i < len >> 1; i++)
        checksum += *data++;

    // add the additional one padding to word if exists
    // but the original implementation uses
    if (len & 1)
        checksum += *reinterpret_cast<unsigned char *>(data);

    // repeatedly take the high 16 bit and add to the low 16 bit
    // until the high 16 bit is 0
    while (checksum >> 16)
        checksum = (checksum & 0xffff) + (checksum >> 16);

    return ~checksum;
}

struct NICINFO *get_nics_info(const char *ifname)
{
    struct NICINFO *info = nullptr;
    struct NICINFO *cur_info = nullptr;
    struct NICINFO *tmp_info = nullptr;
    struct ifaddrs *ifap = nullptr;
    struct NICINFO::IPAddrNode *tmp_ipnode = nullptr, *tmp2_ipnode = nullptr;
    struct NICINFO::IP6AddrNode *tmp_ip6node = nullptr, *tmp2_ip6node = nullptr;
    int fd = 0;
    bool interface_added = false;
    struct ifreq ifr = { 0 };
    struct in_addr dns_addr = { 0 };

    if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
        return nullptr;

    if (getifaddrs(&ifap) == -1 || !ifap) {
        close(fd);
        return nullptr;
    }

    if (get_dns(&dns_addr))
    {}

//        swap32(reinterpret_cast<unsigned char *>(&dns_addr.s_addr));

    if (!ifap) {
        /* freeifaddrs(ifap); */ /* ???? */
        close(fd);
        return nullptr;
    }

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
            strncpy(ifr.ifr_ifrn.ifrn_name, cur_if->ifa_name, IFNAMSIZ - 1);

            if (ioctl(fd, SIOCGIFHWADDR, &ifr))
                memset(&cur_info->hwaddr, 0, sizeof(cur_info->hwaddr));

            else
                memcpy(
                    &cur_info->hwaddr,
                    &ifr.ifr_ifru.ifru_hwaddr.sa_data,
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
                memset(cur_info->unknown, 0, sizeof(cur_info->unknown));
//                swap32(reinterpret_cast<unsigned char *>(&cur_info->gateway.s_addr));

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
                    reinterpret_cast<struct sockaddr_in *>(cur_if->ifa_addr)
                    ->sin_addr;
//                swap32(reinterpret_cast<unsigned char *>(&tmp_ipnode->ipaddr.s_addr));
                tmp_ipnode->netmask =
                    reinterpret_cast<struct sockaddr_in *>(cur_if->ifa_netmask)
                    ->sin_addr;

//                swap32(reinterpret_cast<unsigned char *>(&tmp_ipnode->netmask.s_addr));
                if (!cur_info->ipaddrs) {
                    cur_info->ipaddrs = tmp_ipnode;
                    cur_info->use_dhcp = check_dhcp(cur_if->ifa_name,
                                                    inet_ntoa(tmp_ipnode->ipaddr));

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
                    reinterpret_cast<struct sockaddr_in6 *>(cur_if->ifa_addr)
                    ->sin6_addr;
//                swap128(reinterpret_cast<unsigned char *>(tmp_ip6node->ipaddr));
                tmp_ip6node->netmask =
                    reinterpret_cast<struct sockaddr_in6 *>(cur_if->ifa_netmask)
                    ->sin6_addr;

//                swap128(reinterpret_cast<unsigned char *>(tmp_ip6node->netmask));

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
    for (
        struct NICINFO *inn = info->next;
        info;
        info = inn, inn = info ? nullptr : info->next
    ) {
        for (
            struct NICINFO::IPAddrNode
            *n = info->ipaddrs,
            *nn = n->next;
            n;
            n = nn, nn = n ? nullptr : n->next
        )
            delete n;

        for (
            struct NICINFO::IP6AddrNode
            *n = info->ip6addrs,
            *nn = n->next;
            n;
            n = nn, nn = n ? nullptr : n->next
        )
            delete n;

        delete info;
    }
}

bool get_dns(struct in_addr *dst)
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
        ParseString(line, ' ',val);

        if (val[0] == "nameserver")
            break;
    }

    ifs.close();

    if (!found)
        return false;

    inet_pton(AF_INET, val[1].c_str(), &dst->s_addr);
    return true;
}

bool get_gateway(struct in_addr *result, const char *ifname)
{
    // cat /proc/net/route 2>&- |awk '{if ($1~/$ifname/ && $2~/00000000/) print $3}'
    std::ifstream ifs("/proc/net/route");
    std::string line;
    std::vector<std::string> arr;
    unsigned int pos1 = 0, pos2 = 0, pos3 = 0;

    if (!ifs)
        return false;

    while (std::getline(ifs, line)) {
        ParseString(line, '\t',arr);

        if (arr[0] == ifname && arr[1] == "00000000") {
            result->s_addr = std::stoi(arr[2], nullptr, 16);
            ifs.close();
            return true;
        }
    }

    ifs.close();
    return false;
}

unsigned short get_speed_wl(int fd, char *ifname)
{
    struct iwreq iwr = { 0 };
    int ret = 0;
    strncpy(iwr.ifr_ifrn.ifrn_name, ifname, IFNAMSIZ - 1);
    return ioctl(fd, SIOCGIWRATE, &iwr) ? 0 : iwr.u.bitrate.value / 1000000;
}

unsigned short get_speed(int fd, char *ifname)
{
    struct ethtool_cmd ecmd = { 0 };
    struct ifreq ifr = { 0 };
    ecmd.cmd = ETHTOOL_GSET;
    strncpy(ifr.ifr_ifrn.ifrn_name, ifname, IFNAMSIZ - 1);
    ifr.ifr_data = reinterpret_cast<__caddr_t>(&ecmd);
    return ioctl(fd, SIOCETHTOOL, &ifr) ? 0 : ethtool_cmd_speed(&ecmd);
}

static bool check_manualip_indirectory(
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
    std::string sdir = dir;

    if (!rootdir)
        return false;

    for (struct dirent *dent = readdir(rootdir); dent; dent = readdir(rootdir)) {
        if (dent->d_type != DT_REG)
            continue;

        if (incl_subdir && dent->d_type == DT_DIR) {
            subdir = opendir((sdir + dent->d_name).c_str());

            if (!subdir)
                continue;

            for (struct dirent *sdent = readdir(subdir); sdent; sdent = readdir(subdir)) {
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

static bool check_manualip_infile(const char *ipaddr, const char *file)
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

bool check_dhcp(const char *ifname, const char *ipaddr)
{
    if (!ipaddr || !strcmp(ipaddr, "0.0.0.0"))
        return true;

    return !(
               check_manualip_indirectory(
                   ipaddr,
                   "/etc/sysconfig/networking/",
                   true
               ) ||
               check_manualip_indirectory(
                   ipaddr,
                   "/etc/sysconfig/network-scripts/",
                   false
               ) ||
               check_manualip_infile(
                   ipaddr,
                   "/etc/network/interfaces"
               ) ||
               check_manualip_indirectory(
                   ipaddr,
                   "/etc/NetworkManager/system-connections/",
                   false
               )
           );
}

bool get_ip_mac(struct in_addr ipaddr, unsigned char macaddr[6])
{
    std::string empty_mac = "00:00:00:00:00:00", full_mac = "ff:ff:ff:ff:ff:ff";
    std::string ip, mac;
    std::string line;
    std::ifstream ifs("/proc/net/arp");
    std::string::iterator b, e;
    std::string testip = inet_ntoa(ipaddr);
    std::vector<std::string> val;
    unsigned long pos1 = 0, pos2 = 0, pos3 = 0, pos4 = 0;
    // minimal-ping related
    int fd = socket(AF_INET, SOCK_RAW, getprotobyname("icmp")->p_proto);
    struct icmp_pkg package = { 0 }, recv_package = { 0 };
    struct sockaddr_in dest_addr = { AF_INET, 0, ipaddr };
    struct timeval wait_sec = { 3, 0 };
    unsigned int dest_addrlen = sizeof(dest_addr);
    fd_set listen_fds;

    if (!ifs)
        return false;

    while (std::getline(ifs, line)) {
        ParseString(line, ' ',val);

        if (val[0] != testip || val[3] == empty_mac || val[3] == full_mac)
            continue;

        sscanf(
            val[3].c_str(),
            "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
            &macaddr[0], &macaddr[1], &macaddr[2],
            &macaddr[3], &macaddr[4], &macaddr[5]
        );
        ifs.close();
        return true;
    }

    ifs.close();
    // minimal ping implementation
    // yes, I copied them from inetutils
    FD_ZERO(&listen_fds);
    FD_SET(fd, &listen_fds);
    package.icmp_type = 8; // ICMP_ECHO
    package.icmp_code = 0;
    package.icmp_cksum = 0;
    package.icmp_seq = htons(1);
    package.icmp_id = htons(9527);
    memcpy(
        package.icmp_data,
        "ij478ij478ij478ij478ij478ij478ij478ij478",
        sizeof(package.icmp_data)
    );
    package.icmp_cksum = checksum(
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
    switch (select(1, &listen_fds, nullptr, nullptr, &wait_sec)) {
        case -1: // some error
        case 0: // the host may not be online
            return false;

        case 1:// got response
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
    unsigned short orig_checksum = recv_package.icmp_cksum;
    recv_package.icmp_cksum = 0;

    if (
        checksum(
            reinterpret_cast<unsigned short *>(&recv_package),
            sizeof(recv_package)
        ) != orig_checksum
    )
        return false;

    if (
        recv_package.icmp_type /* != 0 */ ||
        recv_package.icmp_seq != htons(1) ||
        recv_package.icmp_id != htons(9527) ||
        memcmp(
            recv_package.icmp_data,
            "ij478ij478ij478ij478ij478ij478ij478ij478",
            sizeof(recv_package.icmp_data)
        )
    )
        return false;

//    system(std::string("ping -c 1").insert(5, inet_ntoa(ipaddr)).c_str());
    ifs.open("/proc/net/arp");

    if (!ifs)
        return false;

    while (std::getline(ifs, line)) {
        ParseString(line, ' ',val);

        if (val[0] != testip || val[3] == empty_mac || val[3] == full_mac)
            continue;

        sscanf(
            val[3].c_str(),
            "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
            &macaddr[0], &macaddr[1], &macaddr[2],
            &macaddr[3], &macaddr[4], &macaddr[5]
        );
        ifs.close();
        return true;
    }

    ifs.close();
    return false;
}

int check_nic_status(const char *ifname)
{
    int fd = 0;
    struct ifreq ifr = { 0 };
//    struct ethtool_value evalue;

    if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
        return -1;

    strncpy(ifr.ifr_ifrn.ifrn_name, ifname, IFNAMSIZ - 1);
//    evalue.cmd = ETHTOOL_GLINK;
//    ifr.ifr_ifru.ifru_data = &evalue;

    if (ioctl(fd, SIOCGIFFLAGS, &ifr) < 0) {
        close(fd);
        return -1;
    }

    close(fd);
    return ifr.ifr_ifru.ifru_flags & IFF_UP ? ADAPTER_UP : ADAPTER_DOWN;
}

bool get_nic_in_use(std::vector<std::string> &nic_list, bool wireless_only)
{
    struct ifaddrs *ifap = nullptr;
    bool in_list = false;
    int fd = 0;
    nic_list.clear();

    if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
        return false;

    if (getifaddrs(&ifap) == -1 || !ifap) {
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
[[maybe_unused]] int get_nic_list(std::vector<std::string>)
{
    return 0;
}

bool get_nic_speed(char *dst, const char *ifname)
{
    int fd = 0;
    struct ifaddrs *ifap = nullptr;
    unsigned short speed = 0;

    if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
        return false;

    if (getifaddrs(&ifap) == -1 || !ifap) {
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

    return !!speed;
}

bool GetNICInUse(std::vector<std::string> &nic_list, bool wireless_only)
{
    nic_list.clear();
    return get_nic_in_use(nic_list, wireless_only);
}

unsigned int InitIpv4Header(
    char *header_c,
    char *srcaddr,
    char *dstaddr,
    unsigned int datalen
)
{
    struct _IPHeader *header = reinterpret_cast<struct _IPHeader *>(header_c);
    header->version = 4;
    header->ihl = 5;
    header->tos = 0;
    header->total_length = htons(datalen) +
                           sizeof(struct _IPHeader) +
                           sizeof(struct udp_hdr);
    header->ipid = 0;
    header->flags = 0;
    header->fragment_offset = 0;
    header->ttl = 0x80;
    header->protocol = IPPROTO_UDP;
    header->header_checksum = 0;
    header->srcaddr = inet_addr(srcaddr);
    header->dstaddr = inet_addr(dstaddr);
    header->header_checksum = checksum(
                                  reinterpret_cast<unsigned short *>(header_c),
                                  sizeof(struct _IPHeader)
                              );
    return sizeof(struct _IPHeader);
}

unsigned int InitUdpHeader(
    char *header_c,
    int srcport,
    int dstport,
    int datalen
)
{
    struct udp_hdr *header = reinterpret_cast<udp_hdr *>(header_c);
    header->srcport = srcport;
    header->dstport = dstport;
    header->length = datalen + sizeof(struct udp_hdr);
    return sizeof(struct udp_hdr);
}

bool Is8021xGroupAddr(unsigned char macaddr[6])
{
    // 01:80:C2:00:00:03
    unsigned char group_addr[6] = {0x01, 0x80, 0xC2, 0x00, 0x00, 0x03};
    return !memcmp(macaddr, group_addr, sizeof(group_addr));
}

bool IsEqualIP(unsigned char ipaddr1[4], unsigned char ipaddr2[4])
{
    return !memcmp(ipaddr1, ipaddr2, sizeof(char) * 4);
}

bool IsEqualMac(unsigned char macaddr1[6], unsigned char macaddr2[6])
{
    return !memcmp(macaddr1, macaddr2, sizeof(char) * 6);
}

bool IsGetDhcpIpp(unsigned char ip[4])
{
    // !169.254.xxx.xxx ||
    // 0.xxx.xxx.xxx
    return ip[0] ?
           ip[0] != 169 || ip[0] != 254 :
           ip[1] && ip[2] && ip[3];
}

bool IsHostDstMac(unsigned char macaddr1[6], unsigned char macaddr2[6])
{
    return !memcmp(macaddr1, macaddr2, sizeof(char) * 6);
}

bool IsMulDstMac(unsigned char macaddr[6])
{
    // FF:FF:FF:FF:FF:FF
    unsigned char multicast_mac[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    return !memcmp(macaddr, multicast_mac, sizeof(multicast_mac));
}

bool IsStarGroupDstMac(unsigned char macaddr[6])
{
    // 00:D0:F8:00:00:03
    unsigned char star_group_addr[6] = {0x01, 0x80, 0xC2, 0x00, 0x00, 0x03};
    return !memcmp(macaddr, star_group_addr, sizeof(star_group_addr));
}
