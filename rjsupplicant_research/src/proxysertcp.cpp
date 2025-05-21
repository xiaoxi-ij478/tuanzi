#include "timeutil.h"
#include "proxysertcp.h"

ProxySerTcp::ProxySerTcp(const TcpInfo &tcpinfo) :
    CTcp(tcpinfo),
    proxy_client_tcp(),
    pad1(),
    pad2(),
    pad3(),
    creation_time(GetTickCount())
{
}

bool ProxySerTcp::IsExpired()
{
    return false;
}

int ProxySerTcp::FindClientTcp(
    const struct TCPIP &pkg,
    ProxyClientTcp *,
    unsigned int
)
{
}
