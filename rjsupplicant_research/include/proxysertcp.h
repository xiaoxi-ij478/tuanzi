#ifndef PROXYSERTCP_H
#define PROXYSERTCP_H

#include "tcp.h"

class ProxySerTcp : public CTcp
{
    public:
        ProxySerTcp(const struct TcpInfo &tcpinfo);
        bool IsExpired();
        int FindClientTcp(
            const struct TCPIP &pkg,
            ProxyClientTcp *,
            unsigned int
        );

    private:
        unsigned long pad1;
        ProxyClientTcp *proxy_client_tcp;
        unsigned long pad2;
        ProxySerTcp *next_proxy_ser_tcp;
        unsigned long creation_time;
};

#endif // PROXYSERTCP_H
