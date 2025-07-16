#include "all.h"
#include "cmdutil.h"
#include "global.h"
#include "changelanguage.h"
#include "eapolutil.h"
#include "msgutil.h"
#include "mtypes.h"
#include "encodeutil.h"
#include "md5checksum.h"
#include "statemachinethread.h"

CStateMachineThread::CStateMachineThread() :
    field_1D0(1),
    field_1D1(),
    state_visual(),
    state_data()
{
    SetClassName("CStateMachineThread");
}

void CStateMachineThread::DispathMessage(struct LNXMSG *msg)
{
    rj_printf_debug("CStateMachineThread getmsg id=%d\n", msg->mtype);

    switch (msg->mtype) {
            HANDLE_MTYPE(ON_TIMER_MTYPE, OnTimer);
            HANDLE_MTYPE(PACKET_NOTIFY_MTYPE, OnPacketNotify);
            HANDLE_MTYPE(SAY_HELLO_MTYPE, OnSayHello);
            HANDLE_MTYPE(START_STATE_MACHINE_MTYPE, OnStartMachine);
            HANDLE_MTYPE(SEND_LOGOFF_MTYPE, OnSendLogoff);
            HANDLE_MTYPE(STATE_MOVE_MTYPE, OnStateMove);
    }
}

void CStateMachineThread::OnTimer(int tflag) const
{
    if (OnTimerEnter(tflag)) {
        if (!PostThreadMessage(ON_TIMER_MTYPE, tflag, -1))
            OnTimerLeave(tflag);

    } else
        g_logSystem.AppendText(
            "CStateMachineThread::OnTimer(timerFlag=%d),return",
            tflag
        );
}

CStateVisual *CStateMachineThread::CreateState(enum STATES state)
{
    assert(state != STATE_INVALID);

    switch (state) {
#define RETURN_STATE(state_enum, state_var) \
case (state_enum): \
    state_data.state_var.SetStateData(&state_data); \
    state_data.state_var.thread_id = pthread_self(); \
    return &state_data.state_var
            RETURN_STATE(STATE_DISCONNECTED, state_disconnected);
            RETURN_STATE(STATE_CONNECTING, state_connecting);
            RETURN_STATE(STATE_ACQUIRED, state_acquired);
            RETURN_STATE(STATE_AUTHENTICATING, state_authenticating);
            RETURN_STATE(STATE_AUTHENTICATED, state_authenticated);
            RETURN_STATE(STATE_HOLD, state_hold);
            RETURN_STATE(STATE_LOGOFF, state_logoff);
#undef RETURN_STATE
    }
}

// *INDENT-OFF*
struct EAPOLFrame *CStateMachineThread::EncapsulateFrame(
    enum IEEE8021X_PACKET_TYPE packet_type,
    enum EAP_TYPES eap_type,
    // *INDENT-ON*
    unsigned short buflen,
    const char *buf
) const
{
    struct EAPOLFrame *eapol_frame = nullptr;

    if (
        packet_type == IEEE8021X_EAPOL_KEY ||
        packet_type == IEEE8021X_EAPOL_ENCAPSULATED_ASF_ALERT ||
        (
            packet_type == IEEE8021X_EAP_PACKET &&
            eap_type != EAP_TYPE_MD5 &&
            eap_type != EAP_TYPE_IDENTITY &&
            eap_type != EAP_TYPE_NAK &&
            eap_type != EAP_TYPE_NOTIFICATION &&
            eap_type != EAP_TYPE_RUIJIE_PRIVATE
        )
    )
        return nullptr;

    eapol_frame = new struct EAPOLFrame;
    InitEAPOLFrame(eapol_frame);

    if (CtrlThread->IsRuijieNas())
        eapol_frame->dstaddr = CtrlThread->dstaddr;

    else {
        eapol_frame->dstaddr.ether_addr_octet[0] = 0x01;
        eapol_frame->dstaddr.ether_addr_octet[1] = 0x80;
        eapol_frame->dstaddr.ether_addr_octet[2] = 0xC2;
        eapol_frame->dstaddr.ether_addr_octet[3] = 0x00;
        eapol_frame->dstaddr.ether_addr_octet[4] = 0x00;
        eapol_frame->dstaddr.ether_addr_octet[5] = 0x03;
    }

    CtrlThread->GetAdapterMac(&eapol_frame->srcaddr);
    eapol_frame->ether_type = ETH_P_PAE;
    eapol_frame->ieee8021x_version = 1;
    eapol_frame->ieee8021x_packet_type = packet_type;

    if (
        packet_type == IEEE8021X_EAPOL_START ||
        packet_type == IEEE8021X_EAPOL_LOGOFF
    )
        eapol_frame->ieee8021x_packet_length = 0;

    CtrlThread->GetAdapterMac(&eapol_frame->field_C);

    if (packet_type != IEEE8021X_EAP_PACKET)
        return eapol_frame;

    eapol_frame->eap_code = EAP_RESPONSE;
    eapol_frame->ieee8021x_packet_length = 5;
    eapol_frame->eap_type = eap_type;
    eapol_frame->eap_id = state_visual->state_data->recv_id;

    switch (eap_type) {
        case EAP_TYPE_INVALID:
        case EAP_TYPE_OTP:
        case EAP_TYPE_GTC:
        case EAP_TYPE_EXPANDED:
            return eapol_frame;

        case EAP_TYPE_MD5:
            if (!buflen)
                return nullptr;

            eapol_frame->eap_type_md5_length =
                buflen - CtrlThread->configure_info.last_auth_username.length();
            eapol_frame->eap_length = buflen + 6;
            eapol_frame->ieee8021x_packet_length = buflen + 6;
            memcpy(eapol_frame->eap_type_md5_data = new char[buflen], buf, buflen);
            break;

        case EAP_TYPE_IDENTITY:
        case EAP_TYPE_NOTIFICATION:
        case EAP_TYPE_NAK:
        case EAP_TYPE_RUIJIE_PRIVATE:
            eapol_frame->eap_length = buflen + 5;
            eapol_frame->ieee8021x_packet_length = buflen + 5;

            if (buflen)
                memcpy(
                    eapol_frame->eap_type_notif_data = new char[buflen],
                    buf,
                    buflen
                );

            break;
    }
}

void CStateMachineThread::FailNotification(struct EAPOLFrame *eapol_frame)
{
    std::string fail_reason;

    if (eapol_frame)
        rj_printf_debug("strFailReason=%s\n", eapol_frame->fail_reason);

    g_log_Wireless.WriteString("进入CStateMachineThread::FailNotification函数");

    if (!CtrlThread->IsRuijieNas()) {
        g_log_Wireless.AppendText(
            "是第三方兼容, 拷贝错误原因：%s",
            CtrlThread->private_properties.fail_reason.c_str()
        );
        fail_reason = CtrlThread->private_properties.fail_reason;
    }

    if (
        CtrlThread->service_list_updated &&
        std::find(
            CtrlThread->private_properties.services.cbegin(),
            CtrlThread->private_properties.services.cend(),
            CtrlThread->configure_info.public_service
        ) != CtrlThread->private_properties.services.cend()
    )
        fail_reason = CChangeLanguage::Instance().LoadString(202);

    if (!fail_reason.empty() || CtrlThread->has_auth_success) {
        g_log_Wireless.AppendText(
            "strFailReason != _T()=%s || CtrlThread->m_bHasAuthSuccess",
            fail_reason.c_str()
        );
        state_visual->state_data->hold_count = 33;
    }

    CtrlThread->logoff_message =
        eapol_frame && eapol_frame->fail_reason_magic == 0x1311 ?
        eapol_frame->fail_reason :
        fail_reason;
}

void CStateMachineThread::InitState() const
{}

DEFINE_DISPATH_MESSAGE_HANDLER(OnPacketNotify, CStateMachineThread)
{
    if (!state_visual)
        return;

    if (state_visual->state_data->state == STATE_LOGOFF)
        DeleteFrameMemory(reinterpret_cast<struct EAPOLFrame *>(arg2));

    else if (parseFrame(reinterpret_cast<struct EAPOLFrame *>(arg2)) == -1)
        rj_printf_debug("不合法的1x数据包，不进行处理!\n");

    else
        OnStateMove(-1, 0);
}

DEFINE_DISPATH_MESSAGE_HANDLER(OnSayHello, CStateMachineThread) const
{
    if (CtrlThread->send_packet_thread)
        CtrlThread->send_packet_thread.PostThreadMessage(
            SEND_MESSAGE_MTYPE,
            arg1,
            arg2
        );

    else if (arg2)
        delete[] reinterpret_cast<char *>(arg2);
}

DEFINE_DISPATH_MESSAGE_HANDLER(OnSendLogoff, CStateMachineThread) const
{
    txLogOff(arg2);
}

DEFINE_DISPATH_MESSAGE_HANDLER(OnStartMachine, CStateMachineThread)
{
    CtrlThread->has_auth_success = false;

    if (state_visual && state_visual->state_data)
        state_visual->state_data->state =
            state_visual->state_data->prev_state = STATE_INVALID;

    CtrlThread->connecting = false;
    CtrlThread->field_53A = false;
    state_visual = CreateState(STATE_DISCONNECTED);
    state_data.logoff = false;
    state_data.hold_count = 0;
    state_data.logoff_init = false;
    state_data.disconnected = true;
    state_data.state = state_data.prev_state = STATE_INVALID;
    OnStateMove(-1, 0);
}

DEFINE_DISPATH_MESSAGE_HANDLER(OnStateMove, CStateMachineThread)
{
    enum STATES new_state = static_cast<enum STATES>(arg1);

    if (state_visual->state_data->state == STATE_LOGOFF)
        return;

    if (state_visual->state_data->IsAuthenticated()) {
        new_state = STATE_AUTHENTICATED;
        CtrlThread->connecting = CtrlThread->field_53A = false;

    } else if (state_visual->state_data->IsDisconnected())
        new_state = STATE_DISCONNECTED;

    else if (state_visual->state_data->IsHeld())
        new_state = STATE_HOLD;

    else if (state_visual->state_data->IsLogOff())
        new_state = STATE_LOGOFF;

    else if ((new_state = static_cast<enum STATES>(arg1)) == -1) {
        g_log_Wireless.AppendText(
            "CStateMachineThread::OnStateMove pi=%d RETURN",
            -1
        );
        state_visual->MoveState();
        return;
    }

    g_log_Wireless.AppendText(" before m_state = CreateState(p1)=%d", arg1);
    state_visual = CreateState(new_state);
    state_visual->Initlize();

    if (
        CtrlThread->IsRuijieNas() ||
        state_visual->state_data->prev_state != STATE_AUTHENTICATED
    )
        CtrlThread->PostThreadMessage(HANDSHAKE_TO_SAM_MTYPE, arg1);

    state_visual->MoveState();

    if (CtrlThread->connecting)
        state_visual->state_data->state = STATE_LOGOFF;
}

DEFINE_DISPATH_MESSAGE_HANDLER(OnTimer, CStateMachineThread)
{
    switch (arg1) {
        case AUTH_WHILE_MTYPE:
            state_visual->state_data->SetAuthWhile();

            if (!KillTimer(state_visual->state_data->auth_timer))
                rj_printf_debug("%d定时器未被杀死\n", arg1);

            state_visual->state_data->auth_timer = 0;
            break;

        case START_WHILE_MTYPE:
            g_log_Wireless.AppendText("CStateMachineThread::OnTimer TIMEID_STATWHEN");
            state_visual->state_data->SetStartWhen();

            if (!KillTimer(state_visual->state_data->connect_timer))
                rj_printf_debug("%d定时器未被杀死\n", arg1);

            state_visual->state_data->connect_timer = 0;
            break;

        case HOLD_WHILE_MTYPE:
            state_visual->state_data->SetHeldWhile();

            if (!KillTimer(state_visual->state_data->hold_timer))
                rj_printf_debug("%d定时器未被杀死\n", arg1);

            state_visual->state_data->hold_timer = 0;
            break;

        default:
            g_logSystem.AppendText("unknow timer flag = %d", arg1);
            OnTimerLeave(arg1);
            return;
    }

    OnStateMove(-1, 0);
    OnTimerLeave(arg1);
}

void CStateMachineThread::SendNAKFrame() const
{
    struct EAPOLFrame *eapol_frame = nullptr;
    struct eapolpkg *eapol_pkg = nullptr;
    unsigned length = 0;
    eapol_frame = EncapsulateFrame(IEEE8021X_EAP_PACKET, EAP_TYPE_NAK, 0, nullptr);

    if (!eapol_frame)
        return;

    eapol_pkg = CreateEapolPacket(eapol_frame, &length);
    DeleteFrameMemory(eapol_frame);
    eapol_frame = nullptr;

    if (CtrlThread->send_packet_thread)
        CtrlThread->send_packet_thread->PostThreadMessage(
            SEND_MESSAGE_MTYPE,
            length,
            reinterpret_cast<unsigned long>(eapol_pkg)
        );
}

void CStateMachineThread::SendNotificationFrame() const
{
    struct EAPOLFrame *eapol_frame = nullptr;
    struct eapolpkg *eapol_pkg = nullptr;
    unsigned length = 0;
    eapol_frame = EncapsulateFrame(IEEE8021X_EAP_PACKET, EAP_TYPE_NAK, 0, nullptr);

    if (!eapol_frame)
        return;

    eapol_pkg = CreateEapolPacket(eapol_frame, &length);
    DeleteFrameMemory(eapol_frame);
    eapol_frame = nullptr;

    if (CtrlThread->send_packet_thread)
        CtrlThread->send_packet_thread->PostThreadMessage(
            SEND_MESSAGE_MTYPE,
            length,
            reinterpret_cast<unsigned long>(eapol_pkg)
        );
}

int CStateMachineThread::parseFrame(struct EAPOLFrame *eapol_frame)
{
    int ret = 0;
    rj_printf_debug("状态机模块，收到一个报文了。");

    if (
        ntohs(eapol_frame->ether_type) == ETH_P_PAE &&
        eapol_frame->ieee8021x_packet_type == IEEE8021X_EAP_PACKET
    ) {
        DeleteFrameMemory(eapol_frame);
        return -1;
    }

    g_log_Wireless.AppendText(
        "CStateMachineThread::parseFrame EAPCODE=%d,REQTYPE(pFrame)=%d____",
        eapol_frame->eap_code,
        eapol_frame->eap_type
    );

    switch (eapol_frame->eap_code) {
        case EAP_SUCCESS:
            CtrlThread->has_auth_success = 1;
            state_visual->state_data->eap_success = true;

            if (CtrlThread->GetDHCPAuthStep() != 1)
                CtrlThread->SaveRadiusPrivate(eapol_frame);

            ret = 1;
            break;

        case EAP_FAILURE:
            state_visual->state_data->eap_failure = true;
            FailNotification(eapol_frame);
            CtrlThread->PostThreadMessage(EAP_FAILURE_MTYPE, 0, 0);
            ret = 0;
            break;

        case EAP_REQUEST:
            state_visual->state_data->recv_id = eapol_frame->eap_id;
            CtrlThread->dstaddr = eapol_frame->srcaddr;

            if (!CtrlThread->IsRuijieNas()) {
                CtrlThread->dstaddr.ether_addr_octet[0] = 0x01;
                CtrlThread->dstaddr.ether_addr_octet[1] = 0x80;
                CtrlThread->dstaddr.ether_addr_octet[2] = 0xC2;
                CtrlThread->dstaddr.ether_addr_octet[3] = 0x00;
                CtrlThread->dstaddr.ether_addr_octet[4] = 0x00;
                CtrlThread->dstaddr.ether_addr_octet[5] = 0x03;
            }

            switch (eapol_frame->eap_type) {
                case EAP_TYPE_IDENTITY:
                    ret = 1;
                    state_visual->state_data->req_id = true;
                    break;

                case EAP_TYPE_NOTIFICATION:
                    if (eapol_frame->eap_length > 4) {
                        g_uilog.AppendText(
                            "CStateMachineThread::parseFrame RSP_TYPE_NOTIFICATION=%s",
                            eapol_frame->eap_type_notif_data
                        );
                        ShowLocalMsg(eapol_frame->eap_type_notif_data, "");
                    }

                    ret = 1;
                    SendNotificationFrame();
                    break;

                case EAP_TYPE_NAK:
                case EAP_TYPE_OTP:
                case EAP_TYPE_GTC:
                    ret = 1;
                    SendNAKFrame();
                    break;

                case EAP_TYPE_MD5:
                    ret = -1;

                    if (!eapol_frame->eap_type_md5_length)
                        break;

                    ret = 1;
                    state_visual->state_data->req_auth = true;
                    state_visual->state_data->eap_md5_datalen =
                        eapol_frame->eap_type_md5_length;
                    memset(
                        state_visual->state_data->eap_md5_data,
                        0,
                        sizeof(state_visual->state_data->eap_md5_data)
                    );
                    memcpy(
                        state_visual->state_data->eap_md5_data,
                        eapol_frame->eap_type_md5_data,
                        state_visual->state_data->eap_md5_datalen
                    );
                    break;

                case EAP_TYPE_RUIJIE_PRIVATE:
                    ret = 1;
                    state_visual->state_data->req_auth = true;
                    state_visual->state_data->eap_md5_datalen = 0;
                    break;

                default:
                    ret = -1;
                    break;
            }

            break;

        default:
            ret = -1;
            break;
    }

    DeleteFrameMemory(eapol_frame);
    return ret;
}

void CStateMachineThread::txLogOff(char a1) const
{
    struct EAPOLFrame *eapol_frame = nullptr;
    struct eapolpkg *eapol_pkg = nullptr;
    unsigned length = 0;
    g_log_Wireless.AppendText("CStateMachineThread::txLogOff");

    if (!state_visual || !state_visual->state_data)
        return;

    state_visual->state_data->state = STATE_LOGOFF;
    eapol_frame =
        EncapsulateFrame(
            IEEE8021X_EAPOL_LOGOFF,
            EAP_TYPE_INVALID,
            0,
            nullptr
        );

    if (!eapol_frame)
        return;

    eapol_frame->field_6C0 = a1;
    eapol_frame->dhcp_ipinfo = CtrlThread->configure_info.dhcp_ipinfo;
    eapol_pkg = CreateEapolPacket(eapol_frame, &length);
    DeleteFrameMemory(eapol_frame);

    if (CtrlThread->send_packet_thread)
        send_packet_thread->PostThreadMessage(
            SEND_MESSAGE_MTYPE,
            length,
            eapol_pkg
        );

    rj_printf_debug("Send : LogOff\r\n");
}

void CStateMachineThread::txRspAuth() const
{
    struct EAPOLFrame *eapol_frame = nullptr;
    struct eapolpkg *eapol_pkg = nullptr;
    char username_buf[512] = {};
    char *checksum_buf = nullptr;
    char *md5_buf = nullptr;
    char *final_buf = nullptr;
    unsigned username_buflen = 0;
    unsigned checksum_buflen = 0;
    unsigned final_buflen = 0;
    unsigned eapol_pkglen = 0;
    ConvertUtf8ToGBK(
        username_buf,
        sizeof(username_buf),
        CtrlThread->configure_info.last_auth_password.c_str(),
        CtrlThread->configure_info.last_auth_password.length()
    );
    username_buflen = strlen(username_buf);
    checksum_buflen =
        username_buflen + state_visual->state_data->eap_md5_datalen + 1;
    checksum_buf = new char[checksum_buflen];
    checksum_buf[0] = state_visual->state_data->recv_id;
    memcpy(&checksum_buf[1], username_buf, username_buflen);
    memcpy(
        &checksum_buf[1 + username_buflen],
        state_visual->state_data->eap_md5_data,
        state_visual->state_data->eap_md5_datalen
    );
    md5_buf = CMD5Checksum::GetMD5(checksum_buf, checksum_buflen);
    delete[] checksum_buf;
    checksum_buf = nullptr;
    final_buflen =
        (strlen(checksum_buf) >> 1) +
        CtrlThread->configure_info.last_auth_username.length();
    final_buf = new char[final_buflen];
    MD5StrtoUChar(md5_buf, final_buf);
    delete[] md5_buf;
    md5_buf = nullptr;
    strcpy(
        &final_buf[strlen(checksum_buf) >> 1],
        CtrlThread->configure_info.last_auth_username.c_str()
    );
    eapol_frame =
        EncapsulateFrame(
            IEEE8021X_EAP_PACKET,
            EAP_TYPE_MD5,
            final_buflen,
            final_buf
        );
    delete[] final_buf;
    final_buf = nullptr;

    if (!eapol_frame)
        return;

    eapol_frame->dhcp_ipinfo = CtrlThread->configure_info.dhcp_ipinfo;
    eapol_pkg = CreateEapolPacket(eapol_frame, &eapol_pkglen);
    DeleteFrameMemory(eapol_frame);

    if (CtrlThread->send_packet_thread)
        CtrlThread->send_packet_thread->PostThreadMessage(
            SEND_MESSAGE_MTYPE,
            eapol_pkglen,
            eapol_pkg
        );
}

void CStateMachineThread::txRspAuthPAP() const
{
    struct EAPOLFrame *eapol_frame = nullptr;
    struct eapolpkg *eapol_pkg = nullptr;
    char *buf = nullptr;
    unsigned eapol_pkglen = 0;
    unsigned buflen =
        CtrlThread->configure_info.last_auth_password.length() +
        CtrlThread->configure_info.last_auth_username.length() +
        1;
    buf = new char[buflen];
    buf[0] = CtrlThread->configure_info.last_auth_password.length();
    strcpy(&buf[1], CtrlThread->configure_info.last_auth_password.c_str());
    strcpy(
        &buf[1 + CtrlThread->configure_info.last_auth_password.length()],
        CtrlThread->configure_info.last_auth_username.c_str()
    );
    eapol_frame =
        EncapsulateFrame(
            IEEE8021X_EAP_PACKET,
            EAP_TYPE_RUIJIE_PRIVATE,
            buflen,
            buf
        );
    delete[] buf;
    buf = nullptr;

    if (!eapol_frame)
        return;

    eapol_frame->dhcp_ipinfo = CtrlThread->configure_info.dhcp_ipinfo;
    eapol_pkg = CreateEapolPacket(eapol_frame, &eapol_pkglen);
    DeleteFrameMemory(eapol_frame);

    if (CtrlThread->send_packet_thread)
        CtrlThread->send_packet_thread->PostThreadMessage(
            SEND_MESSAGE_MTYPE,
            eapol_pkglen,
            eapol_pkg
        );
}

void CStateMachineThread::txRspID() const
{
    struct EAPOLFrame *eapol_frame = nullptr;
    struct eapolpkg *eapol_pkg = nullptr;
    unsigned length = 0;
    eapol_frame =
        EncapsulateFrame(
            this,
            IEEE8021X_EAP_PACKET,
            EAP_TYPE_IDENTITY,
            CtrlThread->configure_info.last_auth_username.length(),
            CtrlThread->configure_info.last_auth_username.c_str()
        );
    eapol_frame->dhcp_ipinfo = CtrlThread->configure_info.dhcp_ipinfo;
    eapol_pkg = CreateEapolPacket(eapol_frame, &length);
    DeleteFrameMemory(eapol_frame);

    if (CtrlThread->send_packet_thread)
        send_packet_thread->PostThreadMessage(
            SEND_MESSAGE_MTYPE,
            length,
            eapol_pkg
        );

    rj_printf_debug("Send : RspID\r\n");
}

void CStateMachineThread::txSetLogOff_CompThird()
{
    if (state_visual && state_visual->state_data)
        state_visual->state_data->state = STATE_LOGOFF;
}

void CStateMachineThread::txStart() const
{
    struct EAPOLFrame *eapol_frame = nullptr;
    struct eapolpkg *eapol_pkg = nullptr;
    unsigned length = 0;
    eapol_frame =
        EncapsulateFrame(
            IEEE8021X_EAPOL_START,
            EAP_TYPE_INVALID,
            0,
            nullptr
        );

    if (!eapol_frame)
        return;

    g_log_Wireless.AppendText(
        "CStateMachineThread::txStart start addr %02x:%02x:%02x:%02x:%02x:%02x",
        CtrlThread->start_dst_addr.ether_addr_octet[0],
        CtrlThread->start_dst_addr.ether_addr_octet[1],
        CtrlThread->start_dst_addr.ether_addr_octet[2],
        CtrlThread->start_dst_addr.ether_addr_octet[3],
        CtrlThread->start_dst_addr.ether_addr_octet[4],
        CtrlThread->start_dst_addr.ether_addr_octet[5]
    );
    eapol_frame->dstaddr = CtrlThread->start_dst_addr;
    eapol_frame->dhcp_ipinfo = CtrlThread->configure_info.dhcp_ipinfo;
    eapol_pkg = CreateEapolPacket(eapol_frame, &length);
    DeleteFrameMemory(eapol_frame);

    if (CtrlThread->send_packet_thread)
        send_packet_thread->PostThreadMessage(
            SEND_MESSAGE_MTYPE,
            length,
            eapol_pkg
        );

    rj_printf_debug("Send : Start\r\n");
}
