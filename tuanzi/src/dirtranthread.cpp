#include "all.h"
#include "timeutil.h"
#include "global.h"
#include "threadutil.h"
#include "netutil.h"
#include "sudes.h"
#include "dirtranthread.h"

CDirTranThread::CDirTranThread() :
    dir_transpara(),
    dir_respara(),
    running_dir(),
    send_bind(),
    send_bind_mutex(),
    data_send(),
    data_send_mutex(),
    proto_params(),
    get_set_proto_param_mutex(),
    next_alloc_sender_id(1),
    ret_para(),
    direct_transfer(),
    gateway_mac(),
    next_session_id(1),
    udp_listenthread()
{
    dir_respara.sender_bind.id = -1;
    dir_respara.sender_bind.on_receive_packet_post_mtype = 1;
    memset(gateway_mac, 0xFF, sizeof(gateway_mac));
    InitializeCriticalSection(&send_bind_mutex);
    InitializeCriticalSection(&data_send_mutex);
    InitializeCriticalSection(&get_set_proto_param_mutex);
    g_logSystem.AppendText(
        "InitializeCriticalSection m_csSendBind=%p ,m_csDataSend=%p",
        &send_bind_mutex,
        &data_send_mutex
    );
    SetClassName("CDirTranThread");
}

CDirTranThread::~CDirTranThread()
{
    logFile_debug.AppendText("CDirTranThread::~CDirTranThread");

    if (udp_listenthread) {
        udp_listenthread->SetWorkingFalg(false);
        udp_listenthread->SafeExitThread(10000);
        udp_listenthread = nullptr;
    }

    logFile_debug.AppendText(
        "CDirTranThread::~CDirTranThread end CLnxThread::SafeExitThread(m_pThreadHandle)"
    );

    if (running_dir)
        StopRun();

    DeleteCriticalSection(&send_bind_mutex);
    DeleteCriticalSection(&data_send_mutex);
    DeleteCriticalSection(&get_set_proto_param_mutex);
}

bool CDirTranThread::DispathMessage(struct LNXMSG *msg)
{
    if (msg->mtype == ON_TRANSPACKET_MTYPE)
        OnTransPacket(msg->buflen, msg->buf);

    return false;
}

void CDirTranThread::ClearRetPara() const
{
    for (
        auto it = ret_para.begin();
        it != ret_para.end();
        it = ret_para.erase(it)
    ) {
        if (it->field_8) {
            SetEvent(it->field_8, false);
            CloseHandle(it->field_8);
        }

        delete it->field_0;
        it->field_0 = nullptr;
    }
}

void CDirTranThread::CloseAllGSNSender() const
{
    EnterCriticalSection(&data_send_mutex);

    for (
        auto it = data_send.begin();
        it != data_send.end();
        it = data_send.erase(it)
    ) {
        delete it->msg;
        it->msg = nullptr;

        if (it->ret)
            *it->ret = 0;

        if (it->eventret)
            SetEvent(it->eventret, false);
    }

    LeaveCriticalSection(&data_send_mutex);
    EnterCriticalSection(&send_bind_mutex);
    send_bind.clear();
    LeaveCriticalSection(&send_bind_mutex);
    ClearRetPara();
}

void CDirTranThread::CloseGSNReceiver(int id) const
{
    if (udp_listenthread)
        udp_listenthread->CloseGSNReceiver(id);
}

void CDirTranThread::CloseGSNSender(int id) const
{
    EnterCriticalSection(&data_send_mutex);

    for (auto it = data_send.begin(); it != data_send.end(); it++) {
        if (it->id != id)
            continue;

        delete it->msg;
        it->msg = nullptr;

        if (it->eventret)
            SetEvent(it->eventret, 0);

        if (it->ret)
            *it->ret = 0;

        it = data_send.erase(it) - 1;
    }

    LeaveCriticalSection(&data_send_mutex);
    EnterCriticalSection(&send_bind_mutex);

    for (auto it = send_bind.begin(); it != send_bind.end(); it++)
        if (it->id == id)
            it = send_bind.erase(it) - 1;

    LeaveCriticalSection(&send_bind_mutex);
}

bool CDirTranThread::DecryptPrivateData(
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

bool CDirTranThread::DirTranThreadInit() const
{
    bool ret = true;
    struct UdpListenParam *listen_param = new struct UdpListenParam;
    logFile_debug.AppendText("Enter DirTranThreadInit");
    listen_param->su_ipaddr = htonl(CtrlThread->GetNetOrderIPV4());
    listen_param->mainthread = thread_id;
    udp_listenthread = new CUDPListenThread(listen_param);

    if (!udp_listenthread->CreateThread(nullptr, false)) {
        logFile_debug.AppendText("CDirTranThread 创建UDPListenThread失败");
        ret = false;

    } else {
        if (udp_listenthread->StartThread()) {
            g_logSystem.AppendText(
                "CDirectTranSrv::DirTranThreadInit() pThreadHandle->StartThread failed"
            );
            return false;
        }

        logFile_debug.AppendText("WaitUDP_DirectThread_OK before");
    }

    if (!ret || !WaitUDP_DirectThread_OK(listen_param->event_udp_ready)) {
        ret = false;
        udp_listenthread->SafeExitThread(10000);
        udp_listenthread = nullptr;
    }

    CloseHandle(&listen_param->event_udp_ready);
    delete listen_param;
    listen_param = nullptr;
    logFile_debug.AppendText("Leaving DirTranThreadInit");
    return ret;
}

bool CDirTranThread::DoSendPacket(
    struct tagSenderBind &sender_bind,
    const struct tagDataSendUnit &send_unit
) const
{
    struct ether_addr special_mac_all_zero =
    { { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } };
    struct ether_addr special_mac_all_ff =
    { { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF } };
    struct tagDirTranPara trans_para = {};
    struct tagDirectCom_ProtocalParam proto_param = {
        0, 0, 0, 0, 3, 3000, {}, {}, true, 0, GetTickCount(), false, 1
    };
    struct tagDirPacketHead packet_head = {};
    struct [[gnu::packed]] {
        struct mtagFinalDirPacket packet_head;
        uint8_t data[1400];
    } final_packet = {};
    unsigned remain_len = 0;
    unsigned sent_times = 0;
    unsigned wait_ret = 0;
    bool is_multi_send = false;
    bool multi_send_started = false;
    bool multi_send_finished = false;

    if (!GetProtocalParam(proto_param, sender_bind.dstaddr, sender_bind.dstport)) {
        logFile_debug.AppendText("DoSendPacket>找不到相应的协议参数");
        return false;
    }

    sprintf(
        trans_para.field_32,
        "%d.%d.%d.%d",
        proto_param.dstaddr & 0xff,
        proto_param.dstaddr >> 8 & 0xff,
        proto_param.dstaddr >> 16 & 0xff,
        proto_param.dstaddr >> 24
    );
    sprintf(
        trans_para.dstaddr,
        "%d.%d.%d.%d",
        sender_bind.dstaddr & 0xff,
        sender_bind.dstaddr >> 8 & 0xff,
        sender_bind.dstaddr >> 16 & 0xff,
        sender_bind.dstaddr >> 24
    );
    trans_para.dstport = sender_bind.dstport;
    trans_para.field_50 = CtrlThread->field_1F0;
    sprintf(
        trans_para.srcaddr,
        "%d.%d.%d.%d",
        sender_bind.srcaddr & 0xff,
        sender_bind.srcaddr >> 8 & 0xff,
        sender_bind.srcaddr >> 16 & 0xff,
        sender_bind.srcaddr >> 24
    );
    trans_para.srcport = sender_bind.srcport;
    CtrlThread->GetAdapterMac(&trans_para.srcmacaddr);
    trans_para.field_4C = a2a.field_40;

    for (unsigned i = 0; i < 4; i++) {
        if (
            memcmp(gateway_mac, special_mac_all_zero, sizeof(gateway_mac)) &&
            memcmp(gateway_mac, special_mac_all_ff, sizeof(gateway_mac))
        ) {
            logFile_debug.AppendText(
                "发包前，获取网关MAC %02x:%02x:%02x:%02x:%02x:%02x",
                gateway_mac.ether_addr_octet[0],
                gateway_mac.ether_addr_octet[1],
                gateway_mac.ether_addr_octet[2],
                gateway_mac.ether_addr_octet[3],
                gateway_mac.ether_addr_octet[4],
                gateway_mac.ether_addr_octet[5]
            );

            if (i > 2)
                return false;

            break;
        }

        get_ip_mac(
            trans_para.field_4C ? sender_bind->dstaddr : proto_param.dstaddr,
            &gateway_mac
        );
        Sleep(1000);
    }

    trans_para.dstmacaddr = gateway_mac;
#define MAX_TRANSFERRED_DATALEN (MAX_MTU - sizeof(struct mtagFinalDirPacket))
    remain_len = send_unit.totallen;
    is_multi_send = remain_len > MAX_TRANSFERRED_DATALEN;

    while (remain_len) {
        memcpy(
            trans_para.data,
            &send_unit.msg[send_unit.totallen - remain_len],
            trans_para.mtu = std::min(MAX_TRANSFERRED_DATALEN, remain_len)
        );
        multi_send_finished = remain_len <= MAX_TRANSFERRED_DATALEN;
        remain_len -= trans_para.mtu;
        CopyDirTranPara(&dir_transpara, &trans_para);
        direct_transfer.InitPara(&trans_para);
        packet_head.reponse_code = DIRPACKET_REQUEST;
        packet_head.packet_len =
            htons(
                std::min(MAX_MTU, remain_len + sizeof(struct mtagFinalDirPacket))
            );
        packet_head.version = proto_param.version;
        packet_head.id = htonl(sender_bind.on_receive_packet_post_mtype++);
        packet_head.session_id = htonl(send_unit.session_id);

        if (proto_param.version == 1)
            CreateCurrentUTC(
                proto_param.utc_time,
                proto_param.timestamp,
                &packet_head.timestamp
            );

        else if (
            !(
                packet_head.timestamp =
                    udp_listenthread->GetNextTimeStampForSend(
                        proto_param.addr, proto_param.port
                    )
            )
        )
            logFile_debug.AppendText("Failed to generate time stamp for sending.");

        packet_head.timestamp = htonLONGLONG(packet_head.timestamp);
        packet_head.field_28 = true;

        if (!sent_times) {
            packet_head.slicetype =
                is_multi_send ?
                multi_send_finished ? DIRPACKET_MULTI_END :
                multi_send_started ? DIRPACKET_MULTI_MIDDLE : DIRPACKET_MULTI_BEGIN :
                DIRPACKET_SINGLE;
            multi_send_started = true;
        }

        packet_head.data_len = htonl(send_unit.totallen);
        CreateDirPktHead(
            final_packet.packet_head,
            packet_head,
            sender_bind,
            dir_transpara.data,
            trans_para.mtu,
            proto_param.keybuf,
            proto_param.ivbuf
        );
        memcpy(final_packet.data, dir_transpara.data, trans_para.mtu);
        logFile_debug.AppendText(
            "开始发送直通报文【nNext = %d】,【Total=%d】",
            remain_len,
            send_unit.totallen
        );

        if (!send_unit.need_reply) {
            if (!direct_transfer.Send(&final_packet, trans_para.mtu))
                return false;

        } else {
            logFile_debug.AppendText("需要响应报文");
            dir_respara.dir_packet_head = packet_head;
            dir_respara.event_ret = new WAIT_HANDLE;
            udp_listenthread->SetResSender(dir_respara);
            udp_listenthread->SetIfListenRes(true);
            logFile_debug.AppendText(
                "m_dirResPara.hEventRet addr:%u; Timeout:%u;try times:%d",
                dir_respara.event_ret,
                proto_param.timeout,
                proto_param.retry_count
            );
            direct_transfer.Send(&final_packet, trans_para.mtu);
            wait_ret = WaitForSingleObject(dir_respara.event_ret, proto_param.timeout);
            logFile_debug.AppendText("wait ret :%u", wait_ret);

            if (!wait_ret)
                logFile_debug.AppendText(
                    "事件返回成功，直通报文得到响应,发送次数【%d】",
                    sent_times = 0
                );

            else if (wait_ret == ETIMEDOUT) {
                logFile_debug.AppendText(
                    "网关MAC=%x:%x:%x:%x:%x:%x,",
                    trans_para.dstmacaddr.ether_addr_octet[0],
                    trans_para.dstmacaddr.ether_addr_octet[1],
                    trans_para.dstmacaddr.ether_addr_octet[2],
                    trans_para.dstmacaddr.ether_addr_octet[3],
                    trans_para.dstmacaddr.ether_addr_octet[4],
                    trans_para.dstmacaddr.ether_addr_octet[5]
                );
                logFile_debug.AppendText(
                    "等待响应报文超时，进入下一轮发送,发送次数【%d】",
                    sent_times++
                );
                sender_bind.on_receive_packet_post_mtype--;
                remain_len += trans_para.mtu;

            } else {
                udp_listenthread->SetIfListenRes(false);
                CloseHandle(dir_respara.event_ret);
                delete dir_respara.event_ret;
                logFile_debug.AppendText("发送直通报文发送【失败】，被强制停止");
                return false;
            }

            udp_listenthread->SetIfListenRes(false);
            CloseHandle(dir_respara.event_ret);
            delete dir_respara.event_ret;

            if (proto_param.retry_count == sent_times) {
                logFile_debug.AppendText(
                    "发送直通报文发送【失败】，发送次数【%d】",
                    proto_param.retry_count
                );
                return false;
            }
        }
    }

#undef MAX_TRANSFERRED_DATALEN
    logFile_debug.AppendText("发送直通报文【成功】");
    return true;
}

bool CDirTranThread::EncryptPrivateData(
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

int CDirTranThread::GSNReceiver(
    in_addr_t srcaddr,
    unsigned srcport,
    in_addr_t dstaddr,
    unsigned dstport,
    key_t pthread,
    int on_receive_packet_post_mtype
) const
{
    return udp_listenthread ?
           udp_listenthread->GSNReceiver(
               srcaddr,
               srcport,
               dstaddr,
               dstport,
               pthread,
               on_receive_packet_post_mtype
           ) :
           -1;
}

int CDirTranThread::GSNSender(
    in_addr_t srcaddr,
    unsigned srcport,
    in_addr_t dstaddr,
    unsigned dstport
) const
{
    EnterCriticalSection(&send_bind_mutex);
    send_bind.emplace_back(
        next_alloc_sender_id++
        srcaddr,
        srcport,
        dstaddr,
        dstport,
        1
    );
    LeaveCriticalSection(&send_bind_mutex);
    return send_bind.back().id;
}

bool CDirTranThread::GetProtocalParam(
    struct tagDirectCom_ProtocalParam &proto_param,
    in_addr_t addr,
    unsigned short port
) const
{
    EnterCriticalSection(&get_set_proto_param_mutex);

    for (const struct tagDirectCom_ProtocalParam &proto_param_l : proto_params) {
        if (proto_param_l.addr != addr || proto_param_l.port != port)
            continue;

        proto_param = proto_param_l;
        proto_param.addr = addr;
        LeaveCriticalSection(&get_set_proto_param_mutex);
        return true;
    }

    LeaveCriticalSection(&get_set_proto_param_mutex);
    return false;
}

bool CDirTranThread::GetProtocalParamFromSenderHand(
    struct tagDirectCom_ProtocalParam &proto_param,
    int id
) const
{
    EnterCriticalSection(&send_bind_mutex);

    for (const struct tagSenderBind &sender_bind : send_bind) {
        if (sender_bind.id != id)
            continue;

        LeaveCriticalSection(&send_bind_mutex);
        return GetProtocalParam(proto_param, sender_bind.dstaddr, sender_bind.dstport);
    }

    LeaveCriticalSection(&send_bind_mutex);
    logFile_debug.AppendText(
        "GetProtocalParamFromSenderHand>查询不到相应的发送绑定"
    );
    return false;
}

void CDirTranThread::OnTransPacket(
    [[maybe_unused]] unsigned long buflen,
    [[maybe_unused]] void *buf
) const
{
    struct tagDataSendUnit send_unit = {};
    struct tagSenderBind sender_bind = { -1, 0, 0, 0, 0, 1 };
    bool found_sender_bind = false;
    logFile_debug.AppendText(
        "CDirTranThread::OnTransPacket ENTER,m_bRunning_Dir=%d",
        running_dir
    );
    ClearRetPara();

    if (!running_dir) {
        CloseAllGSNSender();
        return;
    }

    while (true) {
        while (true) {
            EnterCriticalSection(&data_send_mutex);

            if (data_send.empty()) {
                logFile_debug.AppendText(
                    "%s m_vecDataSend.size()=%d",
                    "OnTransPacket",
                    0
                );
                LeaveCriticalSection(&data_send_mutex);
                logFile_debug.AppendText("CDirTranThread::OnTransPacket LEAVE");
                return;
            }

            send_unit = data_send.front();
            data_send.erase(data_send.begin());
            LeaveCriticalSection(&data_send_mutex);
            EnterCriticalSection(&send_bind_mutex);

            for (const struct tagSenderBind &sender_bind_l : send_bind) {
                if (sender_bind.id != send_unit.id)
                    continue;

                sender_bind.id = sender_bind_l.id;
                sender_bind.srcaddr = sender_bind_l.srcaddr;
                sender_bind.srcport = sender_bind_l.srcport;
                sender_bind.dstaddr = sender_bind_l.dstaddr;
                sender_bind.dstport = sender_bind_l.dstport;
                sender_bind.on_receive_packet_post_mtype =
                    sender_bind_l.on_receive_packet_post_mtype;
                found_sender_bind = true;
                break;
            }

            if (found_sender_bind)
                break;

            if (send_unit.ret) {
                *send_unit.ret = 0;
                SetEvent(send_unit.eventret, false);
            }

            logFile_debug.AppendText("找不到发送句柄，要释放数据包内存");
            delete[] send_unit.msg;
            send_unit.msg = nullptr;
            LeaveCriticalSection(&send_bind_mutex);
        }

        LeaveCriticalSection(&send_bind_mutex);
        logFile_debug.AppendText("开始发数据包并等待响应报文");

        if (DoSendPacket(sender_bind, send_unit)) {
            EnterCriticalSection(&send_bind_mutex);

            for (struct tagSenderBind &sender_bind_l : send_bind)
                if (sender_bind_l.id == send_unit.id)
                    sender_bind_l.on_receive_packet_post_mtype =
                        sender_bind.on_receive_packet_post_mtype;

            LeaveCriticalSection(&send_bind_mutex);
            logFile_debug.AppendText("整个直通报文发送返回成功");

            if (send_unit.ret) {
                logFile_debug.AppendText("开始进行激活返回值事件【成功】");
                *send_unit.ret = 1;
                SetEvent(send_unit.eventret, false);
            }

        } else {
            logFile_debug.AppendText("整个直通报文发送返回失败");

            if (send_unit.ret) {
                logFile_debug.AppendText("开始进行激活返回值事件【失败】");
                *send_unit.ret = 0;
                SetEvent(send_unit.eventret, false);
                logFile_debug.AppendText(
                    "temUnit.pdwRet = %d,temUnit.phEventRet=%x",
                    *send_unit.ret,
                    send_unit.eventret
                );
            }
        }

        delete[] send_unit.msg;
        send_unit.msg = nullptr;
        CloseGSNReceiver(sender_bind.id);

        if (!running_dir)
            return;
    }
}

bool CDirTranThread::PostPacketNoResponse(
    int id,
    unsigned char *buf,
    unsigned buflen
) const
{
    struct tagDirectCom_ProtocalParam proto_param = {
        0, 0, 0, 0, 3, 3000, {}, {}, true, 0, GetTickCount(), false, 1
    };
    struct tagDataSendUnit send_unit = {};
    unsigned char *msg = nullptr;

    if (!GetProtocalParamFromSenderHand(proto_param, id)) {
        logFile_debug.AppendText("sendMessage>找不到相应的协议参数");
        return false;
    }

    if (buflen & 7)
        buflen = buflen + 8 - buflen % 8;

    send_unit.id = id;
    send_unit.msg = new unsigned char[buflen];
    send_unit.eventret = nullptr;
    send_unit.ret = nullptr;
    send_unit.need_reply = false;
    send_unit.session_id = next_session_id++;
    send_unit.totallen = buflen;

    if (!send_unit.msg)
        return false;

    memcpy(send_unit.msg, buf, buflen);
    EncryptPrivateData(proto_param, send_unit.msg, buflen);
    EnterCriticalSection(&data_send_mutex);
    data_send.push_back(send_unit);
    LeaveCriticalSection(&data_send_mutex);
    logFile_debug.AppendText(
        "%s PostThreadMessage =%d",
        "PostPacketNoResponse",
        PostThreadMessage(ON_TRANSPACKET_MTYPE, 0, nullptr)
    );
    return true;
}

bool CDirTranThread::PostPacketSAMHeartbeatNoResponse(
    int id,
    unsigned char *buf,
    unsigned buflen
) const
{
    struct tagDirectCom_ProtocalParam proto_param = {
        0, 0, 0, 0, 3, 3000, {}, {}, true, 0, GetTickCount(), false, 1
    };
    struct tagDataSendUnit send_unit = {};
if(!GetProtocalParamFromSenderHand(proto_param,id))
{
        logFile_debug.AppendText("sendMessage>找不到相应的协议参数");
    return false;
}
send_unit.id=-1;
send_unit.totallen=0;
send_unit.msg=nullptr;
send_unit.eventret=nullptr;
send_unit.ret=nullptr;
send_unit.session_id=next_session_id++;
send_unit.need_reply=false;
// TODO
}

bool CDirTranThread::SendPacketNoResponse(
    int id,
    unsigned char *buf,
    unsigned buflen,
    unsigned timeout) const
{
}

void CDirTranThread::SetDirParaXieYi(
    const struct tagDirectCom_ProtocalParam &proto_param
) const
{
}

bool CDirTranThread::SetProtocalParam_TimeStamp(
    in_addr_t addr,
    unsigned short port,
    unsigned long utc_time,
    unsigned long timestamp
) const
{
}

bool CDirTranThread::SetReTranPara(
    in_addr_t addr,
    unsigned short port,
    unsigned retry_count,
    unsigned timeout
) const
{
}

void CDirTranThread::StartRun() const
{
}

void CDirTranThread::StopRun() const
{
}

bool CDirTranThread::WaitUDP_DirectThread_OK(WAIT_HANDLE &event_udp_ready) const
{
}

bool CDirTranThread::postMessage(
    int id,
    unsigned char *buf,
    unsigned buflen
) const
{
}

bool CDirTranThread::sendMessage(
    int id,
    unsigned char *buf,
    unsigned buflen
) const
{
}

bool CDirTranThread::sendMessageWithTimeout(
    int id,
    unsigned char *buf,
    unsigned buflen,
    unsigned timeout
) const
{
}
