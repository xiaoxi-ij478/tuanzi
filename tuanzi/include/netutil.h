#ifndef NETUTIL_H_INCLUDED
#define NETUTIL_H_INCLUDED

#include "global.h"

enum ADAPTER_STATUS {
    ADAPTER_INVALID = -1,
    ADAPTER_UP = 1,
    ADAPTER_DOWN,
    ADAPTER_DISABLE,
    ADAPTER_ENABLE,
    ADAPTER_ERROR
};

enum ADAPTER_TYPE {
    ADAPTER_WIRELESS,
    ADAPTER_WIRED
};

struct NICINFO {
    char ifname[IFNAMSIZ];
    struct ether_addr hwaddr;
    bool use_dhcp;
    bool is_wireless;
    unsigned short speed;
    in_addr_t dns;
    in_addr_t gateway;
    char gateway_mac[6];
    unsigned ipaddr_count;
    struct IPAddrNode {
        in_addr_t ipaddr;
        in_addr_t netmask;
        struct IPAddrNode *next;
    } *ipaddrs;
    unsigned ipaddr6_count;
    struct IP6AddrNode {
        struct in6_addr ipaddr;
        struct in6_addr netmask;
        struct IP6AddrNode *next;
    } *ip6addrs;
    struct NICINFO *next;
};

struct DHClientThreadStruct {
    char ipaddr[512];
    sem_t *semaphore;
};

struct DHCPIPInfo {
    unsigned field_0;
    in_addr_t ip4_ipaddr;
    in_addr_t ip4_netmask;
    in_addr_t gateway;
    in_addr_t dns;
    struct ether_addr adapter_mac;
    struct ether_addr gateway_mac;
    unsigned ipaddr6_count;
    struct in6_addr field_24;
    struct in6_addr ip6_link_local_ipaddr;
    struct in6_addr ip6_ipaddr;
    struct in6_addr ip6_netmask;
};

struct NICsStatus {
    NICsStatus(const char *nic_name_l, bool is_up) : is_up(is_up) {
        strncpy(nic_name, nic_name_l, sizeof(nic_name));
    }
    char nic_name[16];
    bool is_up;
};

extern int sockets_open();
extern enum ADAPTER_TYPE get_nic_type(const char *ifname);
extern unsigned short ComputeTcpPseudoHeaderChecksum(
    const struct iphdr *ipheader,
    struct TCPHeader *tcpheader,
    const unsigned char *databuf,
    int length
);
extern unsigned short ComputeUdpPseudoHeaderChecksumV4(
    const struct iphdr *ipheader,
    struct udphdr *udpheader,
    const unsigned char *databuf,
    int length
);
extern unsigned short checksum(const unsigned short *data, unsigned len);
extern struct NICINFO *get_nics_info(const char *ifname);
extern void free_nics_info(struct NICINFO *info);
extern bool get_dns(in_addr_t *dst);
extern bool get_alternate_dns(char *dst, int &counts);
extern bool get_gateway(in_addr_t *result, const char *ifname);
extern unsigned short get_speed_wl(int fd, char *ifname);
extern unsigned short get_speed(int fd, char *ifname);
extern bool check_manualip_indirectory(
    const char *ipaddr,
    const char *dir,
    bool incl_subdir
);
extern bool check_manualip_infile(const char *ipaddr, const char *file);
extern bool check_dhcp(const char *ifname, const char *ipaddr);
extern bool get_ip_mac(in_addr_t ipaddr, struct ether_addr *macaddr);
extern bool check_nic_isok(char *ifname);
extern int check_nic_status(const char *ifname);
extern bool get_nic_in_use(
    std::vector<std::string> &nic_list,
    bool wireless_only
);
extern int get_nic_list(std::vector<std::string>);
extern bool get_nic_speed(char *dst, const char *ifname);
extern bool GetNICInUse(std::vector<std::string> &nic_list, bool wireless_only);
extern unsigned InitIpv4Header(
    unsigned char *header_c,
    const char *srcaddr,
    const char *dstaddr,
    unsigned datalen
);
extern unsigned InitUdpHeader(
    unsigned char *header_c,
    unsigned srcport,
    unsigned dstport,
    unsigned datalen
);
extern void get_and_set_gateway(in_addr_t *gatewayd, const char *ifname);
extern unsigned long htonLONGLONG(unsigned long val);
extern bool Is8021xGroupAddr(struct ether_addr *macaddr);
extern bool IsEqualIP(in_addr_t *ipaddr1, in_addr_t *ipaddr2);
extern bool IsEqualMac(
    struct ether_addr *macaddr1,
    struct ether_addr *macaddr2
);
extern bool IsGetDhcpIpp(in_addr_t *ip);
extern bool IsHostDstMac(struct ether_addr *macaddr);
extern bool IsHostDstMac(
    struct ether_addr *macaddr1,
    struct ether_addr *macaddr2
);
extern bool IsLoopBack(struct ether_addr *macaddr);
extern bool IsMulDstMac(struct ether_addr *macaddr);
extern bool IsStarGroupDstMac(struct ether_addr *macaddr);
extern void createUdpBindSocket(unsigned short port);
extern bool isNoChangeIP(in_addr_t *ipaddr1, in_addr_t *ipaddr2);
extern void stop_dhclient_asyn();
extern bool dhclient_asyn(const char *ipaddr, sem_t *semaphore);
extern void *dhclient_thread(void *varg);
extern void dhclient_exit();
extern void disable_enable_nic(const char *ifname);
extern void get_all_nics_statu(std::vector<struct NICsStatus> &dest);
extern bool SetLanFlag(unsigned flag);
extern void InitSmpInitPacket(struct tagSmpInitPacket &packet);
extern bool IsEqualDhcpInfo(
    const struct DHCPIPInfo &info1,
    const struct DHCPIPInfo &info2
);
extern void InitDHCPIPInfo(struct DHCPIPInfo &info);
extern void InitDhcpIpInfo(struct DHCPIPInfo &info);
extern bool GetDHCPIPInfo(struct DHCPIPInfo &info, bool);
extern void repair_ip_gateway(
    const struct DHCPIPInfo &info,
    const std::string &adapter_name
);
extern void swapipv6(struct in6_addr *addr);
extern void CopyDirTranPara(
    struct tagDirTranPara *dst,
    const struct tagDirTranPara *src
);
extern void CreateDirPktHead(
    struct mtagFinalDirPacket &final_packet_head,
    struct tagDirPacketHead &packet_head,
    struct tagSenderBind &sender_bind,
    unsigned char *buf,
    unsigned buflen,
    unsigned char *keybuf,
    unsigned char *ivbuf
);
extern void CreateSessionIfNecessary(
    struct tagRecvBind &gsn_pkgs,
    in_addr_t srcaddr,
    unsigned session_id,
    struct tagRecvSessionBind &recv_session
);

#endif // NETUTIL_H_INCLUDED
