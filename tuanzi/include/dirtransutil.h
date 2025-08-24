#ifndef DIRTRANSUTIL_H_INCLUDED
#define DIRTRANSUTIL_H_INCLUDED

#include "dirtranstags.h"

extern void InitSmpInitPacket(struct tagSmpInitPacket &packet);
extern void CreateDirPktHead(
    struct mtagFinalDirPacket &final_packet_head,
    struct tagDirPacketHead &packet_head,
    struct tagSenderBind &sender_bind,
    char *buf,
    unsigned buflen,
    char *keybuf,
    char *ivbuf
);
extern void CreateSessionIfNecessary(
    struct tagRecvBind &gsn_pkgs,
    in_addr_t srcaddr,
    unsigned session_id,
    struct tagRecvSessionBind &recv_session
);
extern void CopyDirTranPara(
    struct tagDirTranPara *dst,
    const struct tagDirTranPara *src
);

#endif // DIRTRANSUTIL_H_INCLUDED
