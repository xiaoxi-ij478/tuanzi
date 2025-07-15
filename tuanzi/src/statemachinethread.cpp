#include "all.h"
#include "cmdutil.h"
#include "global.h"
#include "changelanguage.h"
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
    return state_data.state_var
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
    char *buf
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
                    eapol_frame->eap_type_non_md5_data = new char[buflen],
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
        rj_printf_debug("strFailReason=%s\n", eapol_frame.fail_reason);

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

DEFINE_DISPATH_MESSAGE_HANDLER(OnPacketNotify, CStateMachineThread) const
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
    CtrlThread->has_auth_success = 0;

    if (state_visual && state_visual->state_data) {
        state_visual->state_data->state = 0;
        state_visual->state_data->prev_state = 0;
    }

    CtrlThread->connecting = 0;
    CtrlThread->field_53A = 0;
    state_data.logoff = 0;
    state_visual = CreateState(STATE_DISCONNECTED);
    state_data.hold_count = 0;
    state_data.logoff_init = 0;
    state_data.disconnected = 1;
    state_data.state = 0;
    state_data.prev_state = 0;
    OnStateMove(-1, 0);
}

DEFINE_DISPATH_MESSAGE_HANDLER(OnStateMove, CStateMachineThread) const
{
}

DEFINE_DISPATH_MESSAGE_HANDLER(OnTimer, CStateMachineThread) const
{
}

void CStateMachineThread::SendNAKFrame() const
{
}

void CStateMachineThread::SendNotificationFrame() const
{
}

int CStateMachineThread::parseFrame(struct EAPOLFrame *eapol_frame) const
{
}

void CStateMachineThread::txLogOff(char a1) const
{
}

void CStateMachineThread::txRspAuth() const
{
}

void CStateMachineThread::txRspAuthPAP() const
{
}

void CStateMachineThread::txRspID() const
{
}

void CStateMachineThread::txSetLogOff_CompThird() const
{
}

void CStateMachineThread::txStart() const
{
}
