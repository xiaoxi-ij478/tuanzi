#include "all.h"
#include "sudes.h"
#include "cmdutil.h"
#include "threadutil.h"
#include "timeutil.h"
#include "netutil.h"
#include "criticalsection.h"
#include "global.h"
#include "udplistenthread.h"

CUDPListenThread::CUDPListenThread(
    struct UdpListenParam *listen_param
) :
    mainthread(listen_param->mainthread),
    listen_port(),
    sam_ipaddr(),
    su_ipaddr(listen_param->su_ipaddr),
    ndisname(),
    event_udp_ready(&listen_param->event_udp_ready),
    timestamps(),
    timestamp_mutex(),
    dir_head(),
    working_falg(),
    listen_res(),
    proto_params(),
    get_proto_param_mutex(),
    gsn_pkgs(),
    recv_mutex(),
    gsn_pkgid(1),
    dir_trans(),
    clear_timer_id()
{
    assert(listen_param);
    strcpy(ndisname, listen_param->ndisname);
    dir_head.sender_bind.id = -1;
    dir_head.sender_bind.on_receive_packet_post_mtype = 1;
    InitializeCriticalSection(&timestamp_mutex);
    InitializeCriticalSection(&recv_mutex);
    InitializeCriticalSection(&get_proto_param_mutex);
    SetClassName("CUDPListenThread");
}

CUDPListenThread::~CUDPListenThread()
{
    DeleteCriticalSection(&timestamp_mutex);
    DeleteCriticalSection(&recv_mutex);
    DeleteCriticalSection(&get_proto_param_mutex);
}

unsigned CUDPListenThread::GSNReceiver(
    in_addr_t srcaddr,
    unsigned srcport,
    in_addr_t dstaddr,
    unsigned dstport,
    key_t pthread,
    unsigned on_receive_packet_post_mtype
)
{
    EnterCriticalSection(&recv_mutex);
    gsn_pkgs.emplace_back(
        gsn_pkgid,
        srcaddr,
        srcport,
        dstaddr,
        dstport,
        pthread,
        on_receive_packet_post_mtype
    );
    LeaveCriticalSection(&recv_mutex);
    return gsn_pkgid++;
}

bool CUDPListenThread::DispathMessage(struct LNXMSG *msg)
{
    switch (msg->mtype) {
        case RECV_PACKET_RETURN_MTYPE:
            OnRecvPacketReturn(msg->buflen, msg->buf);
            break;

        case ON_TIMER_MTYPE:
            OnTimer(msg->buflen, msg->buf);
            break;
    }
}

bool CUDPListenThread::ExitInstance()
{
    logFile_debug.AppendText("CUDPListenThread::ExitInstance()");

    if (clear_timer_id)
        KillTimer(clear_timer_id);

    return CLnxThread::ExitInstance();
}

bool CUDPListenThread::InitInstance()
{
    if (CtrlThread->read_packet_thread)
        CtrlThread->read_packet_thread->SetDirectMsgID(GetMessageID());

    else
        logFile_debug.AppendText(
            "CUDPListenThread::InitInstance() CtrlThread->m_readPacketThread== NULL,ERROR!"
        );

    SetEvent(event_udp_ready, true);
    rj_printf_debug(
        "CUDPListenThread::InitInstance() bSignal =%d,m_hEventUDPReady=%p\n",
        event_udp_ready->signal,
        &event_udp_ready
    );
    return CLnxThread::InitInstance();
}

void CUDPListenThread::OnTimer(int tflag) const
{
    if (OnTimerEnter(tflag)) {
        if (!PostThreadMessage(ON_TIMER_MTYPE, tflag, reinterpret_cast<void *>(-1)))
            OnTimerLeave(tflag);

    } else
        g_logSystem.AppendText("CUDPListenThread::OnTimer(timerFlag=%d),return", tflag);
}

bool CUDPListenThread::CloseGSNReceiver(unsigned id)
{
    EnterCriticalSection(&recv_mutex);

    for (auto it = gsn_pkgs.begin(); it != gsn_pkgs.end(); it++) {
        if (it->id != id)
            continue;

        for (struct tagRecvSessionBind &recv_session_bind : it->recv_session_bounds)
            delete recv_session_bind.data;

        it = gsn_pkgs.erase(it) - 1;
    }

    LeaveCriticalSection(&recv_mutex);
    return true;
}

bool CUDPListenThread::DecryptPrivateData(
    const struct tagDirectCom_ProtocalParam &proto_param,
    unsigned char *buf,
    unsigned buflen
) const
{
    CSuDES sudes;
    assert(buf && buflen);
    return
        sudes.SetIVBuf(proto_param.ivbuf, sizeof(proto_param.ivbuf)) &&
        sudes.SetKeyBuf(proto_param.keybuf, sizeof(proto_param.keybuf)) &&
        sudes.Decrypts(buf, buflen);
}

bool CUDPListenThread::EncryptPrivateData(
    const struct tagDirectCom_ProtocalParam &proto_param,
    unsigned char *buf,
    unsigned buflen
) const
{
    CSuDES sudes;
    assert(buf && buflen);
    return
        sudes.SetIVBuf(proto_param.ivbuf, sizeof(proto_param.ivbuf)) &&
        sudes.SetKeyBuf(proto_param.keybuf, sizeof(proto_param.keybuf)) &&
        sudes.Encrypts(buf, buflen);
}

unsigned long CUDPListenThread::GetLastTimeStampForReceive(
    in_addr_t addr,
    unsigned short port
)
{
    EnterCriticalSection(&timestamp_mutex);

    for (const struct tagTimeStampV2 &timestamp : timestamps) {
        if (timestamp.addr != addr || timestamp.port != port)
            continue;

        LeaveCriticalSection(&timestamp_mutex);
        return timestamp.last_received_timestamp;
    }

    assert(false);
}

unsigned long CUDPListenThread::GetNextTimeStampForSend(
    in_addr_t addr,
    unsigned short port
)
{
    EnterCriticalSection(&timestamp_mutex);

    for (struct tagTimeStampV2 &timestamp : timestamps) {
        if (timestamp.addr != addr || timestamp.port != port)
            continue;

        LeaveCriticalSection(&timestamp_mutex);
        return ++timestamp.next_send_timestamp;
    }

    assert(false);
}

unsigned CUDPListenThread::GetOutOfOrderNum(
    in_addr_t addr,
    unsigned short port
)
{
    EnterCriticalSection(&timestamp_mutex);

    for (const struct tagTimeStampV2 &timestamp : timestamps) {
        if (timestamp.addr != addr || timestamp.port != port)
            continue;

        LeaveCriticalSection(&timestamp_mutex);
        return timestamp.out_of_order_num;
    }

    assert(false);
}

bool CUDPListenThread::GetProtocalParam(
    struct tagDirectCom_ProtocalParam &proto_param,
    in_addr_t addr,
    unsigned short port
)
{
    EnterCriticalSection(&get_proto_param_mutex);

    for (const struct tagDirectCom_ProtocalParam &proto_param_l : proto_params) {
        if (proto_param_l.addr != addr || proto_param_l.port != port)
            continue;

        proto_param = proto_param_l;
        proto_param.addr = addr;
        LeaveCriticalSection(&get_proto_param_mutex);
        return true;
    }

    LeaveCriticalSection(&get_proto_param_mutex);
    return false;
}

bool CUDPListenThread::HandlePrivateData(
    const unsigned char *buf,
    unsigned buflen
) const
{
    const struct DirTransFullPkg *pkg =
            reinterpret_cast<const struct DirTransFullPkg *>(buf);
    assert(buf && buflen);

    if (ntohs(pkg->udpheader.len) == 8) {
        logFile_debug.AppendText("Error-HandlePrivateData uPrivDataLen %u", 0);
        return false;
    }

    if (CUDPListenThread::ResponseSender(buf, buflen))
        SetEvent(dir_head.event_ret, false);

    else
        CUDPListenThread::RevcDirectPack(buf, buflen);

    return true;
}

void CUDPListenThread::InitTimeStampV2(
    in_addr_t addr,
    unsigned short port,
    unsigned long next_timestamp_for_send
)
{
    EnterCriticalSection(&timestamp_mutex);

    for (struct tagTimeStampV2 &timestamp : timestamps) {
        if (timestamp.addr != addr || timestamp.port != port)
            continue;

        timestamp.next_send_timestamp = next_timestamp_for_send;
        logFile_debug.AppendText(
            "InitTimeStampV2 更新下发送的时间戳 nextTimeStampForSend=%I64d",
            next_timestamp_for_send
        );
        LeaveCriticalSection(&timestamp_mutex);
        return;
    }

    logFile_debug.AppendText(
        "InitTimeStampV2 新的连接 nextTimeStampForSend=%I64d",
        next_timestamp_for_send
    );
    timestamps.emplace_back(addr, port, 0, next_timestamp_for_send, 0, 0, 0);
    LeaveCriticalSection(&timestamp_mutex);
}

bool CUDPListenThread::IsGoodAsyUTC(
    const struct tagDirectCom_ProtocalParam &proto_param,
    unsigned long timestamp
)
{
    unsigned long last_timestamp = 0;
    unsigned outoforder_num = 0;

    if (!proto_param.check_timestamp)
        return true;

    if (proto_param.version == 1)
        return labs(
                   timestamp -
                   proto_param.utc_time -
                   (GetTickCount() - proto_param.timestamp) / 1000
               ) <= 60;

    if (
        (last_timestamp = GetLastTimeStampForReceive(
                              proto_param.addr,
                              proto_param.port
                          ))
        >= timestamp
    ) {
        outoforder_num = GetOutOfOrderNum(proto_param.addr, proto_param.port);
        logFile_debug.AppendText(
            "时间戳校验失败，上一次报文的时间戳为%I64d，"
            "本次报文的时间戳为%I64d，接收到乱序报文的次数为%d.",
            last_timestamp,
            timestamp,
            outoforder_num
        );

        if (outoforder_num <= 4) {
            SetOutOfOrderNum(proto_param.addr, proto_param.port, ++outoforder_num);
            return true;
        }

        return false;
    }

    SetLastTimeStampForReceive(proto_param.addr, proto_param.port, timestamp);
    SetOutOfOrderNum(proto_param.addr, proto_param.port, 0);
    return true;
}

bool CUDPListenThread::IsIPHeadChecksumRight(
    const unsigned char *buf,
    unsigned buflen
) const
{
    assert(buf && buflen);
    return CheckSumForRecv(
               buf + offsetof(struct etherudppkg, ipheader),
               sizeof(struct iphdr)
           ) == 0xFFFF;
}

bool CUDPListenThread::IsSuExpected(
    const unsigned char *buf,
    unsigned buflen
) const
{
    assert(buf && buflen);
    const struct etherudppkg *pkg =
            reinterpret_cast<const struct etherudppkg *>(buf);
    return buflen > 0x2A &&
           pkg->ipheader.version == 4 &&
           pkg->ipheader.ihl == 5 &&
           su_ipaddr == pkg->ipheader.daddr;
}

bool CUDPListenThread::IsUDPChecksumRight(
    const unsigned char *buf,
    unsigned buflen
) const
{
    struct udp_checksum_hdr *header = nullptr;
    const struct etherudppkg *pkg =
            reinterpret_cast<const struct etherudppkg *>(buf);
    assert(buf && buflen);
    header = reinterpret_cast<struct udp_checksum_hdr *>(new unsigned char[
              buflen -
              offsetof(struct etherudppkg, udpheader) +
              sizeof(struct udp_pseudo_hdr)
          ]);
#define SET_PSEUDO_HEADER_INFO(name) header->pseudo_header.name = pkg->ipheader.name
    SET_PSEUDO_HEADER_INFO(saddr);
    SET_PSEUDO_HEADER_INFO(daddr);
    SET_PSEUDO_HEADER_INFO(protocol);
    header->pseudo_header.udp_length = pkg->udpheader.len;
#undef SET_PSEUDO_HEADER_INFO
    memcpy(
        &header->real_header,
        &pkg->udpheader,
        buflen - offsetof(struct etherudppkg, udpheader)
    );
    return CheckSumForRecv(
               reinterpret_cast<const unsigned char *>(&header),
               buflen -
               offsetof(struct etherudppkg, udpheader) +
               sizeof(struct udp_pseudo_hdr)
           ) == 0xFFFF;
}

bool CUDPListenThread::IsUDPPacket(
    const unsigned char *buf,
    unsigned buflen
) const
{
    const struct etherudppkg *pkg =
            reinterpret_cast<const struct etherudppkg *>(buf);
    assert(buf && buflen);
    return buflen > 0x29 &&
           ntohs(pkg->etherheader.ether_type) == ETHERTYPE_IP &&
           pkg->ipheader.protocol == IPPROTO_UDP;
}

void CUDPListenThread::OnRecvPacketReturn(
    unsigned long buflen,
    const void *buf
) const
{
    assert(buf);

    if (working_falg)
        HandlePrivateData(buf, buflen);

    delete[] buf;
}

void CUDPListenThread::OnTimer(unsigned long buflen, void *buf) const
{
    struct tagDirectCom_ProtocalParam query={};
    logFile_debug.AppendText(
        " CUDPListenThread::OnTimer nIDEvent=%d,TIMER_CLEAR_DIR_SENDER=%d ",
        buflen,
        TIMER_CLEAR_DIR_SENDER
    );
    if(buflen!=TIMER_CLEAR_DIR_SENDER)
    {
        OnTimerLeave(buflen);return;
    }
    EnterCriticalSection(&recv_mutex);
    for (const struct tagRecvBind &recv_bind:gsn_pkgs)
    {
        query.addr=0;
        query.port=0;
        query.su_ipaddr=0;
        query.dstaddr=0;

    }
}

bool CUDPListenThread::ResponseSender(
    const unsigned char *buf,
    unsigned buflen
) const
{
}

bool CUDPListenThread::RevcDirectPack(
    const unsigned char *buf,
    unsigned buflen
) const
{
}

bool CUDPListenThread::SendResponse(
    const struct tagDirectCom_ProtocalParam &proto_param,
    struct tagDirPackethead &packet_head,
    in_addr_t dstaddr,
    unsigned dstport,
    in_addr_t srcaddr,
    unsigned srcport,
    const char *packet
) const
{
}

void CUDPListenThread::SetDirParaXieYi(
    const tagDirectCom_ProtocalParam &proto_param
) const
{
}

void CUDPListenThread::SetIfListenRes(bool val) const
{
}

void CUDPListenThread::SetLastTimeStampForReceive(
    in_addr_t addr,
    unsigned short port,
    unsigned long timestamp
) const
{
}

void CUDPListenThread::SetListenPort(unsigned short port) const
{
}

void CUDPListenThread::SetMainThread(key_t mainthread_l) const
{
}

void CUDPListenThread::SetNDISName(unsigned char *ndisname_l) const
{
}

void CUDPListenThread::SetOutOfOrderNum(
    in_addr_t addr,
    unsigned short port,
    unsigned outoforder_num
) const
{
}

void CUDPListenThread::SetProtocalParam_TimeStamp(
    in_addr_t addr,
    unsigned short port,
    unsigned long long utc_time,
    unsigned long timestamp
) const
{
}

void CUDPListenThread::SetResSender(const struct tagDirResPara &res_sender)
const
{
}

void CUDPListenThread::SetSamIPAddress(in_addr_t sam_ipaddr_l) const
{
}

void CUDPListenThread::SetSuIPAddress(in_addr_t su_ipaddr_l) const
{
}

void CUDPListenThread::SetWorkingFalg(bool working_falg_l) const
{
}

void CUDPListenThread::freeMemory() const
{
}

unsigned short CUDPListenThread::CheckSumForRecv(
    const unsigned char *buf,
    unsigned buflen
)
{
    return checksum(reinterpret_cast<const unsigned short *>(buf), buflen);
}
