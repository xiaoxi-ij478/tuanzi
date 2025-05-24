#include "timeutil.h"
#include "global.h"
#include "dnsquery.h"
#include "util.h"
#include "stdpkgs.h"
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
    ProxySerTcp *server,
    unsigned flag
)
{
    int ret = 0;
    GetTickCount();

    if (!pkg.ipheader || !pkg.tcpheader || !pkg.content || !pkg.content_length) {
        g_logFile_proxy.AppendText("ProxyClientTcp::TryDetectTCPIP:parameter error\r\n");
        return -3;
    }

    if (!IsMine(pkg))
        return -2;

    if (!request_type) {
        if ((request_type = QueryProtocolType(pkg, flag)) <= REQUEST_UNKNOWN_0)
            return 0;

        g_logFile_proxy.AppendText("QueryProtocolType:%d\r\n", request_type);
    }

    switch (request_type) {
        case REQUEST_HTTP:
            ret = HandleHttp(pkg, server);
            break;

        case REQUEST_SOCK4:
            ret = HandleSocks4(pkg, server);
            break;

        case REQUEST_SOCK4A:
            ret = HandleSocks4A(pkg, server);
            break;

        case REQUEST_SOCK5:
            ret = HandleSocks5(pkg, server);
            break;

        case REQUEST_FTP:
            ret = HandleFtp(pkg, server);
            break;

        case REQUEST_POP3:
            ret = HandlePop3(pkg, server);
            break;

        case REQUEST_NNTP:
            ret = HandleNntp(pkg, server);
            break;

        case REQUEST_MMS:
            ret = HandleMms(pkg, server);
            break;

        case REQUEST_TELNET:
            ret = HandleTelnet(pkg, server);
            break;

        default:
            ret = 0;
            break;
    }

    return ret == 1 ? request_type : ret;
}

bool ProxyClientTcp::IsExpired() const
{
    return false;
}

int ProxyClientTcp::FindSerTcp(const ProxySerTcp *server)
{
    in_addr_t laddr[13] = {};

    if (reqaddr_int != -1) {
        if (!server)
            return 0;

        while (
            server->request_type > REQUEST_UNKNOWN_0 ||
            server->tcpinfo.srcport != reqport ||
            reqaddr_int != server->tcpinfo.srcaddr
        ) {
            server = server->next;

            if (!server)
                return 0;
        }

        return 1;
    }

    if (!hostent && dns_queryer.QueryByName(reqaddr_char, &hostent))
        return -1;

    if (!hostent->hostent_entry.h_addr_list || !server)
        return 0;

    for (; server; server = server->next) {
        if (server->request_type > REQUEST_UNKNOWN_0 ||
                server->tcpinfo.srcport != reqport)
            continue;

        if (!*hostent->hostent_entry.h_addr_list)
            continue;

        for (char **addr = hostent->hostent_entry.h_addr_list; *addr; addr++) {
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
    unsigned recv_data_times_l = 0, send_data_times_l = 0;
    enum TRANS_DIRECTION t_dir = TRANS_MINE;

    if (reqaddr_char[0]) {
        if (!found_ser) {
            QueryTransTimes(recv_data_times_l, send_data_times_l);

            if (
                (recv_data_times_l != 1 || send_data_times_l != 2) &&
                (recv_data_times_l != 3 || send_data_times_l != 4)
            )
                return 0;

            if (MemCmpare(pkg.content, 0, 2, "331", strlen("331")))
                return 0;

            found_ser = true;
        }

        return bound_proxy_ser_tcp ? 1 : FindSerTcp(server);
    }

    t_dir = QueryTransTimes(recv_data_times_l, send_data_times_l);

    if (send_data_times_l == 1) {
        if (recv_data_times_l != 1)
            return recv_data_times_l > 3 ? 0 : -1;

        if (t_dir != TRANS_RECV)
            return 0;

        GetReqAddr_Port(pkg, true);
        return -1;
    }

    if (send_data_times_l != 3)
        return send_data_times_l > 3 ? 0 : -1;

    if (recv_data_times_l != 3)
        return recv_data_times_l > 3 ? 0 : -1;

    if (t_dir != TRANS_RECV)
        return 0;

    return GetReqAddr_Port(pkg, true) ? -1 : 0;
}

int ProxyClientTcp::HandleHttp(
    const struct TCPIP &pkg,
    const ProxySerTcp *server
)
{
    int res = 0;
    unsigned recv_data_times_l = 0, send_data_times_l = 0;

    if (!reqaddr_char[0])
        return GetReqAddr_Port(pkg, true) ? -1 : 0;

    if (!found_ser) {
        QueryTransTimes(recv_data_times_l, send_data_times_l);

        if (recv_data_times_l > 1 || send_data_times_l > 1)
            return 0;

        else if (recv_data_times_l < 1 || send_data_times_l < 1)
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
    unsigned recv_data_times_l = 0, send_data_times_l = 0;
    enum TRANS_DIRECTION t_dir = TRANS_MINE;

    if (!reqaddr_char[0]) {
        t_dir = QueryTransTimes(recv_data_times_l, send_data_times_l);

        if (recv_data_times_l < 2 || send_data_times_l < 2)
            return -1;

        else if (recv_data_times_l > 2 || send_data_times_l > 2)
            return 0;

        return t_dir != TRANS_RECV ? 0 : GetReqAddr_Port(pkg, true) ? -1 : 0;
    }

    if (!found_ser) {
        if (
            pkg.content_length <= 4 ||
            pkg.content[pkg.content_length - 2] != '\r' ||
            pkg.content[pkg.content_length - 1] != '\n'
        )
            return 0;

        QueryTransTimes(recv_data_times_l, send_data_times_l);

        if (
            recv_data_times_l != 2 ||
            send_data_times_l != 3 ||
            MemCmpare(pkg.content, 0, pkg.content_length - 1, "381", strlen("381"))
        )
            return 0;

        found_ser = true;
        return -1;
    }

    QueryTransTimes(recv_data_times_l, send_data_times_l);

    if (recv_data_times_l > 3 || send_data_times_l > 3)
        return -1;

    return bound_proxy_ser_tcp ? 1 : FindSerTcp(server);
}

int ProxyClientTcp::HandlePop3(
    const struct TCPIP &pkg,
    const ProxySerTcp *server
)
{
    unsigned recv_data_times_l = 0, send_data_times_l = 0;
    enum TRANS_DIRECTION t_dir = TRANS_MINE;

    if (!reqaddr_char[0]) {
        t_dir = QueryTransTimes(recv_data_times_l, send_data_times_l);

        if (recv_data_times_l < 1 || send_data_times_l < 1)
            return -1;

        else if (recv_data_times_l > 1 || send_data_times_l > 1)
            return 0;

        return t_dir != TRANS_RECV ? 0 : GetReqAddr_Port(pkg, true) ? -1 : 0;
    }

    if (!found_ser) {
        if (
            pkg.content_length <= 4 ||
            pkg.content[pkg.content_length - 2] != '\r' ||
            pkg.content[pkg.content_length - 1] != '\n'
        )
            return 0;

        QueryTransTimes(recv_data_times_l, send_data_times_l);

        if (
            recv_data_times_l != 1 ||
            send_data_times_l != 2 ||
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
    struct Socks4ConnResp *reply =
            reinterpret_cast<struct Socks4ConnResp *>(pkg.content);
    unsigned recv_data_times_l = 0, send_data_times_l = 0;

    if (!reqaddr_char[0])
        return GetReqAddr_Port(pkg, true) ? -1 : 0;

    if (found_ser)
        return bound_proxy_ser_tcp ? 1 : FindSerTcp(server);

    QueryTransTimes(recv_data_times_l, send_data_times_l);

    if (recv_data_times_l > 1 || send_data_times_l > 1)
        return 0;

    else if (!recv_data_times_l || !send_data_times_l)
        return -1;

    if (pkg.content_length != 8)
        return 0;

    if (!reply->version && reply->reply_code == SOCKS4_RESPONSE_GRANTED) {
        found_ser = true;
        return -1;
    }

    return 0;
}

int ProxyClientTcp::HandleSocks4A(
    const struct TCPIP &pkg,
    const ProxySerTcp *server
)
{
    // the same as SOCKS4
    return HandleSocks4(pkg, server);
}

int ProxyClientTcp::HandleSocks5(
    const struct TCPIP &pkg,
    const ProxySerTcp *server
)
{
    struct Socks5ConnRespHeader *reply =
            reinterpret_cast<struct Socks5ConnRespHeader *>(pkg.content);
    unsigned recv_data_times_l = 0, send_data_times_l = 0;
    unsigned trans_times_req1 = 0, trans_times_req2 = 0;
    int res = 0;
    g_logFile_proxy.AppendText(
        "HandleSocks5,m_socks5req2.Cmd=%d;m_reqConnAddr:%s",
        socks5_request_header.command,
        reqaddr_char
    );

    if (socks5_request_header.command == SOCKS_REQUEST_UDP_CONN) {
        if (
            reply->version != 5 ||
            reply->response_code != SOCKS5_RESPONSE_GRANTED ||
            reply->reserved_must_be_0
        )
            return 0;

        switch (reply->addr_type) {
            case SOCKS5_ADDR_IPV4:
                return pkg.content_length == GET_SOCKS5_REQUEST_SIZE_IPV4(reply);

            case SOCKS5_ADDR_DOMAIN:
                return pkg.content_length == GET_SOCKS5_REQUEST_SIZE_DOMAIN(reply);

            default:
                return 0;
        }
    }

    if (reqaddr_char[0]) {
        if (found_ser) {
            if (bound_proxy_ser_tcp)
                return 1;

            if (!(res = FindSerTcp(server)))
                return find_ser_times == 1 || find_ser_times++ == 2 ? -1 : res;
        }

        g_logFile_proxy.AppendText("HandleSocks5 收到的应该是响应请求报文");

        if (
            reply->version == 5 &&
            reply->response_code == SOCKS5_RESPONSE_GRANTED &&
            !reply->reserved_must_be_0 &&
            (reply->addr_type == SOCKS5_ADDR_IPV4 ||
             reply->addr_type == SOCKS5_ADDR_DOMAIN)
        ) {
            found_ser = true;
            g_logFile_proxy.AppendText("\t 确认是响应请求报文");
            return -1;
        }

        return 0;
    }

    switch (socks5_request_addr.request_header.reserved_must_be_0) {
        case 0:
            trans_times_req2 = 2;
            trans_times_req1 = 1;
            break;

        case 2:
            trans_times_req2 = 3;
            trans_times_req1 = 2;
            break;

        default:
            return 0;
    }

    g_logFile_proxy.AppendText(
        "TransTimesReq2 =%d",
        trans_times_req2
    );
    QueryTransTimes(recv_data_times_l, send_data_times_l);
    g_logFile_proxy.AppendText(
        "revDataTimes=%d;sendDataTimes=%d ",
        recv_data_times_l,
        send_data_times_l
    );

    if (
        (recv_data_times_l == trans_times_req2 &&
         send_data_times_l == trans_times_req1) ||
        (recv_data_times_l == trans_times_req1 &&
         send_data_times_l == trans_times_req2)
    ) {
        if (GetReqAddr_Port(pkg, true))
            return -1;

        g_logFile_proxy.AppendText("GetReqAddr_Port failed\r\n");
        return socks5_request_header.command == SOCKS_REQUEST_UDP_CONN ? -1 : 0;

    } else
        return trans_times_req2 <= recv_data_times_l ||
               trans_times_req2 <= send_data_times_l ? 0 : -1;
}

int ProxyClientTcp::HandleTelnet(
    [[maybe_unused]] const struct TCPIP &pkg,
    [[maybe_unused]] const ProxySerTcp *server
)
{
    return 0;
}
