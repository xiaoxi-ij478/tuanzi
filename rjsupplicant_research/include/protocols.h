#ifndef PROTOCOLS_H_INCLUDED
#define PROTOCOLS_H_INCLUDED

// for SOCKS related knowledge see https://zh.wikipedia.org/wiki/SOCKS

enum SOCKS5_ADDRTYPE : unsigned char {
    SOCKS5_ADDR_IPV4 = 1,
    SOCKS5_ADDR_DOMAIN = 3,
    SOCKS5_ADDR_IPV6
};

enum SOCKS_REQUEST_COMMAND : unsigned char {
    SOCKS_REQUEST_TCP_CONN = 1,
    SOCKS_REQUEST_TCP_BIND,
    SOCKS_REQUEST_UDP_CONN
};

enum SOCKS4_RESPONSE_CODE : unsigned char {
    SOCKS4_RESPONSE_GRANTED = 0x5A,
    SOCKS4_RESPONSE_REJECTED,
    SOCKS4_RESPONSE_REJECTED_CLIENT_NO_IDENTD,
    SOCKS4_RESPONSE_REJECTED_CLIENT_IDENTD_FAILED
};

enum SOCKS5_RESPONSE_CODE : unsigned char {
    SOCKS5_RESPONSE_GRANTED,
    SOCKS5_RESPONSE_GENERAL_FAILURE,
    SOCKS5_RESPONSE_CONNECTION_DISALLOWED,
    SOCKS5_RESPONSE_NETWORK_UNREACHABLE,
    SOCKS5_RESPONSE_HOST_UNREACHABLE,
    SOCKS5_RESPONSE_CONNECT_REFUSED,
    SOCKS5_RESPONSE_TTL_EXPIRED,
    SOCKS5_RESPONSE_COMMAND_UNSUPPORTED,
    SOCKS5_RESPONSE_ADDRESS_TYPE_UNSUPPORTED
};

struct [[gnu::packed]] Socks4ConnReq {
    unsigned char version;
    enum SOCKS_REQUEST_COMMAND command;
    unsigned short port;
    struct in_addr ip;
    char id[];
};

struct [[gnu::packed]] Socks4AConnReq {
    unsigned char version;
    enum SOCKS_REQUEST_COMMAND command;
    unsigned short port;
    struct in_addr ip;
    char id_domain[];
};

#define GET_SOCKS4A_DOMAIN(content) \
    ((static_cast<struct Socks4AConnReq *>(content)->id_domain) + \
     strlen(static_cast<struct Socks4AConnReq *>(content)->id_domain) + 1)

struct [[gnu::packed]] Socks4ConnResp {
    unsigned char version;
    enum SOCKS4_RESPONSE_CODE reply_code;
    unsigned short port;
    struct in_addr ip;
};

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
    char addr_and_port[255 + 2];
};

union Socks5AddrUnion {
    struct Socks5IPv4 ipv4_addr;
    struct Socks5IPv6 ipv6_addr;
    struct Socks5Domain domain_addr;
};

struct [[gnu::packed]] Socks5ConnReqHeader {
    unsigned char version;
    enum SOCKS_REQUEST_COMMAND command;
    unsigned char reserved_must_be_0; // this field may be special to the program
    enum SOCKS5_ADDRTYPE addr_type;
};

struct [[gnu::packed]] Socks5ConnReq {
    struct Socks5ConnReqHeader request_header;
    union Socks5AddrUnion addr;
};

struct [[gnu::packed]] Socks5ConnRespHeader {
    unsigned char version;
    enum SOCKS5_RESPONSE_CODE response_code;
    unsigned char reserved_must_be_0; // this field may be special to the program
    enum SOCKS5_ADDRTYPE addr_type;
};

struct [[gnu::packed]] Socks5ConnResp {
    struct Socks5ConnRespHeader response_header;
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

// see https://web.archive.org/web/20090219134914/http://download.microsoft.com/download/9/5/E/95EF66AF-9026-4BB0-A41D-A4F81802D92C/%5BMS-MMSP%5D.pdf
// from https://en.wikipedia.org/wiki/Microsoft_Media_Server
// LinkViewerToMacConnect
// and
// https://web.archive.org/web/20081204082646/http://download.microsoft.com/download/9/5/E/95EF66AF-9026-4BB0-A41D-A4F81802D92C/%5BMS-GLOS%5D.pdf
// for why the Unicode is UTF-16LE

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

#endif // PROTOCOLS_H_INCLUDED
