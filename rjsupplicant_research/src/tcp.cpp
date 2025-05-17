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
    socks5_request_addr(),
    socks5_request_addr_first_byte(),
    trans_direction(TRANS_INVALID),
    tcpinfo(info),
    hostent(),
    socks5_request_header(),
    r2h_trans_times(),
    h2r_trans_times()
{}

CTcp::~CTcp()
{
    delete_hostent(&hostent->hostent_entry);
}

bool CTcp::GetReqAddr_Port(const struct TCPIP &pkg, bool query_hostname) const
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

bool CTcp::GetFtpReqAddr_Port(const struct TCPIP &pkg) const
{
    int space_pos = 0;
    int at_pos = 0;
    int copy_len = 0;

    if (
        pkg.content_length <= 7 ||
        pkg.content[pkg.content_length - 2] != '\r' ||
        pkg.content[pkg.content_length - 1] != '\n'
    )
        return false;

    if ((space_pos = FindChar(' ', pkg.content, 0, pkg.content_length - 1)) <= 0)
        return false;

    if (
        MemCmpare(pkg.content, 0, space_pos - 1, "USER", strlen("USER")) &&
        MemCmpare(pkg.content, 0, space_pos - 1, "OPEN", strlen("OPEN")) &&
        MemCmpare(pkg.content, 0, space_pos - 1, "SITE", strlen("SITE"))
    )
        return false;

    if ((at_pos = FindChar(
                      '@',
                      pkg.content,
                      space_pos + 1,
                      pkg.content_length - 1
                  )) < space_pos + 1)
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

bool CTcp::GetHttpReqAddr_Port(const struct TCPIP &pkg) const
{
    int space_pos = 0;
    int space_pos2 = 0;
    int url_begin = 0;
    int path_begin = 0;
    int port_begin = 0;
    int end = 0;
    int domain_len = 0;

    if ((space_pos = FindChar(' ', pkg.content, 0, pkg.content_length - 1)) <= 2)
        return false;

    if (
        MemCmpare(pkg.content, 0, space_pos - 1, "GET", strlen("GET")) &&
        MemCmpare(pkg.content, 0, space_pos - 1, "CONNECT", strlen("CONNECT"))
    )
        return false;

    if ((space_pos2 = FindChar(
                          ' ',
                          pkg.content,
                          space_pos + 1,
                          pkg.content_length - 1
                      )) < 0)
        return false;

    if (
        MemCmpare(
            pkg.content,
            space_pos2 + 1,
            pkg.content_length - 1,
            "HTTP/1.0\r\n",
            strlen("HTTP/1.0\r\n")
        ) &&
        MemCmpare(
            pkg.content,
            space_pos2 + 1,
            pkg.content_length - 1,
            "HTTP/1.1\r\n",
            strlen("HTTP/1.1\r\n")
        )
    )
        return false;

    if (space_pos2 - space_pos - 2 <= 1)
        return false;

    url_begin = space_pos + 1;
    end = space_pos2 - 1;

    if (
        !MemCmpare(
            pkg.content,
            url_begin,
            end,
            "http://",
            strlen("http://")
        )
    )
        url_begin += strlen("http://");

    else if (
        !MemCmpare(
            pkg.content,
            url_begin,
            end,
            "https://",
            strlen("https://")
        )
    )
        url_begin += strlen("https://");

    if (url_begin > end)
        return false;

    if ((path_begin = FindChar('/', pkg.content, url_begin, end)) > 0)
        if ((end = path_begin - 1) <= url_begin)
            return false;

    if ((port_begin = FindChar(':', pkg.content, url_begin, end)) >= 0) {
        if (end - port_begin >= 5)
            return false;

        end = port_begin - 1;
        reqport = strtol(&pkg.content[port_begin], nullptr, 10);

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

bool CTcp::GetMmsReqAddr_Port([[maybe_unused]] const struct TCPIP &pkg) const
{
    return request_type == REQUEST_MMS;
}

bool CTcp::GetNntpReqAddr_Port(const struct TCPIP &pkg) const
{
    int hash_pos = 0;
    int some_len = 0;

    if (
        pkg.content_length <= 17 ||
        pkg.content[pkg.content_length - 2] != '\r' ||
        pkg.content[pkg.content_length - 1] != '\n' ||
        strncasecmp(pkg.content, "AUTHINFO USER ", strlen("AUTHINFO USER "))
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
bool CTcp::GetPop3ReqAddr_Port(const struct TCPIP &pkg) const
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

bool CTcp::GetSocks4AReqAddr_Port(
    [[maybe_unused]] const struct TCPIP &pkg
) const
{
    return request_type == REQUEST_SOCK4A;
}

bool CTcp::GetSocks4ReqAddr_Port([[maybe_unused]] const struct TCPIP &pkg) const
{
    return request_type == REQUEST_SOCK4;
}

bool CTcp::GetSocks5ReqAddr_Port(const struct TCPIP &pkg) const
{
    // the original implementation initializes a header with zero
    // and copy the content into it
    // but we choose to bail out if it is less than a pack's size
    struct Socks5ConnReqHeader *header =
            reinterpret_cast<struct Socks5ConnReqHeader *>(pkg.content);

    if (pkg.content_length < sizeof(struct Socks5ConnReqHeader) + 1)
        return false;

    if (header->version != 5 || header->reserved_must_be_0)
        return false;

    switch (header->command) {
        case SOCKS5_CONNREQ_TCP_BIND:
            switch (header->addr_type) {
                case SOCKS5_ADDR_IPV4:
                    if (
                        pkg.content_length !=
                        sizeof(struct Socks5ConnReqHeader) +
                        sizeof(struct in_addr)
                    )
                        return false;

                    socks5_request_header = *header;
                    socks5_request_addr_first_byte = pkg.content[4];
                    return false;

                case SOCKS5_ADDR_DOMAIN:
            }
    }
}

bool CTcp::GetTelnetReqAddr_Port(const struct TCPIP &pkg) const
{
}

bool CTcp::IsFtpType(const struct TCPIP &pkg) const
{
}

bool CTcp::IsHttpType(const struct TCPIP &pkg) const
{
}

bool CTcp::IsMine(const struct TCPIP &pkg) const
{
}

bool CTcp::IsMmsType(const struct TCPIP &pkg) const
{
}

bool CTcp::IsNntpType(const struct TCPIP &pkg) const
{
}

bool CTcp::IsPop3Type(const struct TCPIP &pkg) const
{
}

bool CTcp::IsSocks4AType(const struct TCPIP &pkg) const
{
}

bool CTcp::IsSocks4Type(const struct TCPIP &pkg) const
{
}

bool CTcp::IsSocks5Type(const struct TCPIP &pkg) const
{
}

bool CTcp::IsTelnetType(const struct TCPIP &pkg) const
{
}

int CTcp::QueryAndUpdate(const struct TCPIP &pkg) const
{
}

bool CTcp::QueryProtocolType(const struct TCPIP &pkg, unsigned int flag) const
{
}

bool CTcp::QueryTransTimes(int &r2h_trans_times, int &h2r_trans_times) const
{
}
