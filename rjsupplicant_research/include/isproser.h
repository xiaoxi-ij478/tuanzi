#ifndef ISPROSER_H_INCLUDED
#define ISPROSER_H_INCLUDED

#include "stdpkgs.h"
#include "proxyclienttcp.h"
#include "proxysertcp.h"

struct tagLocalIP {
    in_addr_t local_ip;
    unsigned long creation_time;
};

class CIsProSer
{
    public:
        CIsProSer();
        ~CIsProSer();

        int Detect(
            unsigned char *pkg,
            unsigned int pkglen
        );
        bool GetFakeMacInfo(
            in_addr_t *ipaddr,
            struct ether_addr *macaddr
        ) const;
        bool Start(
            const char *adapter_name,
            const struct ether_addr *host_mac,
            unsigned int kind
        ) const;
        bool Stop() const;

    private:
        void AddToHRList(ProxySerTcp *server) const;
        void AddToRHList(ProxyClientTcp *client) const;
        void DelFromHRList(ProxySerTcp *server) const;
        void DelFromRHList(ProxyClientTcp *client) const;
        int HandleDataPacket() const;
        int HandleFinAckPacket() const;
        int HandleSynAckPacket() const;
        bool IsFakeMac(const struct ether_header *ehdr, const struct iphdr *iphdr) const;
        bool IsIPInLocalIPTable(in_addr_t ipaddr) const;
        void OnTimer(int a2, int a3) const;
        void UpdateLocalIPTable() const;

        unsigned int kind;
        bool host_addr_set;
        char adapter_name[256];
        struct ether_addr hostmac;
        unsigned int check_status;
        struct ether_addr detected_fakemac;
        in_addr_t detected_fakeipaddr;
        unsigned int updated_times;
        unsigned long last_updated_time;
        std::vector<struct tagLocalIP> local_ips;
        struct TCPIP tcpip_info;
        ProxyClientTcp *clienttcp_next;
        ProxyClientTcp *clienttcp_prev;
        ProxySerTcp *sertcp_next;
        ProxySerTcp *sertcp_prev;
};

#endif // ISPROSER_H_INCLUDED
