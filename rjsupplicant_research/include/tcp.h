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
    TRANS_MINE,
    TRANS_H2R
};

enum SOCKS5_ADDRTYPE : unsigned char {
    SOCKS5_ADDR_IPV4 = 1,
    SOCKS5_ADDR_DOMAIN = 3,
    SOCKS5_ADDR_IPV6
};

enum SOCKS_CONNREQ_COMMAND : unsigned char {
    SOCKS_CONNREQ_TCP_CONN = 1,
    SOCKS_CONNREQ_TCP_BIND,
    SOCKS_CONNREQ_UDP_CONN
};

struct [[gnu::packed]] Socks4ConnReq {
    unsigned char version;
    enum SOCKS_CONNREQ_COMMAND command;
    unsigned short port;
    struct in_addr ip;
    char id[];
};

struct [[gnu::packed]] Socks4AConnReq {
    unsigned char version;
    enum SOCKS_CONNREQ_COMMAND command;
    unsigned short port;
    struct in_addr ip;
    char id_domain[];
};

#define GET_SOCKS4A_DOMAIN(content) \
    ((static_cast<struct Socks4AConnReq *>(content)->id_domain) + \
     strlen(static_cast<struct Socks4AConnReq *>(content)->id_domain) + 1)

struct [[gnu::packed]] Socks5IPv4 {
    struct in_addr ipv4_addr;
    unsigned short port;
};

struct [[gnu::packed]] Socks5IPv6 {
    struct in6_addr ipv6_addr;
    unsigned short port;
};

struct [[gnu::packed]] Socks5Domain {
    unsigned char addr_len;
    char addr_and_port[257];
};

union Socks5AddrUnion {
    struct Socks5IPv4 ipv4_addr;
    struct Socks5IPv6 ipv6_addr;
    struct Socks5Domain domain_addr;
};

struct [[gnu::packed]] Socks5ConnReqHeader {
    unsigned char version;
    enum SOCKS_CONNREQ_COMMAND command;
    unsigned char reserved_must_be_0;
    enum SOCKS5_ADDRTYPE addr_type;
};

struct [[gnu::packed]] Socks5ConnReq {
    struct Socks5ConnReqHeader request_header;
    union Socks5AddrUnion addr;
};

#define GET_SOCKS5_REQUEST_ADDR(content) \
    (reinterpret_cast<union Socks5AddrUnion *> \
     (reinterpret_cast<unsigned char *>(content) + \
      sizeof(struct Socks5ConnReqHeader)))

#define GET_SOCKS5_REQUEST_ADDR_DOMAIN(content) \
    (GET_SOCKS5_REQUEST_ADDR(content)->domain_addr)

#define GET_SOCKS5_REQUEST_PORT_DOMAIN(content) \
    (*reinterpret_cast<unsigned short *> \
     ((reinterpret_cast<char *>(content) + \
       sizeof(struct Socks5ConnReqHeader) + sizeof(unsigned char) + \
       GET_SOCKS5_REQUEST_ADDR_DOMAIN(content).addr_len)))

#define GET_SOCKS5_REQUEST_SIZE_IPV4(content) \
    (sizeof(struct Socks5ConnReqHeader) + \
     sizeof(struct Socks5IPv4))

#define GET_SOCKS5_REQUEST_SIZE_IPV6(content) \
    (sizeof(struct Socks5ConnReqHeader) + \
     sizeof(struct Socks5IPv6))

#define GET_SOCKS5_REQUEST_SIZE_DOMAIN(content) \
    (sizeof(struct Socks5ConnReqHeader) + \
     sizeof(unsigned char) + \
     GET_SOCKS5_REQUEST_ADDR_DOMAIN(content).addr_len + \
     sizeof(unsigned short))

enum PlayIncarnation : unsigned {
    MMS_DISABLE_PACKET_PAIR = 0xf0f0f0ef,
    MMS_USE_PACKET_PAIR = 0xf0f0f0f0
};

union MMSMessage {
    struct [[gnu::packed]] {
        unsigned chunkLen;
        unsigned MID;
        enum PlayIncarnation playIncarnation;
        unsigned MacToViewerProtocolRevision;
        unsigned ViewerToMacProtocolRevision;
        wchar_t subscriberName[];
    } LinkViewerToMacConnect;
};

struct [[gnu::packed]] MMSTcpMessage {
    unsigned char rep;
    unsigned char version;
    unsigned char versionMinor;
    unsigned char padding;
    unsigned sessionId;
    unsigned messageLength;
    unsigned seal;
    unsigned chunkCount;
    unsigned short seq;
    unsigned short MBZ;
    unsigned long timeSent;
    union MMSMessage message;
};

struct TcpInfo {
    in_addr_t dstaddr;
    in_addr_t srcaddr;
    unsigned short dstport;
    unsigned short srcport;
    unsigned h2r_last_seq;
    unsigned r2h_last_seq;
};

struct TCPIP {
    unsigned long pad;
    struct IPHeader *ipheader;
    struct TCPHeader *tcpheader;
    unsigned char *content;
    unsigned content_length;
};

class CTcp
{
    public:
        CTcp(const struct TcpInfo &info);
        ~CTcp();

    private:
        bool GetReqAddr_Port(const struct TCPIP &pkg, bool query_hostname);

        bool GetFtpReqAddr_Port(const struct TCPIP &pkg);
        bool GetHttpReqAddr_Port(const struct TCPIP &pkg);
        bool GetMmsReqAddr_Port(const struct TCPIP &pkg);
        bool GetNntpReqAddr_Port(const struct TCPIP &pkg);
        bool GetPop3ReqAddr_Port(const struct TCPIP &pkg);
        bool GetSocks4AReqAddr_Port(const struct TCPIP &pkg);
        bool GetSocks4ReqAddr_Port(const struct TCPIP &pkg);
        bool GetSocks5ReqAddr_Port(const struct TCPIP &pkg);
        bool GetTelnetReqAddr_Port(const struct TCPIP &pkg);

        bool IsFtpType(const struct TCPIP &pkg);
        bool IsHttpType(const struct TCPIP &pkg);
        bool IsMine(const struct TCPIP &pkg);
        bool IsMmsType(const struct TCPIP &pkg);
        bool IsNntpType(const struct TCPIP &pkg);
        bool IsPop3Type(const struct TCPIP &pkg);
        bool IsSocks4AType(const struct TCPIP &pkg);
        bool IsSocks4Type(const struct TCPIP &pkg);
        bool IsSocks5Type(const struct TCPIP &pkg);
        bool IsTelnetType(const struct TCPIP &pkg);
        int QueryAndUpdate(const struct TCPIP &pkg);
        int QueryProtocolType(const struct TCPIP &pkg, unsigned flag);
        enum TRANS_DIRECTION QueryTransTimes(
            int &r2h_trans_times,
            int &h2r_trans_times
        ) const;
        enum REQUEST_TYPE request_type;
        char reqaddr_char[64];
        in_addr_t reqaddr_int;
        unsigned short reqport;
        struct CHostEnt *hostent;
        struct {
            struct Socks5ConnReqHeader request_header;
            char may_be_address[0x101 - sizeof(struct Socks5ConnReqHeader)];
        } socks5_request_addr;
        struct Socks5ConnReqHeader socks5_request_header;
        unsigned char socks5_request_domain_len;
        unsigned r2h_trans_times;
        unsigned h2r_trans_times;
        enum TRANS_DIRECTION trans_direction;
        struct TcpInfo tcpinfo;

        static CDNSQuery dns_queryer; // m_dns
};

#endif // TCP_H
