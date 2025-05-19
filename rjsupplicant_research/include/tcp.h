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

struct Socks4ConnReq {
    unsigned char version;
    enum SOCKS_CONNREQ_COMMAND command;
    unsigned short port;
    struct in_addr ip;
    char id[];
} __attribute__((packed));

struct Socks4AConnReq {
    unsigned char version;
    enum SOCKS_CONNREQ_COMMAND command;
    unsigned short port;
    struct in_addr ip;
    char id_domain[];
} __attribute__((packed));

#define GET_SOCKS4A_DOMAIN(content) \
    ((reinterpret_cast<struct Socks4AConnReq *>(content)->id_domain) + \
     strlen(reinterpret_cast<struct Socks4AConnReq *>(content)->id_domain) + 1)

struct Socks5IPv4 {
    struct in_addr ipv4_addr;
    unsigned short port;
} __attribute__((packed));

struct Socks5IPv6 {
    struct in6_addr ipv6_addr;
    unsigned short port;
} __attribute__((packed));

struct Socks5Domain {
    unsigned char addr_len;
    char addr_and_port[257];
} __attribute__((packed));

union Socks5AddrUnion {
    struct Socks5IPv4 ipv4_addr;
    struct Socks5IPv6 ipv6_addr;
    struct Socks5Domain domain_addr;
};

struct Socks5ConnReqHeader {
    unsigned char version;
    enum SOCKS_CONNREQ_COMMAND command;
    unsigned char reserved_must_be_0;
    enum SOCKS5_ADDRTYPE addr_type;
} __attribute__((packed));

struct Socks5ConnReq {
    struct Socks5ConnReqHeader request_header;
    union Socks5AddrUnion addr;
} __attribute__((packed));

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

enum PlayIncarnation : unsigned int {
    MMS_DISABLE_PACKET_PAIR = 0xf0f0f0ef,
    MMS_USE_PACKET_PAIR = 0xf0f0f0f0
};

union MMSMessage {
    struct {
        unsigned int chunkLen;
        unsigned int MID;
        enum PlayIncarnation playIncarnation;
        unsigned int MacToViewerProtocolRevision;
        unsigned int ViewerToMacProtocolRevision;
        wchar_t subscriberName[];
    } LinkViewerToMacConnect __attribute__((packed));
};

struct MMSTcpMessage {
    unsigned char rep;
    unsigned char version;
    unsigned char versionMinor;
    unsigned char padding;
    unsigned int sessionId;
    unsigned int messageLength;
    unsigned int seal;
    unsigned int chunkCount;
    unsigned short seq;
    unsigned short MBZ;
    unsigned long timeSent;
    union MMSMessage message;
} __attribute__((packed));

struct TcpInfo {
  in_addr_t dstaddr;
  in_addr_t srcaddr;
  unsigned short dstport;
  unsigned short srcport;
  unsigned int h2r_last_seq;
  unsigned int r2h_last_seq;
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
        bool GetReqAddr_Port(const struct TCPIP &pkg, bool query_hostname);

        bool GetFtpReqAddr_Port(const struct TCPIP &pkg);
        bool GetHttpReqAddr_Port(const struct TCPIP &pkg);
        bool GetMmsReqAddr_Port(const struct TCPIP &pkg) const;
        bool GetNntpReqAddr_Port(const struct TCPIP &pkg);
        bool GetPop3ReqAddr_Port(const struct TCPIP &pkg);
        bool GetSocks4AReqAddr_Port(const struct TCPIP &pkg) const;
        bool GetSocks4ReqAddr_Port(const struct TCPIP &pkg) const;
        bool GetSocks5ReqAddr_Port(const struct TCPIP &pkg);
        bool GetTelnetReqAddr_Port(const struct TCPIP &pkg) const;

        bool IsFtpType(const struct TCPIP &pkg) const;
        bool IsHttpType(const struct TCPIP &pkg) const;
        bool IsMine(const struct TCPIP &pkg) const;
        bool IsMmsType(const struct TCPIP &pkg);
        bool IsNntpType(const struct TCPIP &pkg) const;
        bool IsPop3Type(const struct TCPIP &pkg) const;
        bool IsSocks4AType(const struct TCPIP &pkg);
        bool IsSocks4Type(const struct TCPIP &pkg);
        bool IsSocks5Type(const struct TCPIP &pkg);
        bool IsTelnetType(const struct TCPIP &pkg) const;
        int QueryAndUpdate(const struct TCPIP &pkg) const;
        int QueryProtocolType(const struct TCPIP &pkg, unsigned int flag) const;
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
        unsigned int r2h_trans_times;
        unsigned int h2r_trans_times;
        enum TRANS_DIRECTION trans_direction;
        struct TcpInfo tcpinfo;

        static CDNSQuery dns_queryer; // m_dns
};

#endif // TCP_H
