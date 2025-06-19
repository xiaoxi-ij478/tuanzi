#ifndef DIRTRANSTAGS_H_INCLUDED
#define DIRTRANSTAGS_H_INCLUDED

#include "stdpkgs.h"
#include "waithandle.h"

struct tagDirectCom_ProtocalParam {
    in_addr_t addr;
    unsigned short port;
    in_addr_t su_ipaddr;
    in_addr_t dstaddr;
    unsigned retry_count;
    unsigned timeout;
    unsigned char keybuf[8];
    unsigned char ivbuf[8];
    bool check_timestamp;
    unsigned long utc_time;
    unsigned long timestamp;
    bool field_40;
    unsigned char version;
};

struct tagRecvSessionBind {
    tagRecvSessionBind() = default;
    tagRecvSessionBind(
        unsigned session_id,
        in_addr_t srcaddr,
        unsigned srcport,
        in_addr_t dstaddr,
        unsigned dstport,
        unsigned on_receive_packet_post_mtype,
        unsigned char *data,
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
    unsigned on_receive_packet_post_mtype;
    unsigned char *data;
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
        unsigned on_receive_packet_post_mtype
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
    unsigned on_receive_packet_post_mtype;
    std::vector<struct tagRecvSessionBind> recv_session_bounds;
};

enum DIRPACKETRESPCODE : uint8_t {
    DIRPACKET_REQUEST = 1,
    DIRPACKET_RESPONSE
};

struct [[gnu::packed]] mtagFinalDirPacket {
    uint8_t version;
    enum DIRPACKETRESPCODE response_code;
    uint32_t id;
    uint16_t packet_len;
    uint8_t md5sum[16];
    uint32_t session_id;
    uint64_t timestamp;
    uint8_t field_24;
    uint8_t sliceid;
    uint32_t data_len;
};

struct [[gnu::packed]] DirTransFullPkg : etherudppkg, mtagFinalDirPacket {};

struct tagDirPacketHead {
    unsigned char version;
    enum DIRPACKETRESPCODE response_code;
    unsigned id;
    unsigned short packet_len;
    unsigned char md5sum[16];
    unsigned session_id;
    unsigned long timestamp;
    bool field_28;
    unsigned char sliceid;
    unsigned data_len;
};

struct tagSenderBind {
    unsigned id;
    in_addr_t srcaddr;
    unsigned srcport;
    in_addr_t dstaddr;
    unsigned dstport;
    unsigned on_receive_packet_post_mtype;
};

struct tagDirResPara {
    struct tagSenderBind sender_bind;
    WAIT_HANDLE *event_ret;
    struct tagDirPacketHead dir_packet_head;
};

struct tagTimeStampV2 {
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

#endif // DIRTRANSTAGS_H_INCLUDED
