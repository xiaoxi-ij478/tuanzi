#include "timeutil.h"
#include "global.h"
#include "dnsquery.h"
#include "util.h"
#include "proxyclienttcp.h"

ProxyClientTcp::ProxyClientTcp(const struct TcpInfo &info) :
    CTcp(info),
    bound_proxy_ser_tcp(),
    field_190(),
    found_ser(),
    find_ser_times(),
    creation_time(GetTickCount()),
    prev(),
    next()
{}

int ProxyClientTcp::TryDetectTCPIP(
    const struct TCPIP &pkg,
    const ProxySerTcp *server,
    unsigned int flag
)
{
}

bool ProxyClientTcp::IsExpired() const
{
}

int ProxyClientTcp::FindSerTcp(const ProxySerTcp *server)
{
    in_addr_t laddr[13] = {};

    if (reqaddr_int != -1) {
        if (!server)
            return 0;

        while (
            server->request_type > 0 ||
            server->tcpinfo.srcport != reqport ||
            reqaddr_int != server->tcpinfo.srcaddr
        ) {
            server = server->next;

            if (!server)
                return 0;
        }

        return 1;
    }

    if (!hostent) {
        if (dns_queryer.QueryByName(reqaddr_char, &hostent))
            return -1;
    }

    if (!hostent->hostent_entry.h_addr_list || !server)
        return 0;

    for (; server; server = server->next) {
        if (server->request_type > 0 || server->tcpinfo.srcport != reqport)
            continue;

        if (!*hostent->hostent_entry.h_addr_list)
            continue;

        for (const char **addr = hostent->hostent_entry.h_addr_list; *addr; addr++) {
            memcpy(laddr, *addr, hostent->hostent_entry.h_length);

            if (server->tcpinfo.srcaddr == laddr[0])
                return 1;
        }
    }

    return 0;
}

int ProxyClientTcp::HandleFtp(
    const struct TCPIP &pkg,
    const ProxySerTcp *server
)
{
    unsigned r2h_trans_times_l = 0, h2r_trans_times_l = 0;
    enum TRANS_DIRECTION t_dir = TRANS_MINE;

    if (reqaddr_char[0]) {
        if (!found_ser) {
            QueryTransTimes(r2h_trans_times_l, h2r_trans_times_l);

            if (
                (r2h_trans_times_l != 1 || h2r_trans_times_l != 2) &&
                (r2h_trans_times_l != 3 || h2r_trans_times_l != 4)
            )
                return 0;

            if (MemCmpare(pkg.content, 0, 2, "331", strlen("331")))
                return 0;

            found_ser = true;
        }

        return bound_proxy_ser_tcp ? 1 : FindSerTcp(server);
    }

    t_dir = QueryTransTimes(r2h_trans_times_l, h2r_trans_times_l);

    if (h2r_trans_times_l == 1) {
        if (r2h_trans_times_l != 1)
            return r2h_trans_times_l > 3 ? 0 : -1;

        if (t_dir != TRANS_R2H)
            return 0;

        GetReqAddr_Port(pkg, true);
        return -1;
    }

    if (h2r_trans_times_l != 3)
        return h2r_trans_times_l > 3 ? 0 : -1;

    if (r2h_trans_times_l != 3)
        return r2h_trans_times_l > 3 ? 0 : -1;

    if (t_dir != TRANS_R2H)
        return 0;

    return GetReqAddr_Port(pkg, true) ? -1 : 0;
}

int ProxyClientTcp::HandleHttp(
    const struct TCPIP &pkg,
    const ProxySerTcp *server
)
{
    int res = 0;
    unsigned r2h_trans_times_l = 0, h2r_trans_times_l = 0;

    if (!reqaddr_char[0])
        return GetReqAddr_Port(pkg, true) ? -1 : 0;

    if (!found_ser) {
        QueryTransTimes(r2h_trans_times_l, h2r_trans_times_l);

        if (r2h_trans_times_l > 1 || h2r_trans_times_l > 1)
            return 0;

        else if (r2h_trans_times_l < 1 || h2r_trans_times_l < 1)
            return -1;

        if (
            MemCmpare(pkg.content, 0, pkg.content_length, "HTTP/1.1", strlen("HTTP/1.1")) ||
            MemCmpare(pkg.content, 0, pkg.content_length, "HTTP/1.0", strlen("HTTP/1.0"))
        )
            return 0;

        found_ser = true;
    }

    if (bound_proxy_ser_tcp)
        return 1;

    res = FindSerTcp(server);
    g_logFile_proxy.AppendText(
        "FindSerTcp res=%d,m_findSerTimes=%d",
        res,
        find_ser_times
    );

    if (res)
        return res;

    return find_ser_times == 3 || find_ser_times++ == 4 ? -1 : res;
}

int ProxyClientTcp::HandleMms(
    const struct TCPIP &pkg,
    const ProxySerTcp *server
)
{
    if (!reqaddr_char[0])
        return -GetReqAddr_Port(pkg, true);

    if (!found_ser)
        found_ser = true;

    return bound_proxy_ser_tcp ? 1 : FindSerTcp(server);
}

int ProxyClientTcp::HandleNntp(
    const struct TCPIP &pkg,
    const ProxySerTcp *server
)
{
    unsigned r2h_trans_times_l = 0, h2r_trans_times_l = 0;
    enum TRANS_DIRECTION t_dir = TRANS_MINE;

    if (!reqaddr_char[0]) {
        t_dir = QueryTransTimes(r2h_trans_times_l, h2r_trans_times_l);

        if (r2h_trans_times_l < 2 || h2r_trans_times_l < 2)
            return -1;

        else if (r2h_trans_times_l > 2 || h2r_trans_times_l > 2)
            return 0;

        return t_dir != TRANS_R2H ? 0 : GetReqAddr_Port(pkg, true) ? -1 : 0;
    }

    if (!found_ser) {
        if (
            pkg.content_length <= 4 ||
            pkg.content[pkg.content_length - 2] != '\r' ||
            pkg.content[pkg.content_length - 1] != '\n'
        )
            return 0;

        QueryTransTimes(r2h_trans_times_l, h2r_trans_times_l);

        if (
            r2h_trans_times_l != 2 ||
            h2r_trans_times_l != 3 ||
            MemCmpare(pkg.content, 0, pkg.content_length - 1, "381", strlen("381"))
        )
            return 0;

        found_ser = true;
        return -1;
    }

    QueryTransTimes(r2h_trans_times_l, h2r_trans_times_l);

    if (r2h_trans_times_l > 3 || h2r_trans_times_l > 3)
        return -1;

    return bound_proxy_ser_tcp ? 1 : FindSerTcp(server);
}

int ProxyClientTcp::HandlePop3(
    const struct TCPIP &pkg,
    const ProxySerTcp *server
)
{
    unsigned r2h_trans_times_l = 0, h2r_trans_times_l = 0;
    enum TRANS_DIRECTION t_dir = TRANS_MINE;

    if (!reqaddr_char[0]) {
        t_dir = QueryTransTimes(r2h_trans_times_l, h2r_trans_times_l);

        if (r2h_trans_times_l < 1 || h2r_trans_times_l < 1)
            return -1;

        else if (r2h_trans_times_l > 1 || h2r_trans_times_l > 1)
            return 0;

        return t_dir != TRANS_R2H ? 0 : GetReqAddr_Port(pkg, true) ? -1 : 0;
    }

    if (!found_ser) {
        if (
            pkg.content_length <= 4 ||
            pkg.content[pkg.content_length - 2] != '\r' ||
            pkg.content[pkg.content_length - 1] != '\n'
        )
            return 0;

        QueryTransTimes(r2h_trans_times_l, h2r_trans_times_l);

        if (
            r2h_trans_times_l != 1 ||
            h2r_trans_times_l != 2 ||
            MemCmpare(pkg.content, 0, pkg.content_length - 1, "+OK", strlen("+OK"))
        )
            return 0;

        found_ser = true;
    }

    return bound_proxy_ser_tcp ? 1 : FindSerTcp(server);
}

int ProxyClientTcp::HandleSocks4(
    const struct TCPIP &pkg,
    const ProxySerTcp *server
)
{
    struct Socks4ConnReq *request=&pkg.content;
    unsigned r2h_trans_times_l = 0, h2r_trans_times_l = 0;
    if(!reqaddr_char[0])
    return GetReqAddr_Port(pkg, true) ? -1 : 0;
    if(found_ser)

    return bound_proxy_ser_tcp ? 1 : FindSerTcp(server);
     QueryTransTimes(r2h_trans_times_l, h2r_trans_times_l);
    if ( r2h_trans_times_l == 1 )
    {
      if ( h2r_trans_times_l == 1 )
      {
        if ( pkg.content_length == 8
          && !(unsigned __int8)*(_QWORD *)a2->content
          && BYTE1(*(_QWORD *)a2->content) == 0x5A )
        {
          v3 = -1;
          this->found_ser = 1;
          return v3;
        }
        return 0;
      }
    }
    else if ( v7[0] > 1 )
    {
      return 0;
    }
    v3 = -1;
    if ( v6 > 1 )
      return 0;
}

int ProxyClientTcp::HandleSocks4A(
    const struct TCPIP &pkg,
    const ProxySerTcp *server
)
{
}

int ProxyClientTcp::HandleSocks5(
    const struct TCPIP &pkg,
    const ProxySerTcp *server
)
{
}

int ProxyClientTcp::HandleTelnet(
    const struct TCPIP &pkg,
    const ProxySerTcp *server
)
{
}
