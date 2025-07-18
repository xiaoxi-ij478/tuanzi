#ifndef TCP_H_INCLUDED
#define TCP_H_INCLUDED

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
        bool IsMine(const struct TCPIP &pkg);

        char reqaddr_char[64]; // m_reqConnAddr
        enum REQUEST_TYPE request_type;
        struct TcpInfo tcpinfo;
        unsigned short reqport; // m_reqPort

    protected:
        in_addr_t reqaddr_int; // m_ulReqAddr
        struct CHostEnt *hostent;
        struct {
            struct Socks5ConnReqHeader request_header;
            char may_be_address[0x101 - sizeof(struct Socks5ConnReqHeader)];
        } socks5_request_addr;
        struct Socks5ConnReqHeader socks5_request_header;
        char socks5_request_domain_len;

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

#endif // TCP_H_INCLUDED
