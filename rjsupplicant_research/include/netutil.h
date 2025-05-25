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
    unsigned char hwaddr[6];
    bool use_dhcp;
    bool is_wireless;
    unsigned short speed;
    struct in_addr dns;
    struct in_addr gateway;
    char unknown[6];
    unsigned ipaddr_count;
    struct IPAddrNode {
        struct in_addr ipaddr;
        struct in_addr netmask;
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

struct NICsStatus {
    NICsStatus(char *nic_name, bool is_up) : is_up(is_up) {
        strncpy(this->nic_name, nic_name, sizeof(this->nic_name));
    }
    char nic_name[16];
    bool is_up;
};

extern int sockets_open();
extern enum ADAPTER_TYPE get_nic_type(const char *ifname);
extern unsigned short ComputeTcpPseudoHeaderChecksum(
    const struct iphdr *ipheader,
    const struct TCPHeader *tcpheader,
    const unsigned char *databuf,
    int length
);
extern unsigned short ComputeUdpPseudoHeaderChecksumV4(
    const struct iphdr *ipheader,
    const struct udphdr *udpheader,
    const unsigned char *databuf,
    int length
);
extern unsigned short checksum(unsigned short *data, unsigned len);
extern struct NICINFO *get_nics_info(const char *ifname);
extern void free_nics_info(struct NICINFO *info);
extern bool get_dns(struct in_addr *dst);
extern bool get_alternate_dns(char *dst, int &counts);
extern bool get_gateway(struct in_addr *result, const char *ifname);
extern unsigned short get_speed_wl(int fd, char *ifname);
extern unsigned short get_speed(int fd, char *ifname);
extern bool check_manualip_indirectory(
    const char *ipaddr,
    const char *dir,
    bool incl_subdir
);
extern bool check_manualip_infile(const char *ipaddr, const char *file);
extern bool check_dhcp(const char *ifname, const char *ipaddr);
extern bool get_ip_mac(struct in_addr ipaddr, unsigned char macaddr[6]);
extern int check_nic_status(const char *ifname);
extern bool get_nic_in_use(
    std::vector<std::string> &nic_list,
    bool wireless_only
);
extern int get_nic_list(std::vector<std::string> list);
extern bool get_nic_speed(char *dst, const char *ifname);
extern bool GetNICInUse(std::vector<std::string> &nic_list, bool wireless_only);
extern unsigned InitIpv4Header(
    char *header_c,
    char *srcaddr,
    char *dstaddr,
    unsigned datalen
);
extern unsigned InitUdpHeader(
    char *header_c,
    int srcport,
    int dstport,
    int datalen
);
extern bool Is8021xGroupAddr(unsigned char macaddr[6]);
extern bool IsEqualIP(unsigned char ipaddr1[4], unsigned char ipaddr2[4]);
extern bool IsEqualMac(unsigned char macaddr1[6], unsigned char macaddr2[6]);
extern bool IsGetDhcpIpp(unsigned char ip[4]);
extern bool IsHostDstMac(unsigned char macaddr1[6], unsigned char macaddr2[6]);
extern bool IsMulDstMac(unsigned char macaddr[6]);
extern bool IsStarGroupDstMac(unsigned char macaddr[6]);
extern void createUdpBindSocket(unsigned short port);
extern bool isNoChangeIP(unsigned char ipaddr1[4], unsigned char ipaddr2[4]);
extern void stop_dhclient_asyn();
extern bool dhclient_asyn(const char *ipaddr, sem_t *semaphore);
extern void *dhclient_thread(void *varg);
extern void dhclient_exit();
extern void disable_enable_nic(const char *ifname);
extern void get_all_nics_statu(std::vector<struct NICsStatus> &dest);

#endif // NETUTIL_H_INCLUDED
