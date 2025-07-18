#ifndef ISPROSER_H_INCLUDED
#define ISPROSER_H_INCLUDED

#include "stdpkgs.h"
class ProxyClientTcp;
class ProxySerTcp;

struct tagLocalIP {
    tagLocalIP(in_addr_t local_ip, unsigned long creation_time) :
        local_ip(local_ip),
        creation_time(creation_time)
    {}
    in_addr_t local_ip;
    unsigned long creation_time;
};

class CIsProSer
{
    public:
        CIsProSer();
        ~CIsProSer();

        int Detect(const char *pkg, unsigned pkglen);
        bool GetFakeMacInfo(in_addr_t *ipaddr, struct ether_addr *macaddr) const;
        void OnTimer(int, int);
        bool Start(
            const char *adapter_name_l,
            const struct ether_addr *hostmac_l,
            unsigned kind_l
        );
        bool Stop();

    private:
        void AddToHRList(ProxySerTcp *server);
        void AddToRHList(ProxyClientTcp *client);
        void DelFromHRList(ProxySerTcp *server);
        void DelFromRHList(ProxyClientTcp *client);
        int HandleDataPacket();
        int HandleFinAckPacket();
        int HandleSynAckPacket();
        bool IsFakeMac(
            const struct ether_header *ehdr,
            const struct iphdr *iphdr
        );
        bool IsIPInLocalIPTable(in_addr_t ipaddr);
        void UpdateLocalIPTable();

        unsigned kind;
        bool host_addr_set;
        char adapter_name[256];
        struct ether_addr hostmac;
        unsigned check_status;
        struct ether_addr detected_fakemac;
        in_addr_t detected_fakeipaddr;
        unsigned updated_times;
        unsigned long last_updated_time;
        std::vector<struct tagLocalIP> local_ips;
        struct TCPIP tcpip_info;
        ProxyClientTcp *clienttcp_begin;
        ProxyClientTcp *clienttcp_end;
        ProxySerTcp *sertcp_begin;
        ProxySerTcp *sertcp_end;
};

#endif // ISPROSER_H_INCLUDED
