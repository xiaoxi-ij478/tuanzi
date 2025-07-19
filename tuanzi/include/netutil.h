#ifndef NETUTIL_H_INCLUDED
#define NETUTIL_H_INCLUDED

#include "miscdefs.h"

extern int sockets_open();
extern enum ADAPTER_TYPE get_nic_type(const char *ifname);
extern unsigned short ComputeTcpPseudoHeaderChecksum(
    const struct iphdr *ipheader,
    struct TCPHeader *tcpheader,
    const char *databuf,
    int length
);
extern unsigned short ComputeUdpPseudoHeaderChecksumV4(
    const struct iphdr *ipheader,
    struct udphdr *udpheader,
    const char *databuf,
    int length
);
extern unsigned short checksum(const unsigned short *data, unsigned len);
extern struct NICINFO *get_nics_info(const char *ifname);
extern void free_nics_info(struct NICINFO *info);
extern bool get_dns(in_addr_t *dst);
extern bool get_alternate_dns(char *dst, unsigned &length);
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
extern enum ADAPTER_STATUS check_nic_status(const char *ifname);
extern bool get_nic_in_use(
    std::vector<std::string> &nic_list,
    bool wireless_only
);
extern int get_nic_list(std::vector<std::string>);
extern bool get_nic_speed(char *dst, const char *ifname);
extern bool GetNICInUse(std::vector<std::string> &nic_list, bool wireless_only);
extern unsigned InitIpv4Header(
    struct iphdr *header,
    const char *srcaddr,
    const char *dstaddr,
    unsigned datalen
);
extern unsigned InitUdpHeader(
    struct udphdr *header,
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
extern bool dhclient_asyn(const char *ipaddr, sem_t *semaphore);
extern void *dhclient_thread(void *varg);
extern void disable_enable_nic(const char *ifname);
extern void get_all_nics_statu(std::vector<struct NICsStatus> &dest);
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
extern void del_default_gateway();
extern void get_ssid_list(
    const char *adapter_name,
    std::vector<struct tagWirelessSignal> &signals
);

#endif // NETUTIL_H_INCLUDED
