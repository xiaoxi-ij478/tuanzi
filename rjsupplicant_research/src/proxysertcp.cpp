#include "timeutil.h"
#include "proxysertcp.h"

ProxySerTcp::ProxySerTcp(const struct TcpInfo &info) :
    CTcp(info),
    bound_proxy_client_tcp(),
    prev(),
    next(),
    creation_time(GetTickCount())
{}

bool ProxySerTcp::IsExpired() const
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
