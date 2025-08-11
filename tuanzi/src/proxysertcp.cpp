#include "all.h"
#include "timeutil.h"
#include "global.h"
#include "proxyclienttcp.h"
#include "proxysertcp.h"

ProxySerTcp::ProxySerTcp(const struct TcpInfo& info) :
    CTcp(info),
    bound_proxy_client_tcp(),
    prev(),
    next(),
    creation_time(GetTickCount())
{}

bool ProxySerTcp::IsExpired(unsigned) const
{
    return false;
}

int ProxySerTcp::FindClientTcp(
    const struct TCPIP& pkg,
    ProxyClientTcp *client,
    unsigned flag
)
{
    bool need_two_check = true;
    unsigned recv_data_times_l = 0, send_data_times_l = 0;
    unsigned trans_times_req1 = 0, trans_times_req2 = 0;
    GetTickCount(); // ?

    if (
        !pkg.ipheader ||
        !pkg.tcpheader ||
        !pkg.content ||
        !pkg.content_length
    ) {
        g_logFile_proxy.AppendText("ProxyClientTcp::TryDetectTCPIP:parameter error\r\n");
        return 0;
    }

    if (!QueryAndUpdate(pkg))
        return -2;

    if (!bound_proxy_client_tcp)
        return 1;

    if (!request_type)
        request_type = QueryProtocolType(pkg, flag);

    switch (request_type) {
        case REQUEST_HTTP:
            if (reqaddr_char[0])
                break;

            need_two_check = GetReqAddr_Port(pkg, false);

            if (!need_two_check)
                g_logFile_proxy.AppendText(
                    "didn't find address,it's no two step proxy\r\n"
                );

            break;

        case REQUEST_SOCK5:
            if (reqaddr_char[0])
                break;

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

            QueryTransTimes(recv_data_times_l, send_data_times_l);

            if (
                (recv_data_times_l != trans_times_req2 ||
                 send_data_times_l != trans_times_req1) &&
                (recv_data_times_l != trans_times_req1 ||
                 send_data_times_l != trans_times_req2)
            )
                return trans_times_req2 <= recv_data_times_l ||
                       trans_times_req2 <= send_data_times_l ? -1 : 0;

            need_two_check = GetReqAddr_Port(pkg, false);
            break;

        default:
            need_two_check = request_type > REQUEST_UNKNOWN_0;
            break;
    }

    if (need_two_check && reqaddr_char[0])
        for (ProxyClientTcp *c = client; c; c = c->next) {
            if (reqport != c->reqport || strcmp(reqaddr_char, c->reqaddr_char))
                continue;

            bound_proxy_client_tcp = c;
            c->bound_proxy_ser_tcp = this;
            g_logFile_proxy.AppendText(
                "ProxySerTcp find clientTcp- tow step\r\n"
            );
            return 1;
        }

    for (ProxyClientTcp *c = client; c; c = c->next) {
        if (!c->reqaddr_char[0] || c->reqport != tcpinfo.srcport)
            continue;

        g_logFile_proxy.AppendText(
            "one step proxy, wait proxy client tcp just\r\n"
        );
        request_type = REQUEST_UNKNOWN_N1;
        return 0;
    }

    return -1;
}
