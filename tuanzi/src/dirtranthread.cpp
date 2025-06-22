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
    set_proto_param_mutex(),
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
    InitializeCriticalSection(&set_proto_param_mutex);
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
    DeleteCriticalSection(&set_proto_param_mutex);
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
    unsigned i = 0;

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

    for (i = 0; i < 4; i++) {
        if (
            memcmp(gateway_mac, special_mac_all_zero, sizeof(gateway_mac)) &&
            memcmp(gateway_mac, special_mac_all_ff, sizeof(gateway_mac))
        )
            break;

        get_ip_mac(
            trans_para.field_4C ? sender_bind->dstaddr : proto_param.dstaddr,
            &gateway_mac
        );
        Sleep(1000);
    }
      logFile_debug.AppendText(
    "发包前，获取网关MAC %02x:%02x:%02x:%02x:%02x:%02x",
    gateway_mac.ether_addr_octet[0],
    gateway_mac.ether_addr_octet[1],
    gateway_mac.ether_addr_octet[2],
    gateway_mac.ether_addr_octet[3],
    gateway_mac.ether_addr_octet[4],
    gateway_mac.ether_addr_octet[5]
    );
  if ( i > 2 )
    return false;
    trans_para.dstmacaddr=gateway_mac;
    if(send_unit.totallen<=0)
    {
            logFile_debug.AppendText("发送直通报文【成功】");
    return true;
    }

}

bool CDirTranThread::EncryptPrivateData(
    const struct tagDirectCom_ProtocalParam &proto_param,
    unsigned char *buf,
    unsigned buflen
)
const
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
}

int CDirTranThread::GSNSender(
    in_addr_t srcaddr,
    unsigned srcport,
    in_addr_t dstaddr,
    unsigned dstport
) const
{
}

bool CDirTranThread::GetProtocalParam(
    struct tagDirectCom_ProtocalParam &proto_param,
    in_addr_t addr,
    unsigned short port
) const
{
}

bool CDirTranThread::GetProtocalParamFromSenderHand(
    struct tagDirectCom_ProtocalParam &proto_param,
    int id
) const
{
}

void CDirTranThread::OnTransPacket(
    [[maybe_unused]] unsigned long buflen,
    [[maybe_unused]] void *buf
) const
{
}

bool CDirTranThread::PostPacketNoResponse(
    int id,
    unsigned char *buf,
    unsigned buflen
) const
{
}

bool CDirTranThread::PostPacketSAMHeartbeatNoResponse(
    int id,
    unsigned char *buf,
    unsigned buflen
) const
{
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
