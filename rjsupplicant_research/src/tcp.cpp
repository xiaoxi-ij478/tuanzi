#include "netutil.h"
#include "dnsquery.h"
#include "global.h"
#include "util.h"
#include "hostentutil.h"
#include "tcp.h"

CDNSQuery CTcp::dns_queryer;

CTcp::CTcp(const struct TcpInfo &info) :
    request_type(REQUEST_INVALID),
    reqaddr_int(-1),
    reqport(),
    hostent(),
    socks5_request_addr(),
    socks5_request_header(),
    socks5_request_domain_len(),
    r2h_trans_times(),
    h2r_trans_times(),
    trans_direction(TRANS_MINE),
    tcpinfo(info)
{}

CTcp::~CTcp()
{
    delete_hostent(&hostent->hostent_entry);
}

bool CTcp::GetReqAddr_Port(const struct TCPIP &pkg, bool query_hostname)
{
    bool ret = false;

    switch (request_type) {
        case REQUEST_HTTP:
            ret = GetHttpReqAddr_Port(pkg);
            break;

        case REQUEST_SOCK4:
            ret = GetSocks4ReqAddr_Port(pkg);
            break;

        case REQUEST_SOCK4A:
            ret = GetSocks4AReqAddr_Port(pkg);
            break;

        case REQUEST_SOCK5:
            ret = GetSocks5ReqAddr_Port(pkg);
            break;

        case REQUEST_FTP:
            ret = GetFtpReqAddr_Port(pkg);
            break;

        case REQUEST_POP3:
            ret = GetPop3ReqAddr_Port(pkg);
            break;

        case REQUEST_NNTP:
            ret = GetNntpReqAddr_Port(pkg);
            break;

        case REQUEST_MMS:
            ret = GetMmsReqAddr_Port(pkg);
            break;

        case REQUEST_TELNET:
            ret = GetTelnetReqAddr_Port(pkg);
            break;

        default:
            ret = false;
            break;
    }

    if (!ret)
        return false;

    if (query_hostname && reqaddr_int == -1)
        if (
            (reqaddr_int = inet_addr(reqaddr_char)) == -1 &&
            dns_queryer.PostQueryByName(reqaddr_char, &hostent) != 1
        )
            g_logFile_proxy.AppendText(
                "ProxyClientTcp::TryDetectTCPIP PostQueryByName failed\r\n"
            );

    g_logFile_proxy.AppendText(
        "m_reqConnAddr:%s; m_ulReqAddr:%u; m_reqPort:%d",
        reqaddr_char,
        reqaddr_int,
        reqport
    );
    return true;
}

bool CTcp::GetFtpReqAddr_Port(const struct TCPIP &pkg)
{
    int space_pos = 0;
    int at_pos = 0;
    int copy_len = 0;
    const char *content = reinterpret_cast<const char *>(pkg.content);

    if (
        pkg.content_length <= 7 ||
        pkg.content[pkg.content_length - 2] != '\r' ||
        pkg.content[pkg.content_length - 1] != '\n'
    )
        return false;

    if ((space_pos = FindChar(' ', content, 0, pkg.content_length - 1)) <= 0)
        return false;

    if (
        MemCmpare(pkg.content, 0, space_pos - 1, "USER", strlen("USER")) &&
        MemCmpare(pkg.content, 0, space_pos - 1, "OPEN", strlen("OPEN")) &&
        MemCmpare(pkg.content, 0, space_pos - 1, "SITE", strlen("SITE"))
    )
        return false;

    if (
        (at_pos = FindChar('@', content, space_pos + 1, pkg.content_length - 1))
        < space_pos + 1
    )
        return false;

    if (at_pos + 1 >= pkg.content_length - 3)
        return false;

    if ((copy_len = pkg.content_length - 3 - at_pos) > sizeof(reqaddr_char) - 1)
        return false;

    memcpy(reqaddr_char, &pkg.content[at_pos + 1], copy_len);
    reqaddr_char[copy_len] = 0;
    reqport = 21;
    return true;
}

bool CTcp::GetHttpReqAddr_Port(const struct TCPIP &pkg)
{
    int space_pos = 0;
    int space_pos2 = 0;
    int url_begin = 0;
    int path_begin = 0;
    int port_begin = 0;
    int end = 0;
    int domain_len = 0;
    const char *content = reinterpret_cast<const char *>(pkg.content);

    if ((space_pos = FindChar(' ', content, 0, pkg.content_length - 1)) <= 2)
        return false;

    if (
        MemCmpare(pkg.content, 0, space_pos - 1, "GET", strlen("GET")) &&
        MemCmpare(pkg.content, 0, space_pos - 1, "CONNECT", strlen("CONNECT"))
    )
        return false;

    if ((space_pos2 =
                FindChar(
                    ' ', content,
                    space_pos + 1,
                    pkg.content_length - 1
                )) < 0)
        return false;

    if (
        MemCmpare(
            pkg.content, space_pos2 + 1, pkg.content_length - 1,
            "HTTP/1.0\r\n", strlen("HTTP/1.0\r\n")
        ) &&
        MemCmpare(
            pkg.content, space_pos2 + 1, pkg.content_length - 1,
            "HTTP/1.1\r\n", strlen("HTTP/1.1\r\n")
        )
    )
        return false;

    if (space_pos2 - space_pos - 2 <= 1)
        return false;

    url_begin = space_pos + 1;
    end = space_pos2 - 1;

    if (!MemCmpare(pkg.content, url_begin, end, "http://", strlen("http://")))
        url_begin += strlen("http://");

    else if (!MemCmpare(
                 pkg.content,
                 url_begin, end,
                 "https://", strlen("https://")
             ))
        url_begin += strlen("https://");

    if (url_begin > end)
        return false;

    if ((path_begin = FindChar('/', content, url_begin, end)) > 0)
        if ((end = path_begin - 1) <= url_begin)
            return false;

    if ((port_begin = FindChar(':', content, url_begin, end)) >= 0) {
        if (end - port_begin >= 5)
            return false;

        end = port_begin - 1;
        reqport = strtol(&content[port_begin], nullptr, 10);

    } else
        reqport = 80;

    if (url_begin > end)
        return false;

    if ((domain_len = end + 1 - url_begin) > 63)
        return false;

    if (!pkg.content[url_begin])
        return false;

    memcpy(reqaddr_char, &pkg.content[url_begin], domain_len);
    reqaddr_char[domain_len] = 0;
    return true;
}

bool CTcp::GetMmsReqAddr_Port([[maybe_unused]] const struct TCPIP &pkg)
{
    return request_type == REQUEST_MMS;
}

bool CTcp::GetNntpReqAddr_Port(const struct TCPIP &pkg)
{
    int hash_pos = 0;
    int some_len = 0;
    const char *content = reinterpret_cast<const char *>(pkg.content);

    if (
        pkg.content_length <= 17 ||
        pkg.content[pkg.content_length - 2] != '\r' ||
        pkg.content[pkg.content_length - 1] != '\n' ||
        strncasecmp(content, "AUTHINFO USER ", strlen("AUTHINFO USER "))
    )
        return false;

    hash_pos = pkg.content_length - 4;

    while (hash_pos > 4 && pkg.content[hash_pos--] != '#');

    some_len = pkg.content_length - 3 - hash_pos;

    if (some_len > sizeof(reqaddr_char) - 1)
        return false;

    memcpy(reqaddr_char, &pkg.content[hash_pos + 1], some_len);
    reqaddr_char[some_len] = 0;

    if (strlen(reqaddr_char) <= 2)
        return false;

    reqport = 119;
    return true;
}

// almost the same as NNTP
bool CTcp::GetPop3ReqAddr_Port(const struct TCPIP &pkg)
{
    int hash_pos = 0;
    int some_len = 0;

    if (
        pkg.content_length <= 8 ||
        pkg.content[pkg.content_length - 2] != '\r' ||
        pkg.content[pkg.content_length - 1] != '\n' ||
        MemCmpare(pkg.content, 0, strlen("USER "), "USER ", strlen("USER "))
    )
        return false;

    hash_pos = pkg.content_length - 4;

    while (hash_pos > 4 && pkg.content[hash_pos--] != '#');

    some_len = pkg.content_length - 3 - hash_pos;

    if (some_len > sizeof(reqaddr_char) - 1)
        return false;

    memcpy(reqaddr_char, &pkg.content[hash_pos + 1], some_len);
    reqaddr_char[some_len] = 0;

    if (strlen(reqaddr_char) <= 2)
        return false;

    reqport = 110;
    return true;
}

bool CTcp::GetSocks4AReqAddr_Port([[maybe_unused]] const struct TCPIP &pkg)
{
    return request_type == REQUEST_SOCK4A;
}

bool CTcp::GetSocks4ReqAddr_Port([[maybe_unused]]const struct TCPIP &pkg)
{
    return request_type == REQUEST_SOCK4;
}

bool CTcp::GetSocks5ReqAddr_Port(const struct TCPIP &pkg)
{
    // the original implementation initializes a header with zero
    // and copy the content into it
    // but we choose to bail out if it is less than a pack's size
    char *ipv4_alpha = nullptr;
    struct Socks5ConnReq *request =
            reinterpret_cast<struct Socks5ConnReq *>(pkg.content);

    if (pkg.content_length < sizeof(struct Socks5ConnReqHeader) + 1)
        return false;

    if (
        request->request_header.version != 5 ||
        request->request_header.reserved_must_be_0
    )
        return false;

    switch (request->request_header.command) {
        case SOCKS_CONNREQ_UDP_CONN:
            switch (request->request_header.addr_type) {
                case SOCKS5_ADDR_IPV4:
                    if (pkg.content_length == GET_SOCKS5_REQUEST_SIZE_IPV4(request)) {
                        socks5_request_header = request->request_header;
                        socks5_request_domain_len =
                            reinterpret_cast<char *>(&request->addr.ipv4_addr)[0];
                    }

                    return false;

                case SOCKS5_ADDR_DOMAIN:
                    if (pkg.content_length == GET_SOCKS5_REQUEST_SIZE_DOMAIN(request)) {
                        socks5_request_header = request->request_header;
                        socks5_request_domain_len = request->addr.domain_addr.addr_len;
                    }

                    return false;

                case SOCKS5_ADDR_IPV6:
                    return false;
            }

            break;

        case SOCKS_CONNREQ_TCP_BIND:
            return false;
    }

//    case SOCKS_CONNREQ_TCP_CONN:
    switch (request->request_header.addr_type) {
        case SOCKS5_ADDR_IPV4:
            if (pkg.content_length != GET_SOCKS5_REQUEST_SIZE_IPV4(request))
                return false;

            if (!(ipv4_alpha = inet_ntoa(request->addr.ipv4_addr.ipv4_addr)) ||
                    !ipv4_alpha[0])
                return false;

            strcpy(reqaddr_char, ipv4_alpha);
            reqaddr_int = inet_addr(reqaddr_char);
            reqport = ntohs(request->addr.ipv4_addr.port);
            socks5_request_header = request->request_header;
            socks5_request_domain_len =
                reinterpret_cast<char *>(&request->addr.ipv4_addr)[0];
            break;

        case SOCKS5_ADDR_DOMAIN:
            if (pkg.content_length != GET_SOCKS5_REQUEST_SIZE_DOMAIN(request))
                return false;

            if (
                request->addr.domain_addr.addr_len >
                sizeof(reqaddr_char) - 1
            )
                return false;

            memcpy(
                reqaddr_char,
                request->addr.domain_addr.addr_and_port,
                request->addr.domain_addr.addr_len
            );
            reqaddr_char[request->addr.domain_addr.addr_len] = 0;
            reqport = ntohs(GET_SOCKS5_REQUEST_PORT_DOMAIN(request));
            socks5_request_header = request->request_header;
            socks5_request_domain_len = request->addr.domain_addr.addr_len;
            break;

        case SOCKS5_ADDR_IPV6:
            return false;
    }

    return true;
}

bool CTcp::GetTelnetReqAddr_Port([[maybe_unused]] const struct TCPIP &pkg)
{
    return false;
}

bool CTcp::IsFtpType(const struct TCPIP &pkg)
{
    int trans_times[2] = {};
    QueryTransTimes(trans_times[1], trans_times[0]);
    return trans_times[0] == 1 && !trans_times[1] &&
           MemCmpare(pkg.content, 0, pkg.content_length - 1, "220", strlen("220"));
}

bool CTcp::IsHttpType(const struct TCPIP &pkg)
{
    int space_pos = 0;
    int space_pos2 = 0;
    const char *content = reinterpret_cast<const char *>(pkg.content);

    if ((space_pos = FindChar(' ', content, 0, pkg.content_length - 1) <= 2))
        return false;

    if (
        MemCmpare(pkg.content, 0, space_pos - 1, "GET", strlen("GET")) ||
        MemCmpare(pkg.content, 0, space_pos - 1, "CONNECT", strlen("CONNECT"))
    )
        return false;

    if ((space_pos2 =
                FindChar(
                    ' ', content,
                    space_pos + 1, pkg.content_length - 1
                )) < 0)
        return false;

    if (
        MemCmpare(
            pkg.content, space_pos2 + 1, pkg.content_length - 1,
            "HTTP/1.0\r\n", strlen("HTTP/1.0\r\n")
        ) &&
        MemCmpare(
            pkg.content, space_pos2 + 1, pkg.content_length - 1,
            "HTTP/1.1\r\n", strlen("HTTP/1.1\r\n")
        )
    )
        return false;

    if (space_pos + 1 > space_pos2 - 1)
        return false;

    if (space_pos + 1 == space_pos2 - 1)
        return pkg.content[space_pos + 1] == '/';

    return true;
}

bool CTcp::IsMine(const struct TCPIP &pkg)
{
    return QueryAndUpdate(pkg);
}

// see https://web.archive.org/web/20090219134914/http://download.microsoft.com/download/9/5/E/95EF66AF-9026-4BB0-A41D-A4F81802D92C/%5BMS-MMSP%5D.pdf
// from https://en.wikipedia.org/wiki/Microsoft_Media_Server
// LinkViewerToMacConnect
// and
// https://web.archive.org/web/20081204082646/http://download.microsoft.com/download/9/5/E/95EF66AF-9026-4BB0-A41D-A4F81802D92C/%5BMS-GLOS%5D.pdf
// for why the Unicode is UTF-16LE
bool CTcp::IsMmsType(const struct TCPIP &pkg)
{
    wchar_t *wchar_buf = nullptr;
    char *char_buf = nullptr;
    unsigned char_buflen = 0;
    int host_pos = 0;
    struct MMSTcpMessage *message =
            reinterpret_cast<struct MMSTcpMessage *>(pkg.content);

    if (pkg.content_length <= sizeof(struct MMSTcpMessage) + 3)
        return false;

    if (
        message->rep != 1 ||
        message->version ||
        message->versionMinor ||
        message->padding ||
        message->sessionId != 0xB00BFACE ||
        message->messageLength != pkg.content_length + 16 ||
        message->seal != 0x20534D4D ||
        message->seq ||
        // this is how they check mid
        message->message.LinkViewerToMacConnect.MID >> 4 != 3 ||
        message->message.LinkViewerToMacConnect.playIncarnation != MMS_USE_PACKET_PAIR
        ||
        message->message.LinkViewerToMacConnect.MacToViewerProtocolRevision != 0x4000B
    )
        return false;

    char_buflen = (pkg.content_length - sizeof(struct MMSTcpMessage)) / 2;
    wchar_buf = new wchar_t[char_buflen + 1];
    char_buf = new char[char_buflen + 1];
    memcpy(
        wchar_buf,
        message->message.LinkViewerToMacConnect.subscriberName,
        char_buflen * 2
    );
    wchar_buf[char_buflen] = 0;
    sprintf(char_buf, "%S", wchar_buf);

    if (
        (host_pos =
             FindSub(
                 "Host: ", strlen("Host: "),
                 char_buf, 0, strlen(char_buf)
             )) >= 0 &&
        strlen(char_buf) + 1 -
        (host_pos + strlen("Host: ")) <=
        sizeof(reqaddr_char) - 1
    )
        strcpy(reqaddr_char, char_buf + host_pos + strlen("Host: "));

    delete[] wchar_buf;
    delete[] char_buf;

    if (strlen(reqaddr_char) <= 2)
        return false;

    reqport = 1755;
    return true;
}

bool CTcp::IsNntpType(const struct TCPIP &pkg)
{
    int trans_times[2] = {};
    QueryTransTimes(trans_times[1], trans_times[0]);

    if (
        pkg.content_length > 0x200 ||
        pkg.content[pkg.content_length - 2] != '\r' ||
        pkg.content[pkg.content_length - 1] != '\n'
    )
        return false;

    QueryTransTimes(trans_times[1], trans_times[0]);

    if (
        trans_times[0] != 1 ||
        MemCmpare(pkg.content, 0, pkg.content_length - 1, "200", strlen("200"))
    )
        return false;

    return true;
}

bool CTcp::IsPop3Type(const struct TCPIP &pkg)
{
    int trans_times[2] = {};
    QueryTransTimes(trans_times[1], trans_times[0]);

    if (
        pkg.content_length <= 4 ||
        pkg.content[pkg.content_length - 2] != '\r' ||
        pkg.content[pkg.content_length - 1] != '\n'
    )
        return false;

    QueryTransTimes(trans_times[1], trans_times[0]);

    if (
        trans_times[0] != 1 ||
        MemCmpare(pkg.content, 0, pkg.content_length - 1, "+OK", strlen("+OK"))
    )
        return false;

    return true;
}

bool CTcp::IsSocks4AType(const struct TCPIP &pkg)
{
    struct Socks4AConnReq *request =
            reinterpret_cast<struct Socks4AConnReq *>(pkg.content);

    if (pkg.content_length <= sizeof(struct Socks4AConnReq) + 1)
        return false;

    if (request->version != 4 || request->command != SOCKS_CONNREQ_TCP_CONN)
        return false;

    if (ntohl(request->ip.s_addr) > 255)
        return false;

    if (
        strlen(request->id_domain) +
        strlen(GET_SOCKS4A_DOMAIN(request)) +
        sizeof(struct Socks4AConnReq) + 2 != pkg.content_length
    )
        return false;

    if (strlen(GET_SOCKS4A_DOMAIN(request)) > sizeof(reqaddr_char) - 1)
        return false;

    if (!GET_SOCKS4A_DOMAIN(request)[0])
        return false;

    strcpy(reqaddr_char, GET_SOCKS4A_DOMAIN(request));
    reqport = ntohs(request->port);
    return true;
}

bool CTcp::IsSocks4Type(const struct TCPIP &pkg)
{
    struct Socks4ConnReq *request =
            reinterpret_cast<struct Socks4ConnReq *>(pkg.content);
    char *ipaddr = nullptr;

    if (pkg.content_length <= sizeof(struct Socks4ConnReq))
        return false;

    if (request->version != 4 || request->command != SOCKS_CONNREQ_TCP_BIND)
        return false;

    if (
        !(ipaddr = inet_ntoa(request->ip)) ||
        pkg.content_length != strlen(request->id) +
        sizeof(struct Socks4ConnReq) + 1
    )
        return false;

    strcpy(reqaddr_char, ipaddr);
    reqport = ntohs(request->port);
    reqaddr_int = request->ip.s_addr;
    return true;
}

bool CTcp::IsSocks5Type(const struct TCPIP &pkg)
{
    struct Socks5ConnReq *request =
            reinterpret_cast<struct Socks5ConnReq *>(pkg.content);

    if (pkg.content_length != 3)
        return false;

    if (
        request->request_header.version != 5 ||
        request->request_header.command != SOCKS_CONNREQ_TCP_CONN ||
        (
            request->request_header.reserved_must_be_0 &&
            request->request_header.reserved_must_be_0 != 2
        )
    )
        return false;

    socks5_request_addr.request_header = request->request_header;
    return true;
}

bool CTcp::IsTelnetType([[maybe_unused]] const struct TCPIP &pkg)
{
    return false;
}

int CTcp::QueryAndUpdate(const struct TCPIP &pkg)
{
    if (
        pkg.ipheader->srcaddr == tcpinfo.srcaddr &&
        pkg.ipheader->dstaddr == tcpinfo.dstaddr &&
        ntohs(pkg.tcpheader->srcport) == tcpinfo.srcport &&
        ntohs(pkg.tcpheader->dstport) == tcpinfo.dstport
    ) {
        if (tcpinfo.r2h_last_seq != bswap_64(pkg.tcpheader->seq)) {
            if (tcpinfo.r2h_last_seq >= bswap_64(pkg.tcpheader->seq))
                return TRANS_R2H;

            g_logFile_proxy.AppendText(
                "CTcp::Query may be losted R to H packet Last Seq:%u,Now Seq:%u\r\n",
                tcpinfo.r2h_last_seq,
                bswap_64(pkg.tcpheader->seq)
            );
        }

        r2h_trans_times++;
        trans_direction = TRANS_R2H;
        tcpinfo.r2h_last_seq = pkg.content_length + bswap_64(pkg.tcpheader->seq);
        return TRANS_R2H;
    }

    if (
        pkg.ipheader->srcaddr == tcpinfo.dstaddr &&
        pkg.ipheader->dstaddr == tcpinfo.srcaddr &&
        ntohs(pkg.tcpheader->srcport) == tcpinfo.dstport &&
        ntohs(pkg.tcpheader->dstport) == tcpinfo.srcport
    ) {
        if (tcpinfo.h2r_last_seq != bswap_64(pkg.tcpheader->seq)) {
            if (tcpinfo.h2r_last_seq >= bswap_64(pkg.tcpheader->seq))
                return TRANS_H2R;

            g_logFile_proxy.AppendText(
                "CTcp::Query may be losted H to R packet Last Seq:%u,Now Seq:%u\r\n",
                tcpinfo.h2r_last_seq,
                bswap_64(pkg.tcpheader->seq)
            );
            h2r_trans_times++;
            trans_direction = TRANS_H2R;
            tcpinfo.h2r_last_seq = pkg.content_length + bswap_64(pkg.tcpheader->seq);
            return TRANS_H2R;
        }
    }

    return TRANS_MINE;
}

int CTcp::QueryProtocolType(const struct TCPIP &pkg, unsigned flag)
{
    if (flag & 4 && IsHttpType(pkg))
        return 1;

    if (!(flag & 2))
        return 0;

    if (IsSocks4Type(pkg))
        return 2;

    if (IsSocks4AType(pkg))
        return 3;

    if (IsSocks5Type(pkg))
        return 4;

    return 0;
}

enum TRANS_DIRECTION CTcp::QueryTransTimes(
    unsigned &r2h_trans_times,
    unsigned &h2r_trans_times
) const
{
    r2h_trans_times = this->r2h_trans_times;
    h2r_trans_times = this->h2r_trans_times;
    return trans_direction;
}
