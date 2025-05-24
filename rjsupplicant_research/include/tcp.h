#ifndef TCP_H
#define TCP_H

#include "netutil.h"
#include "dnsquery.h"
#include "protocols.h"

enum REQUEST_TYPE {
    REQUEST_UNKNOWN_N1 = -1,
    REQUEST_UNKNOWN_0,
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
    TRANS_RECV = -1,
    TRANS_MINE,
    TRANS_SEND
};

struct TcpInfo {
    in_addr_t dstaddr;
    in_addr_t srcaddr;
    unsigned short dstport;
    unsigned short srcport;
    unsigned send_last_seq;
    unsigned recv_last_seq;
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

        bool GetReqAddr_Port(const struct TCPIP &pkg, bool query_hostname);
        int QueryAndUpdate(const struct TCPIP &pkg);
        enum REQUEST_TYPE QueryProtocolType(
            const struct TCPIP &pkg,
            unsigned flag
        );
        enum TRANS_DIRECTION QueryTransTimes(
            unsigned &recv_data_times,
            unsigned &send_data_times
        ) const;

    protected:
        bool IsMine(const struct TCPIP &pkg);

        enum REQUEST_TYPE request_type;
        char reqaddr_char[64]; // m_reqConnAddr
        in_addr_t reqaddr_int; // m_ulReqAddr
        unsigned short reqport; // m_reqPort
        struct CHostEnt *hostent;
        struct TcpInfo tcpinfo;
        struct {
            struct Socks5ConnReqHeader request_header;
            char may_be_address[0x101 - sizeof(struct Socks5ConnReqHeader)];
        } socks5_request_addr;
        struct Socks5ConnReqHeader socks5_request_header;
        unsigned char socks5_request_domain_len;

        static CDNSQuery dns_queryer; // m_dns

    private:
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
        bool IsMmsType(const struct TCPIP &pkg);
        bool IsNntpType(const struct TCPIP &pkg);
        bool IsPop3Type(const struct TCPIP &pkg);
        bool IsSocks4AType(const struct TCPIP &pkg);
        bool IsSocks4Type(const struct TCPIP &pkg);
        bool IsSocks5Type(const struct TCPIP &pkg);
        bool IsTelnetType(const struct TCPIP &pkg);

        unsigned recv_data_times; // revDataTimes
        unsigned send_data_times; // sendDataTimes
        enum TRANS_DIRECTION trans_direction;
};

#endif // TCP_H
