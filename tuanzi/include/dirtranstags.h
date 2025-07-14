#ifndef DIRTRANSTAGS_H_INCLUDED
#define DIRTRANSTAGS_H_INCLUDED

#include "stdpkgs.h"
#include "waithandle.h"

#define MAX_MTU 1400u

enum DIRPACKET_SLICETYPE : uint8_t {
    DIRPACKET_SINGLE = 1,
    DIRPACKET_MULTI_BEGIN,
    DIRPACKET_MULTI_MIDDLE,
    DIRPACKET_MULTI_END
};

struct tagDirectCom_ProtocalParam {
    in_addr_t addr;
    unsigned short port;
    in_addr_t su_ipaddr;
    in_addr_t dstaddr;
    unsigned retry_count;
    unsigned timeout;
    char keybuf[8];
    char ivbuf[8];
    bool check_timestamp;
    unsigned long utc_time;
    unsigned long timestamp;
    bool field_40;
    char version;
};

struct tagRecvSessionBind {
    tagRecvSessionBind() = default;
    tagRecvSessionBind(
        unsigned session_id,
        in_addr_t srcaddr,
        unsigned srcport,
        in_addr_t dstaddr,
        unsigned dstport,
        int on_receive_packet_post_mtype,
        char *data,
        unsigned received,
        unsigned cur_sliceid,
        unsigned long creation_time,
        bool data_decrypted
    ) :
        session_id(session_id),
        srcaddr(srcaddr),
        srcport(srcport),
        dstaddr(dstaddr),
        dstport(dstport),
        on_receive_packet_post_mtype(on_receive_packet_post_mtype),
        data(data),
        received(received),
        cur_sliceid(cur_sliceid),
        creation_time(creation_time),
        data_decrypted(data_decrypted)
    {}
    unsigned session_id;
    in_addr_t srcaddr;
    unsigned srcport;
    in_addr_t dstaddr;
    unsigned dstport;
    int on_receive_packet_post_mtype;
    char *data;
    unsigned received;
    unsigned cur_sliceid;
    unsigned long creation_time;
    bool data_decrypted;
};

struct tagRecvBind {
    tagRecvBind() = default;
    tagRecvBind(
        unsigned id,
        in_addr_t srcaddr,
        unsigned srcport,
        in_addr_t dstaddr,
        unsigned dstport,
        key_t pthread,
        int on_receive_packet_post_mtype
    ) :
        id(id),
        srcaddr(srcaddr),
        srcport(srcport),
        dstaddr(dstaddr),
        dstport(dstport),
        pthread(pthread),
        on_receive_packet_post_mtype(on_receive_packet_post_mtype)
    {}
    unsigned id;
    in_addr_t srcaddr;
    unsigned srcport;
    in_addr_t dstaddr;
    unsigned dstport;
    key_t pthread;
    int on_receive_packet_post_mtype;
    std::vector<struct tagRecvSessionBind> recv_session_bounds;
};

enum DIRPACKET_RESPCODE : uint8_t {
    DIRPACKET_REQUEST = 1,
    DIRPACKET_RESPONSE
};

struct [[gnu::packed]] mtagFinalDirPacket {
    uint8_t version;
    enum DIRPACKET_RESPCODE response_code;
    uint32_t id;
    uint16_t packet_len;
    uint8_t md5sum[16];
    uint32_t session_id;
    uint64_t timestamp;
    uint8_t field_24;
    enum DIRPACKET_SLICETYPE slicetype;
    uint32_t data_len;
};

struct [[gnu::packed]] DirTransFullPkg : etherudppkg, mtagFinalDirPacket {};

struct tagDirPacketHead {
    char version;
    enum DIRPACKET_RESPCODE response_code;
    unsigned id;
    unsigned short packet_len;
    char md5sum[16];
    unsigned session_id;
    unsigned long timestamp;
    bool field_28;
    enum DIRPACKET_SLICETYPE slicetype;
    unsigned data_len;
};

struct tagSenderBind {
    tagSenderBind() = default;
    tagSenderBind(
        int id,
        in_addr_t srcaddr,
        unsigned srcport,
        in_addr_t dstaddr,
        unsigned dstport,
        int on_receive_packet_post_mtype
    ) :
        id(id),
        srcaddr(srcaddr),
        srcport(srcport),
        dstaddr(dstaddr),
        dstport(dstport),
        on_receive_packet_post_mtype(on_receive_packet_post_mtype)
    {}

    int id;
    in_addr_t srcaddr;
    unsigned srcport;
    in_addr_t dstaddr;
    unsigned dstport;
    int on_receive_packet_post_mtype;
};

struct tagDirResPara {
    struct tagSenderBind sender_bind;
    WAIT_HANDLE *event_ret;
    struct tagDirPacketHead dir_packet_head;
};

struct tagTimeStampV2 {
    tagTimeStampV2() = default;
    tagTimeStampV2(
        in_addr_t addr,
        unsigned short port,
        unsigned short field_6,
        unsigned long next_send_timestamp,
        unsigned long last_received_timestamp,
        unsigned out_of_order_num,
        unsigned field_1C
    ) :
        addr(addr),
        port(port),
        field_6(field_6),
        next_send_timestamp(next_send_timestamp),
        last_received_timestamp(last_received_timestamp),
        out_of_order_num(out_of_order_num),
        field_1C(field_1C)
    {}
    in_addr_t addr;
    unsigned short port;
    unsigned short field_6;
    unsigned long next_send_timestamp;
    unsigned long last_received_timestamp;
    unsigned out_of_order_num;
    unsigned field_1C;
};

struct tagSmpInitPacket_ARPAttackDetection {
    bool is_detection;
    unsigned short dontknowwhat;
    struct ether_addr gateway_mac;
    in_addr_t gateway_ip;
    unsigned report_interval;
};

struct tagSmpInitPacket_SendPacketCheck {
    unsigned cycle;
    unsigned threshold;
    std::string warning_message;
    bool offline;
    std::string offline_message;
};

struct tagSmpInitPacket {
    std::string smp_current_time;
    struct {
        unsigned hi_detect_interval;
        unsigned hello_interval;
        std::string hello_response;
        unsigned hostinfo_report_interval;
        unsigned timeout;
        unsigned retry_times;
        std::string login_url;
        std::string disable_arpbam;
        std::string disable_dhcpbam;
    } basic_config;
    struct {
        unsigned enabled;
        std::string gateway_ip;
        std::string gateway_mac;
    } arp;
    struct {
        unsigned enabled;
        std::string syslog_ip;
        unsigned syslog_port;
        unsigned detect_interval;
        unsigned is_block;
        std::string block_tip;
    } illegal_network_detect;
    std::string hi_xml;
    std::string security_domain_xml;
};

struct tagDataSendUnit {
    unsigned id;
    unsigned field_4;
    char *msg;
    unsigned totallen;
    unsigned field_14;
    WAIT_HANDLE *eventret;
    unsigned *ret;
    bool need_reply;
    unsigned session_id;
};

struct tagRetPara {
    tagRetPara() = default;
    tagRetPara(unsigned *ret, WAIT_HANDLE *eventret) :
        ret(ret), eventret(eventret)
    {}
    unsigned *ret;
    WAIT_HANDLE *eventret;
};

struct tagDirTranPara {
    char dstaddr[20];
    unsigned short dstport;
    char srcaddr[20];
    unsigned short srcport;
    struct ether_addr srcmacaddr;
    char field_32[20];
    struct ether_addr dstmacaddr;
    bool field_4C;
    unsigned long field_50;
    char data[MAX_MTU];
    unsigned mtu;
    unsigned field_5D4;
};

struct tagSmpParaDir {
    char username[128];
    in_addr_t su_ipaddr;
    struct ether_addr adapter_mac;
    char diskid[128];
    unsigned diskid_len;
    in_addr_t smp_ipaddr;
    in_addr_t gateway_ipaddr;
    unsigned smp_port;
    unsigned su_port;
    char keybuf[8];
    char ivbuf[8];
    unsigned long utc_time;
    unsigned timeout;
    unsigned retry_count;
    char version;
    unsigned hello_interval;
    bool hello_response;
    unsigned hello_fail_time;
};

struct tagDirectTranSrvPara {
    bool field_0;
    bool use_handshake_to_sam;
    unsigned timer_to_sam;
    char username[128];
    in_addr_t su_ipaddr;
    struct ether_addr adapter_mac;
    in_addr_t sam_ipaddr;
    in_addr_t gateway_ipaddr;
    unsigned sam_port;
    unsigned su_port;
    char keybuf[8];
    char ivbuf[8];
    unsigned long utc_time;
    unsigned timeout;
    unsigned retry_count;
    char version;
    bool server_and_us_in_the_same_subnet;
};

struct tagPasSecurityInfo {
    bool enable_modify_pw;
    unsigned timeout;
    unsigned result;
    std::string password_modify_message;
    unsigned failcode;
    unsigned force_offline;
    unsigned offline_wait_time;
};

struct tagWirelessConf {
    std::string field_0;
    std::string field_8;
    char field_10[32];
    unsigned field_10_len;
    std::string field_38;
    std::string field_40;
    struct ether_addr macaddr;
    unsigned field_50;
};

#endif // DIRTRANSTAGS_H_INCLUDED
