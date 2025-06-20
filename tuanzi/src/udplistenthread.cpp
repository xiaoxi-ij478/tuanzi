#include "all.h"
#include "sudes.h"
#include "cmdutil.h"
#include "threadutil.h"
#include "timeutil.h"
#include "md5checksum.h"
#include "netutil.h"
#include "global.h"
#include "util.h"
#include "udplistenthread.h"

CUDPListenThread::CUDPListenThread(struct UdpListenParam *listen_param) :
    mainthread(listen_param->mainthread),
    listen_port(),
    sam_ipaddr(),
    su_ipaddr(listen_param->su_ipaddr),
    ndisname(),
    event_udp_ready(&listen_param->event_udp_ready),
    timestamps(),
    timestamp_mutex(),
    dir_para(),
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
    dir_para.sender_bind.id = -1;
    dir_para.sender_bind.on_receive_packet_post_mtype = -1;
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

        for (struct tagRecvSessionBind &session_bind : it->recv_session_bounds) {
            delete[] session_bind.data;
            session_bind.data = nullptr;
        }

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
)
{
    const struct DirTransFullPkg *pkg =
            reinterpret_cast<const struct DirTransFullPkg *>(buf);
    assert(buf && buflen);

    if (ntohs(pkg->udpheader.len) == 8) {
        logFile_debug.AppendText("Error-HandlePrivateData uPrivDataLen %u", 0);
        return false;
    }

    if (ResponseSender(buf, buflen))
        SetEvent(dir_para.event_ret, false);

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
    return buflen > sizeof(struct etherudppkg) &&
           pkg->ipheader.version == 4 &&
           pkg->ipheader.ihl == 5 &&
           su_ipaddr == pkg->ipheader.daddr;
}

bool CUDPListenThread::IsUDPChecksumRight(
    const unsigned char *buf,
    unsigned buflen
) const
{
    struct udp_checksum_hdr header = {};
    const struct etherudppkg *pkg =
            reinterpret_cast<const struct etherudppkg *>(buf);
    unsigned checksum_hdrlen = 0;
    assert(buf && buflen);
    checksum_hdrlen =
        buflen -
        offsetof(struct etherudppkg, udpheader) +
        sizeof(struct udp_pseudo_hdr);
#define SET_PSEUDO_HEADER_INFO(name) header.pseudo_header.name = pkg->ipheader.name
    SET_PSEUDO_HEADER_INFO(saddr);
    SET_PSEUDO_HEADER_INFO(daddr);
    SET_PSEUDO_HEADER_INFO(protocol);
    header.pseudo_header.udp_length = pkg->udpheader.len;
#undef SET_PSEUDO_HEADER_INFO
    memcpy(&header.real_header, &pkg->udpheader, sizeof(struct udphdr));
    memcpy(
        header.data,
        buf + sizeof(struct etherudppkg),
        buflen - sizeof(struct etherudppkg)
    );
    return CheckSumForRecv(
               reinterpret_cast<const unsigned char *>(&header),
               checksum_hdrlen
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
    return buflen >= sizeof(struct etherudppkg) &&
           ntohs(pkg->etherheader.ether_type) == ETHERTYPE_IP &&
           pkg->ipheader.protocol == IPPROTO_UDP;
}

void CUDPListenThread::OnRecvPacketReturn(
    unsigned long buflen,
    const void *buf
)
{
    assert(buf);

    if (working_falg)
        HandlePrivateData(static_cast<const unsigned char *>(buf), buflen);

    delete[] buf;
}

void CUDPListenThread::OnTimer(unsigned long buflen, void *buf)
{
    struct tagDirectCom_ProtocalParam proto_param = {};
    unsigned finish_time = 0;
    logFile_debug.AppendText(
        " CUDPListenThread::OnTimer nIDEvent=%d,TIMER_CLEAR_DIR_SENDER=%d ",
        buflen,
        TIMER_CLEAR_DIR_SENDER
    );

    if (buflen != TIMER_CLEAR_DIR_SENDER) {
        OnTimerLeave(buflen);
        return;
    }

    EnterCriticalSection(&recv_mutex);

    for (struct tagRecvBind &recv_bind : gsn_pkgs) {
        proto_param.addr = 0;
        proto_param.port = 0;
        proto_param.su_ipaddr = 0;
        proto_param.dstaddr = 0;
        proto_param.retry_count = 3;
        proto_param.timeout = 3000;
        memset(proto_param.keybuf, 0, sizeof(proto_param.keybuf));
        memset(proto_param.ivbuf, 0, sizeof(proto_param.ivbuf));
        proto_param.check_timestamp = true;
        proto_param.utc_time = 0;
        proto_param.version = 1;
        proto_param.timestamp = GetTickCount();
        proto_param.field_40 = false;
        finish_time =
            GetProtocalParam(proto_param, recv_bind.srcaddr, recv_bind.srcport) ?
            proto_param.timeout * proto_param.retry_count :
            0;

        for (
            auto it = recv_bind.recv_session_bounds.begin();
            it != recv_bind.recv_session_bounds.end();
            it++
        ) {
            if (!it->data_decrypted || GetTickCount() - it->creation_time < finish_time)
                continue;

            logFile_debug.AppendText("删除一个已经完成的会话");
            delete it->data;
            it->data = nullptr;
            it = recv_bind.recv_session_bounds.erase(it) - 1;
        }
    }

    LeaveCriticalSection(&recv_mutex);
    OnTimerLeave(buflen);
}

bool CUDPListenThread::ResponseSender(
    const unsigned char *buf,
    unsigned buflen
)
{
    const struct DirTransFullPkg *pkg =
            reinterpret_cast<const struct DirTransFullPkg *>(buf);
    struct tagDirectCom_ProtocalParam proto_param = {};
    struct tagDirPacketHead dir_head = {};
    unsigned char *checksum_buf = nullptr;
    unsigned char md5_checksum[16] = {};
    char *md5_checksum_ascii = nullptr;

    if (buflen < sizeof(struct DirTransFullPkg) || !listen_res)
        return false;

    if (ntohs(pkg->udpheader.dest) != dir_para.sender_bind.srcport) {
        logFile_debug.AppendText(
            "目的端口号(%d)是不匹配(%d)",
            ntohs(pkg->udpheader.dest),
            dir_para.sender_bind.srcport
        );
        return false;
    }

    if (pkg->ipheader.saddr != dir_para.sender_bind.dstaddr) {
        logFile_debug.AppendText("目的IP是否为预期的，不是");
        return false;
    }

    proto_param.retry_count = 3;
    proto_param.timeout = 3000;
    proto_param.check_timestamp = true;
    proto_param.version = 1;
    proto_param.timestamp = GetTickCount();

    if (
        !GetProtocalParam(
            proto_param,
            dir_para.sender_bind.dstaddr,
            ntohs(pkg->udpheader.source)
        )
    ) {
        logFile_debug.AppendText(
            "ResponseSender>没有找到相应的协议参数 nSrcIP=%x nSrcPort=%d",
            dir_para.sender_bind.dstaddr,
            ntohs(pkg->udpheader.source)
        );
        return false;
    }

#define COPY_FIELD(name, trans_func) dir_head.name = trans_func(pkg->name)
    COPY_FIELD(version,);
    COPY_FIELD(response_code,);
    COPY_FIELD(id, htonl);
    COPY_FIELD(packet_len, htons);
    memcpy(dir_head.md5sum, pkg->md5sum, sizeof(pkg->md5sum));
    COPY_FIELD(session_id, htonl);
    COPY_FIELD(timestamp, htonLONGLONG);
    dir_head.field_28 = pkg->field_24;
    COPY_FIELD(sliceid,);
    COPY_FIELD(data_len, htonl);
#undef COPY_FIELD

    if (dir_head.version != proto_param.version) {
        logFile_debug.AppendText(
            "Version does not match. SU is %d and Radius Server is %d.",
            proto_param.version,
            dir_head.version
        );
        return false;
    }

    if (dir_head.response_code != DIRPACKET_RESPONSE) {
        logFile_debug.AppendText(
            "1=请求，2=响应，必须为响应报文类型 err rep code=%d",
            dir_head.response_code
        );
        return false;
    }

    if (dir_head.id != htonl(dir_para.dir_packet_head.id)) {
        logFile_debug.AppendText("dirHead.ID = %d", htonl(dir_para.dir_packet_head.id));
        return false;
    }

    if (dir_head.packet_len != sizeof(struct mtagFinalDirPacket)) {
        logFile_debug.AppendText(
            "dirHead.packet_len = %d",
            dir_head.packet_len
        );
        return false;
    }

    if (dir_head.session_id != htonl(dir_para.dir_packet_head.session_id)) {
        logFile_debug.AppendText(
            "dirHead.dwSessionID = %d",
            htonl(dir_para.dir_packet_head.session_id)
        );
        return false;
    }

    if (dir_head.field_28 != 1 || dir_head.sliceid != 1 || dir_head.data_len)
        return false;

    if (!IsGoodAsyUTC(proto_param, dir_head.timestamp)) {
        logFile_debug.AppendText(
            "CUDPListenThread::ResponseSender 报文时间戳[%I64d]检验失败",
            dir_head.timestamp
        );
        return false;
    }

    checksum_buf = new unsigned char[dir_head.packet_len + sizeof(md5_checksum)];
    memcpy(
        checksum_buf,
        buf + sizeof(struct etherudppkg),
        dir_head.packet_len
    );
    memcpy(
        checksum_buf + dir_head.packet_len,
        proto_param.keybuf,
        sizeof(proto_param.keybuf)
    );
    memcpy(
        checksum_buf + dir_head.packet_len + sizeof(proto_param.keybuf),
        proto_param.ivbuf,
        sizeof(proto_param.ivbuf)
    );
    memset(
        checksum_buf + offsetof(struct mtagFinalDirPacket, md5sum),
        0,
        sizeof(md5_checksum)
    );
    md5_checksum_ascii =
        CMD5Checksum::GetMD5(
            checksum_buf,
            dir_head.packet_len + sizeof(md5_checksum)
        );
    MD5StrtoUChar(md5_checksum_ascii, md5_checksum);
    delete[] checksum_buf;

    if (memcmp(md5_checksum, dir_head.md5sum, sizeof(md5_checksum))) {
        logFile_debug.AppendText("authenticator[%s]不正确", md5_checksum_ascii);
        delete[] md5_checksum_ascii;
        return false;
    }

    delete[] md5_checksum_ascii;
    return true;
}

bool CUDPListenThread::RevcDirectPack(
    const unsigned char *buf,
    unsigned buflen
)
{
    const struct DirTransFullPkg *pkg =
            reinterpret_cast<const struct DirTransFullPkg *>(buf);
    struct tagDirectCom_ProtocalParam proto_param = {};
    struct tagDirPacketHead dir_head = {};
    struct tagRecvBind tmp_recvbind = {};
    unsigned char *checksum_buf = nullptr;
    unsigned char md5_checksum[16] = {};
    char *md5_checksum_ascii = nullptr;
    bool found_session = false;
    struct tagRecvSessionBind session = {};
    unsigned char *tmp_received_data = nullptr;

    if (buflen < sizeof(struct DirTransFullPkg))
        return false;

    EnterCriticalSection(&recv_mutex);

    if (gsn_pkgs.empty()) {
        LeaveCriticalSection(&recv_mutex);
        return false;
    }

    LeaveCriticalSection(&recv_mutex);
    proto_param.retry_count = 3;
    proto_param.timeout = 3000;
    proto_param.check_timestamp = true;
    proto_param.timestamp = GetTickCount();
    proto_param.version = 1;

    if (
        !GetProtocalParam(
            proto_param,
            pkg->ipheader.saddr,
            ntohs(pkg->udpheader.source)
        )
    )
        return false;

#define COPY_FIELD(name, trans_func) dir_head.name = trans_func(pkg->name)
    COPY_FIELD(version,);
    COPY_FIELD(response_code,);
    COPY_FIELD(id, htonl);
    COPY_FIELD(packet_len, htons);
    memcpy(dir_head.md5sum, pkg->md5sum, sizeof(pkg->md5sum));
    COPY_FIELD(session_id, htonl);
    COPY_FIELD(timestamp, htonLONGLONG);
    dir_head.field_28 = pkg->field_24;
    COPY_FIELD(sliceid,);
    COPY_FIELD(data_len, htonl);
#undef COPY_FIELD

    if (dir_head.version != proto_param.version) {
        logFile_debug.AppendText(
            "Version does not match. SU is %d and radius server is %d.",
            proto_param.version,
            dir_head.version
        );
        return false;
    }

    if (
        dir_head.response_code != DIRPACKET_REQUEST ||
        dir_head.packet_len != buflen - sizeof(struct etherudppkg) ||
        dir_head.field_28 != 1 ||
        !dir_head.data_len
    )
        return false;

    if (!IsGoodAsyUTC(proto_param, dir_head.timestamp)) {
        logFile_debug.AppendText(
            "CUDPListenThread::RevcDirectPack 报文时间戳[%I64d]检验失败",
            dir_head.timestamp
        );
        return false;
    }

    logFile_debug.AppendText(
        "CUDPListenThread::RevcDirectPack 报文时间戳[%I64d]检验成功",
        dir_head.timestamp
    );

    if (dir_head.id == 1 && dir_head.sliceid <= 4) {
        logFile_debug.AppendText("报文ID为1的,不能为中间分片报文或者最后一个分片报文");
        return false;
    }

    logFile_debug.AppendText("报文各基本标识检查成功，开始authenticator MD5校验检查");
    checksum_buf = new unsigned char[dir_head.packet_len +  sizeof(md5_checksum)];
    memcpy(
        checksum_buf,
        buf + sizeof(struct etherudppkg),
        dir_head.packet_len
    );
    memcpy(
        checksum_buf + dir_head.packet_len,
        proto_param.keybuf,
        sizeof(proto_param.keybuf)
    );
    memcpy(
        checksum_buf + dir_head.packet_len + sizeof(proto_param.keybuf),
        proto_param.ivbuf,
        sizeof(proto_param.ivbuf)
    );
    memset(
        checksum_buf + offsetof(struct mtagFinalDirPacket, md5sum),
        0,
        sizeof(md5_checksum)
    );
    md5_checksum_ascii =
        CMD5Checksum::GetMD5(
            checksum_buf,
            dir_head.packet_len + sizeof(md5_checksum)
        );
    MD5StrtoUChar(md5_checksum_ascii, md5_checksum);
    delete[] checksum_buf;

    if (memcmp(md5_checksum, dir_head.md5sum, sizeof(md5_checksum))) {
        logFile_debug.AppendText("authenticator MD5校验检查失败");
        delete[] md5_checksum_ascii;
        return false;
    }

    delete[] md5_checksum_ascii;
    logFile_debug.AppendText("authenticator MD5校验检查成功");
    EnterCriticalSection(&recv_mutex);

    if (gsn_pkgs.empty())
        return true;

    for (const struct tagRecvBind &recv_bind : gsn_pkgs) {
        tmp_recvbind = recv_bind;

        if (
            tmp_recvbind.dstaddr != pkg->ipheader.daddr ||
            tmp_recvbind.dstport != ntohs(pkg->udpheader.dest) ||
            tmp_recvbind.srcport != ntohs(pkg->udpheader.dest) ||
            tmp_recvbind.srcaddr != pkg->ipheader.saddr
        )
            continue;

        found_session = true;
        break;
    }

    if (!found_session) {
        LeaveCriticalSection(&recv_mutex);
        return true;
    }

    CreateSessionIfNecessary(
        tmp_recvbind,
        tmp_recvbind.srcaddr,
        dir_head.session_id,
        session
    );

    if (dir_head.id == 1 || dir_head.sliceid == 2 || dir_head.sliceid == 1) {
        if (session.data && session.received) {
            logFile_debug.AppendText("重新接收一下数据，前面数据丢失掉");
            delete[] session.data;
            session.data = nullptr;
            session.received = 0;
        }

        logFile_debug.AppendText(
            "接收直通报文，第一个分片，总长度[%d]",
            dir_head.data_len
        );
        session.data = new unsigned char[dir_head.data_len];
        memcpy(
            session.data,
            buf + sizeof(struct DirTransFullPkg),
            dir_head.packet_len - sizeof(struct mtagFinalDirPacket)
        );
        session.received =
            dir_head.packet_len +
            session.received -
            sizeof(struct mtagFinalDirPacket);
        session.cur_sliceid = dir_head.id;

    } else {
        if (
            dir_head.id < session.cur_sliceid ||
            session.cur_sliceid < dir_head.id - 1 ||
            dir_head.sliceid == 4 &&
            dir_head.packet_len +
            session.received -
            sizeof(struct mtagFinalDirPacket) != dir_head.data_len
        ) {
            logFile_debug.AppendText(
                "当前分片ID[%d],收到非法的ID[%d] 或者报文长度检验不正确，丢弃该报文分片",
                session.cur_sliceid,
                dir_head.id
            );
            LeaveCriticalSection(&recv_mutex);
            return false;
        }

        if (dir_head.id == session.cur_sliceid)
            logFile_debug.AppendText(
                "当前分片ID[%d],收到重复的分片ID[%d]，丢弃该报文分片，"
                "但要做直通报文反馈，内容暂时不检查",
                dir_head.id,
                dir_head.id
            );

        if (
            session.cur_sliceid == dir_head.id - 1 &&
            dir_head.packet_len - sizeof(struct mtagFinalDirPacket) <=
            dir_head.data_len - session.received
        ) {
            if (session.data) {
                memcpy(
                    &session.data[session.received],
                    buf + sizeof(struct DirTransFullPkg),
                    dir_head.packet_len - sizeof(struct mtagFinalDirPacket)
                );
                session.received =
                    dir_head.packet_len +
                    session.received -
                    sizeof(struct mtagFinalDirPacket);
                session.cur_sliceid = dir_head.id;
                logFile_debug.AppendText(
                    "收到合法ID[%d]分片报文，当前已接收长度[%d]，总长度[%d]",
                    dir_head.id,
                    session.received,
                    dir_head.data_len
                );

            } else
                logFile_debug.AppendText(
                    "收到合法ID[%d]分片报文，但是报文分片[%d]或者长度[%d]不正确",
                    dir_head.id,
                    dir_head.sliceid,
                    dir_head.data_len
                );
        }
    }

    if (session.received == dir_head.data_len) {
        tmp_received_data = new unsigned char[session.received];
        memcpy(tmp_received_data, session.data, dir_head.data_len);

        if (DecryptPrivateData(proto_param, tmp_received_data, dir_head.data_len))
            ::PostThreadMessage(
                tmp_recvbind.pthread,
                tmp_recvbind.on_receive_packet_post_mtype,
                reinterpret_cast<unsigned long>(tmp_received_data),
                reinterpret_cast<void *>(dir_head.data_len)
            );

        else {
            logFile_debug.AppendText(
                "直通报文解密有错，数据总长度[%d]",
                dir_head.data_len
            );
            delete[] tmp_received_data;
        }

        delete[] session.data;
        session.data = nullptr;
        session.received = 0;
        session.data_decrypted = true;
        session.creation_time = GetTickCount();
    }

    SendResponse(
        proto_param,
        dir_head,
        pkg->ipheader.saddr,
        ntohs(pkg->udpheader.source),
        pkg->ipheader.daddr,
        ntohs(pkg->udpheader.dest),
        buf
    );
    LeaveCriticalSection(&recv_mutex);
    return true;
}

void CUDPListenThread::SendResponse(
    const struct tagDirectCom_ProtocalParam &proto_param,
    struct tagDirPacketHead &packet_head,
    in_addr_t dstaddr,
    unsigned dstport,
    in_addr_t srcaddr,
    unsigned srcport,
    const unsigned char *packet
)
{
    struct tagDirPacketHead new_packet_head = {};
    struct tagDirTranPara trans_para = {};
    struct mtagFinalDirPacket final_packet_head = {};
    unsigned char md5_checksum[16] = {};
    char *md5_checksum_ascii = nullptr;
    unsigned char checksum_buf[
     sizeof(struct mtagFinalDirPacket) + sizeof(md5_checksum)
    ] = {};
    assert(packet);
    (
        std::ostringstream()
        << (dstaddr >> 24)
        << '.'
        << (dstaddr >> 16 & 0xff)
        << '.'
        << (dstaddr >> 8 & 0xff)
        << '.'
        << (dstaddr & 0xff)
    ).str().copy(trans_para.dstaddr, sizeof(trans_para.dstaddr));
    trans_para.dstport = dstport;
    (
        std::ostringstream()
        << (srcaddr >> 24)
        << '.'
        << (srcaddr >> 16 & 0xff)
        << '.'
        << (srcaddr >> 8 & 0xff)
        << '.'
        << (srcaddr & 0xff)
    ).str().copy(trans_para.srcaddr, sizeof(trans_para.srcaddr));
    trans_para.srcport = srcport;
    CtrlThread->GetAdapterMac(&trans_para.srcmacaddr);
    trans_para.dstmacaddr =
        *reinterpret_cast<const struct ether_addr *>
        (reinterpret_cast<const struct ether_header *>(packet)->ether_shost);
    dir_trans.InitPara(&trans_para);
    new_packet_head = packet_head;
    new_packet_head.response_code = DIRPACKET_RESPONSE;
    new_packet_head.packet_len = htons(sizeof(struct mtagFinalDirPacket));
    new_packet_head.sliceid = 1;
    new_packet_head.field_28 = packet_head.field_28;
    new_packet_head.session_id = htonl(packet_head.session_id);
    new_packet_head.id = htonl(new_packet_head.id);
    new_packet_head.data_len = 0;

    if (packet_head.version == 1)
        CreateCurrentUTC(
            proto_param.utc_time,
            proto_param.timestamp,
            &new_packet_head.timestamp
        );

    else if (
        !(
            new_packet_head.timestamp =
                GetNextTimeStampForSend(proto_param.addr, proto_param.port)
        )
    )
        logFile_debug.AppendText("Failed to generate time stamp for sending.");

    new_packet_head.timestamp = htonl(new_packet_head.timestamp);
#define COPY_FIELD(name) final_packet_head.name = new_packet_head.name
    COPY_FIELD(version);
    COPY_FIELD(id);
    COPY_FIELD(packet_len);
    memcpy(
        final_packet_head.md5sum,
        new_packet_head.md5sum,
        sizeof(new_packet_head.md5sum)
    );
    COPY_FIELD(session_id);
    COPY_FIELD(timestamp);
    final_packet_head.field_24 = new_packet_head.field_28;
    COPY_FIELD(sliceid);
    COPY_FIELD(data_len);
#undef COPY_FIELD
    *reinterpret_cast<struct mtagFinalDirPacket *>(checksum_buf) =
        final_packet_head;
    memset(
        reinterpret_cast<struct mtagFinalDirPacket *>(checksum_buf)->md5sum,
        0,
        sizeof(md5_checksum)
    );
    memcpy(
        checksum_buf + sizeof(struct mtagFinalDirPacket),
        proto_param.keybuf,
        sizeof(proto_param.keybuf)
    );
    memcpy(
        checksum_buf +
        sizeof(struct mtagFinalDirPacket) +
        sizeof(proto_param.keybuf),
        proto_param.ivbuf,
        sizeof(proto_param.ivbuf)
    );
    md5_checksum_ascii =
        CMD5Checksum::GetMD5(
            checksum_buf,
            sizeof(struct mtagFinalDirPacket) + sizeof(md5_checksum)
        );
    MD5StrtoUChar(md5_checksum_ascii, md5_checksum);
    memcpy(
        reinterpret_cast<struct mtagFinalDirPacket *>(checksum_buf)->md5sum,
        md5_checksum,
        sizeof(md5_checksum)
    );
    dir_trans.Send(&final_packet_head, sizeof(final_packet_head));
}

void CUDPListenThread::SetDirParaXieYi(
    const tagDirectCom_ProtocalParam &proto_param
)
{
    bool found = false;
    EnterCriticalSection(&get_proto_param_mutex);

    for (struct tagDirectCom_ProtocalParam &proto_param_l : proto_params) {
        if (
            // the original implementation checks if proto_param
            // is the same as proto_param_l, but we do not check for that
            proto_param_l->addr != proto_param->addr ||
            proto_param_l->port != proto_param->port
        )
            continue;

        proto_param_l = proto_param;
        found = true;
        break;
    }

    if (!found)
        proto_params.push_back(proto_param);

    su_ipaddr = proto_param.su_ipaddr;
    LeaveCriticalSection(&get_proto_param_mutex);

    if (proto_param->version != 1)
        InitTimeStampV2(proto_param->addr, proto_param->port, proto_param->utc_time);
}

void CUDPListenThread::SetIfListenRes(bool val)
{
    listen_res = val;
}

void CUDPListenThread::SetLastTimeStampForReceive(
    in_addr_t addr,
    unsigned short port,
    unsigned long timestamp
)
{
    EnterCriticalSection(&timestamp_mutex);

    for (struct tagTimeStampV2 &timestamp_l : timestamps) {
        if (timestamp_l.addr != addr || timestamp_l.port != port)
            continue;

        timestamp_l->last_received_timestamp = timestamp;
        LeaveCriticalSection(&timestamp_mutex);
        return;
    }

    assert(false);
}

void CUDPListenThread::SetListenPort(unsigned short port)
{
    listen_port = port;
}

void CUDPListenThread::SetMainThread(key_t mainthread_l)
{
    mainthread = mainthread_l;
}

void CUDPListenThread::SetNDISName(unsigned char *ndisname_l)
{
    assert(ndisname_l);
    strcpy(ndisname, ndisname_l);
}

void CUDPListenThread::SetOutOfOrderNum(
    in_addr_t addr,
    unsigned short port,
    unsigned out_of_order_num
)
{
    EnterCriticalSection(&timestamp_mutex);

    for (struct tagTimeStampV2 &timestamp_l : timestamps) {
        if (timestamp_l.addr != addr || timestamp_l.port != port)
            continue;

        timestamp_l->out_of_order_num = out_of_order_num;
        LeaveCriticalSection(&timestamp_mutex);
        return;
    }

    assert(false);
}

bool CUDPListenThread::SetProtocalParam_TimeStamp(
    in_addr_t addr,
    unsigned short port,
    unsigned long long utc_time,
    unsigned long timestamp
)
{
    bool found = false;
    EnterCriticalSection(&get_proto_param_mutex);

    for (struct tagDirectCom_ProtocalParam &proto_param_l : proto_params) {
        if (
            proto_param_l->addr != proto_param->addr ||
            proto_param_l->port != proto_param->port
        )
            continue;

        proto_param_l.utc_time = utc_time;
        proto_param_l.timestamp = timestamp;
        proto_param_l.check_timestamp = utc_time;
        LeaveCriticalSection(&get_proto_param_mutex);

        if (proto_param_l.version != 1)
            InitTimeStampV2(addr, port, utc_time);

        return true;
    }

    LeaveCriticalSection(&get_proto_param_mutex);
    return false;
}

void CUDPListenThread::SetResSender(
    const struct tagDirResPara &res_sender
)
{
    dir_para = res_sender;
}

void CUDPListenThread::SetSamIPAddress(in_addr_t sam_ipaddr_l)
{
    sam_ipaddr = sam_ipaddr_l;
}

void CUDPListenThread::SetSuIPAddress(in_addr_t su_ipaddr_l)
{
    su_ipaddr = su_ipaddr_l;
}

void CUDPListenThread::SetWorkingFalg(bool working)
{
    working_falg = working;
    g_log_Wireless.AppendText(
        "CUDPListenThread::SetWorkingFalg(BOOL bWorking=%d)",
        working
    );

    if (!working_falg) {
        g_log_Wireless.AppendText(
            "CUDPListenThread::SetWorkingFalg(BOOL bWorking)_m_nTimerClear=%d",
            clear_timer_id
        );

        if (clear_timer_id) {
            OnTimer(TIMER_CLEAR_DIR_SENDER, reinterpret_cast<void *>(-1));
            KillTimer(clear_timer_id);
            clear_timer_id = 0;
        }

    } else if (!clear_timer_id)
        clear_timer_id = SetTimer(TIMER_CLEAR_DIR_SENDER, 180000);
}

void CUDPListenThread::freeMemory()
{
    EnterCriticalSection(&recv_mutex);

    for (auto it = gsn_pkgs.begin(); it != gsn_pkgs.end(); it++) {
        for (struct tagRecvSessionBind &session_bind : it->recv_session_bounds) {
            delete[] session_bind.data;
            session_bind.data = nullptr;
        }

        it = gsn_pkgs.erase(it) - 1;
    }

    LeaveCriticalSection(&recv_mutex);
}

unsigned short CUDPListenThread::CheckSumForRecv(
    const unsigned char *buf,
    unsigned buflen
)
{
    return checksum(reinterpret_cast<const unsigned short *>(buf), buflen);
}
