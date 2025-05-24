#ifndef PROXYSERTCP_H
#define PROXYSERTCP_H

#include "tcp.h"

class ProxyClientTcp;

class ProxySerTcp : public CTcp
{
        friend class ProxyClientTcp;

    public:
        ProxySerTcp(const struct TcpInfo &tcpinfo);

        int FindClientTcp(
            const struct TCPIP &pkg,
            ProxyClientTcp *client,
            unsigned flag
        );
        bool IsExpired() const;

    private:
//        unsigned long pad1;
        ProxyClientTcp *bound_proxy_client_tcp;
        // a doubly linked list
        ProxySerTcp *prev;
        ProxySerTcp *next;
        unsigned long creation_time;
};

#endif // PROXYSERTCP_H
