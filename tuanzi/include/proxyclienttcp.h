#ifndef PROXYCLIENTTCP_H_INCLUDED
#define PROXYCLIENTTCP_H_INCLUDED

#include "tcp.h"

class ProxySerTcp;
class CIsProSer;

class ProxyClientTcp : public CTcp
{
    public:
        ProxyClientTcp(const struct TcpInfo &tcpinfo);

        int TryDetectTCPIP(
            const struct TCPIP &pkg,
            const ProxySerTcp *server,
            unsigned flag
        );
        bool IsExpired(unsigned) const;

    private:
        int FindSerTcp(const ProxySerTcp *server);

        int HandleFtp(const struct TCPIP &pkg, const ProxySerTcp *server);
        int HandleHttp(const struct TCPIP &pkg, const ProxySerTcp *server);
        int HandleMms(const struct TCPIP &pkg, const ProxySerTcp *server);
        int HandleNntp(const struct TCPIP &pkg, const ProxySerTcp *server);
        int HandlePop3(const struct TCPIP &pkg, const ProxySerTcp *server);
        int HandleSocks4(const struct TCPIP &pkg, const ProxySerTcp *server);
        int HandleSocks4A(const struct TCPIP &pkg, const ProxySerTcp *server);
        int HandleSocks5(const struct TCPIP &pkg, const ProxySerTcp *server);
        int HandleTelnet(const struct TCPIP &pkg, const ProxySerTcp *server);

        unsigned field_190;
        bool found_ser;
        unsigned find_ser_times; // m_findSerTimes

    public:
        ProxySerTcp *bound_proxy_ser_tcp;
        ProxyClientTcp *prev;
        ProxyClientTcp *next;
        unsigned long creation_time;
};

#endif // PROXYCLIENTTCP_H_INCLUDED
