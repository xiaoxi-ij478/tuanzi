#ifndef PROXYSERTCP_H_INCLUDED
#define PROXYSERTCP_H_INCLUDED

#include "tcp.h"

class ProxyClientTcp;
class CIsProSer;

class ProxySerTcp : public CTcp
{
        friend class ProxyClientTcp;
        friend class CIsProSer;

    public:
        ProxySerTcp(const struct TcpInfo &tcpinfo);

        int FindClientTcp(
            const struct TCPIP &pkg,
            ProxyClientTcp *client,
            unsigned flag
        );
        bool IsExpired(unsigned a2) const;

    private:
        ProxyClientTcp *bound_proxy_client_tcp;
        // a doubly linked list
        ProxySerTcp *prev;
        ProxySerTcp *next;
        unsigned long creation_time;
};

#endif // PROXYSERTCP_H_INCLUDED
