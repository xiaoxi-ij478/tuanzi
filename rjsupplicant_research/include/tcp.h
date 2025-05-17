#ifndef TCP_H
#define TCP_H

#include "netutil.h"
#include "dnsquery.h"

enum REQUEST_TYPE {
    REQUEST_INVALID,
    REQUEST_HTTP,
    REQUEST_SOCK4,
    REQUEST_SOCK4A,
    REQUEST_SOCK5,
    REQUEST_FTP,
    REQUEST_POP3,
    REQUEST_NNTP = 8,
    REQUEST_MMS,
    REQUEST_TELNET = 11
};

enum TRANS_DIRECTION {
    TRANS_R2H = -1,
    TRANS_INVALID,
    TRANS_H2R
};

enum SOCKS5_ADDRTYPE : unsigned char {
    SOCKS5_ADDR_IPV4 = 1,
    SOCKS5_ADDR_DOMAIN = 3,
    SOCKS5_ADDR_IPV6
};

enum SOCKS5_CONNREQ_COMMAND : unsigned char {
    SOCKS5_CONNREQ_TCP_CONN = 1,
    SOCKS5_CONNREQ_TCP_BIND,
    SOCKS5_CONNREQ_UDP_CONN
};

struct Socks5Domain {
    char addr_len;
    char addr[0];
};

union Socks5AddrUnion {
    struct in_addr ipv4_addr;
    struct Socks5Domain domain_addr;
    struct in6_addr ipv6_addr;
};

struct Socks5ConnReqHeader {
    char version;
    enum SOCKS5_CONNREQ_COMMAND command;
    char reserved_must_be_0;
    enum SOCKS5_ADDRTYPE addr_type;
};

struct Socks5ConnReq {
    struct Socks5ConnReqHeader request_header;
    union Socks5AddrUnion addr;
};

struct TcpInfo {
    in_addr_t dstaddr;
    in_addr_t srcaddr;
    struct IPHeader *ipheader;
    struct TCPHeader *tcpheader;
};

struct TCPIP {
    char pad[8];
    struct IPHeader *ipheader;
    struct TCPHeader *tcpheader;
    unsigned char *content;
    unsigned int content_length;
};

class CTcp
{
    public:
        CTcp(const struct TcpInfo &info);
        ~CTcp();

    private:
        bool GetReqAddr_Port(const struct TCPIP &pkg, bool query_hostname) const;

        bool GetFtpReqAddr_Port(const struct TCPIP &pkg) const;
        bool GetHttpReqAddr_Port(const struct TCPIP &pkg) const;
        bool GetMmsReqAddr_Port(const struct TCPIP &pkg) const;
        bool GetNntpReqAddr_Port(const struct TCPIP &pkg) const;
        bool GetPop3ReqAddr_Port(const struct TCPIP &pkg) const;
        bool GetSocks4AReqAddr_Port(const struct TCPIP &pkg) const;
        bool GetSocks4ReqAddr_Port(const struct TCPIP &pkg) const;
        bool GetSocks5ReqAddr_Port(const struct TCPIP &pkg) const;
        bool GetTelnetReqAddr_Port(const struct TCPIP &pkg) const;

        bool IsFtpType(const struct TCPIP &pkg) const;
        bool IsHttpType(const struct TCPIP &pkg) const;
        bool IsMine(const struct TCPIP &pkg) const;
        bool IsMmsType(const struct TCPIP &pkg) const;
        bool IsNntpType(const struct TCPIP &pkg) const;
        bool IsPop3Type(const struct TCPIP &pkg) const;
        bool IsSocks4AType(const struct TCPIP &pkg) const;
        bool IsSocks4Type(const struct TCPIP &pkg) const;
        bool IsSocks5Type(const struct TCPIP &pkg) const;
        bool IsTelnetType(const struct TCPIP &pkg) const;
        int QueryAndUpdate(const struct TCPIP &pkg) const;
        bool QueryProtocolType(const struct TCPIP &pkg, unsigned int flag) const;
        bool QueryTransTimes(int &r2h_trans_times, int &h2r_trans_times) const;

        enum REQUEST_TYPE request_type;
        char reqaddr_char[64];
        in_addr_t reqaddr_int;
        unsigned short reqport;
        struct CHostEnt *hostent;
        struct {
            enum SOCKS5_ADDRTYPE addr_type;
            union Socks5AddrUnion addr;
        } socks5_request_addr;
        struct Socks5ConnReqHeader socks5_request_header;
        unsigned char socks5_request_addr_first_byte;
        unsigned int r2h_trans_times;
        unsigned int h2r_trans_times;
        enum TRANS_DIRECTION trans_direction;
        struct TcpInfo tcpinfo;

        static CDNSQuery dns_queryer; // m_dns
};

#endif // TCP_H
