#ifndef STDPKGS_H_INCLUDED
#define STDPKGS_H_INCLUDED

// for SOCKS related knowledge see https://zh.wikipedia.org/wiki/SOCKS

enum SOCKS5_ADDRTYPE : uint8_t {
    SOCKS5_ADDR_IPV4 = 1,
    SOCKS5_ADDR_DOMAIN = 3,
    SOCKS5_ADDR_IPV6
};

enum SOCKS_REQUEST_COMMAND : uint8_t {
    SOCKS_REQUEST_TCP_CONN = 1,
    SOCKS_REQUEST_TCP_BIND,
    SOCKS_REQUEST_UDP_CONN
};

enum SOCKS4_RESPONSE_CODE : uint8_t {
    SOCKS4_RESPONSE_GRANTED = 0x5A,
    SOCKS4_RESPONSE_REJECTED,
    SOCKS4_RESPONSE_REJECTED_CLIENT_NO_IDENTD,
    SOCKS4_RESPONSE_REJECTED_CLIENT_IDENTD_FAILED
};

enum SOCKS5_RESPONSE_CODE : uint8_t {
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
    uint8_t version;
    enum SOCKS_REQUEST_COMMAND command;
    uint16_t port;
    in_addr_t ip;
    char id[];
};

struct [[gnu::packed]] Socks4AConnReq {
    uint8_t version;
    enum SOCKS_REQUEST_COMMAND command;
    uint16_t port;
    struct in_addr ip;
    char id_domain[];
};

#define GET_SOCKS4A_DOMAIN(content) \
    ((static_cast<const struct Socks4AConnReq *>(content)->id_domain) + \
     strlen(static_cast<const struct Socks4AConnReq *>(content)->id_domain) + 1)

struct [[gnu::packed]] Socks4ConnResp {
    uint8_t version;
    enum SOCKS4_RESPONSE_CODE reply_code;
    uint16_t port;
    struct in_addr ip;
};

struct [[gnu::packed]] Socks5IPv4 {
    struct in_addr ipv4_addr;
    uint16_t port;
};

struct [[gnu::packed]] Socks5IPv6 {
    struct in6_addr ipv6_addr;
    uint16_t port;
};

struct [[gnu::packed]] Socks5Domain {
    uint8_t addr_len;
    uint8_t addr_and_port[255 + 2];
};

union Socks5AddrUnion {
    struct Socks5IPv4 ipv4_addr;
    struct Socks5IPv6 ipv6_addr;
    struct Socks5Domain domain_addr;
};

struct [[gnu::packed]] Socks5ConnReqHeader {
    uint8_t version;
    enum SOCKS_REQUEST_COMMAND command;
    uint8_t reserved_must_be_0; // this field may be special to the program
    enum SOCKS5_ADDRTYPE addr_type;
};

struct [[gnu::packed]] Socks5ConnReq {
    struct Socks5ConnReqHeader request_header;
    union Socks5AddrUnion addr;
};

struct [[gnu::packed]] Socks5ConnRespHeader {
    uint8_t version;
    enum SOCKS5_RESPONSE_CODE response_code;
    uint8_t reserved_must_be_0; // this field may be special to the program
    enum SOCKS5_ADDRTYPE addr_type;
};

struct [[gnu::packed]] Socks5ConnResp {
    struct Socks5ConnRespHeader response_header;
    union Socks5AddrUnion addr;
};

#define GET_SOCKS5_REQUEST_ADDR_DOMAIN(content) \
    (reinterpret_cast<const struct Socks5ConnReq *>(content)->addr.domain_addr)

#define GET_SOCKS5_REQUEST_PORT_DOMAIN(content) \
    (*reinterpret_cast<const uint16_t *> \
     ((GET_SOCKS5_REQUEST_ADDR_DOMAIN(content).addr_and_port + \
       GET_SOCKS5_REQUEST_ADDR_DOMAIN(content).addr_len)))

#define GET_SOCKS5_REQUEST_SIZE_IPV4(content) \
    (sizeof(struct Socks5ConnReqHeader) + \
     sizeof(struct Socks5IPv4))

#define GET_SOCKS5_REQUEST_SIZE_IPV6(content) \
    (sizeof(struct Socks5ConnReqHeader) + \
     sizeof(struct Socks5IPv6))

#define GET_SOCKS5_REQUEST_SIZE_DOMAIN(content) \
    (sizeof(struct Socks5ConnReqHeader) + \
     sizeof(uint8_t) + \
     GET_SOCKS5_REQUEST_ADDR_DOMAIN(content).addr_len + \
     sizeof(uint16_t))

// see https://web.archive.org/web/20090219134914/http://download.microsoft.com/download/9/5/E/95EF66AF-9026-4BB0-A41D-A4F81802D92C/%5BMS-MMSP%5D.pdf
// from https://en.wikipedia.org/wiki/Microsoft_Media_Server
// LinkViewerToMacConnect
// and
// https://web.archive.org/web/20081204082646/http://download.microsoft.com/download/9/5/E/95EF66AF-9026-4BB0-A41D-A4F81802D92C/%5BMS-GLOS%5D.pdf
// for why the Unicode is UTF-16LE

enum PlayIncarnation : uint32_t {
    MMS_DISABLE_PACKET_PAIR = 0xf0f0f0ef,
    MMS_USE_PACKET_PAIR = 0xf0f0f0f0
};

struct [[gnu::packed]] MMSTcpMessage {
    uint8_t rep;
    uint8_t version;
    uint8_t versionMinor;
    uint8_t padding;
    uint32_t sessionId;
    uint32_t messageLength;
    uint32_t seal;
    uint32_t chunkCount;
    uint16_t seq;
    uint16_t MBZ;
    uint64_t timeSent;
    union {
        struct [[gnu::packed]] {
            uint32_t chunkLen;
            uint32_t MID;
            enum PlayIncarnation playIncarnation;
            uint32_t MacToViewerProtocolRevision;
            uint32_t ViewerToMacProtocolRevision;
            wchar_t subscriberName[];
        } LinkViewerToMacConnect;
    } message;
};

struct [[gnu::packed]] tcp_pseudo_hdr {
    uint32_t saddr;
    uint32_t daddr;
    uint8_t zero;
    uint8_t protocol;
    uint16_t tcp_length;
};

struct [[gnu::packed]] tcp_checksum_hdr {
    struct tcp_pseudo_hdr pseudo_header;
    struct tcphdr real_header;
    uint8_t data[2024];
};

struct [[gnu::packed]] udp_pseudo_hdr {
    uint32_t saddr;
    uint32_t daddr;
    uint8_t zero;
    uint8_t protocol;
    uint16_t udp_length;
};

struct [[gnu::packed]] udp_checksum_hdr {
    struct udp_pseudo_hdr pseudo_header;
    struct udphdr real_header;
    uint8_t data[2040];
};

struct [[gnu::packed]] icmppkg {
    struct icmphdr hdr;
    uint8_t data[20];
};

struct [[gnu::packed]] ethertcppkg {
    struct ether_header etherheader;
    struct iphdr ipheader;
    struct tcphdr tcpheader;
};

struct [[gnu::packed]] etherudppkg {
    struct ether_header etherheader;
    struct iphdr ipheader;
    struct udphdr udpheader;
};

enum EAP_CODES : uint8_t {
    EAP_CODE_INVALID,
    EAP_REQUEST,
    EAP_RESPONSE,
    EAP_SUCCESS,
    EAP_FAILURE
};

enum EAP_TYPES : uint8_t {
    EAP_TYPE_INVALID,
    EAP_TYPE_IDENTITY,
    EAP_TYPE_NOTIFICATION,
    EAP_TYPE_NAK,
    EAP_TYPE_MD5,
    EAP_TYPE_OTP,
    EAP_TYPE_GTC,
    EAP_TYPE_RUIJIE_PRIVATE,
    EAP_TYPE_TLS = 13,
    EAP_TYPE_PEAP = 25,
    EAP_TYPE_EXPANDED = 254
};

enum IEEE8021X_PACKET_TYPE : uint8_t {
    IEEE8021X_EAP_PACKET,
    IEEE8021X_EAPOL_START,
    IEEE8021X_EAPOL_LOGOFF,
    IEEE8021X_EAPOL_KEY,
    IEEE8021X_EAPOL_ENCAPSULATED_ASF_ALERT
};

enum IEEE8021X_KEY_DESC_TYPE : uint8_t {
    IEEE8021X_KEY_RC4 = 1
};

struct [[gnu::packed]] eapolpkg {
    struct ether_header etherheader;
    uint8_t ieee8021x_version;
    enum IEEE8021X_PACKET_TYPE ieee8021x_packet_type;
    uint16_t ieee8021x_packet_length;
    union {
        struct [[gnu::packed]] {
            enum EAP_CODES code;
            uint8_t id;
            uint16_t length;
            enum EAP_TYPES type;
            union {
                struct [[gnu::packed]] {
                    uint8_t message[];
                } identity, notification;
                struct [[gnu::packed]] {
                    enum EAP_TYPES expected_eap_types[];
                } nak;
                struct [[gnu::packed]] {
                    uint8_t value_size;
                    uint8_t value_name[];
                } md5;
            } data;
        } eap_packet;
        struct [[gnu::packed]] {
            enum IEEE8021X_KEY_DESC_TYPE desc_type;
            uint16_t key_length;
            uint64_t replay_counter;
            uint8_t key_iv[16];
            uint8_t key_index;
            uint8_t key_signature[16];
            uint8_t key[];
        } key_desc;
    };
};

#endif // STDPKGS_H_INCLUDED
