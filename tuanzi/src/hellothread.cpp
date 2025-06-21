#include "all.h"
#include "global.h"
#include "util.h"
#include "sendpacketthread.h" // SEND_MESSAGE_MTYPE
#include "hellothread.h"

CHelloThread::CHelloThread() :
    field_1D0(-1),
    hello_timer_interval(),
    hello_para(),
    hello_timerid(),
    field_1E8(),
    hello_id_offset(),
    field_1F0()
{
    SetClassName("CHelloThread");
}

bool CHelloThread::DispathMessage(struct LNXMSG *msg)
{
    switch (msg->mtype) {
        case ON_TIMER_MTYPE:
            OnHelloTimer(msg->buflen, msg->buf);
            break;

        case SET_HELLOTIMER_PLEASE_MTYPE:
            OnSetHelloTimerPlease(msg->buflen, msg->buf);
            break;

        case CHANGE_HELLOPARA_MTYPE:
            OnChangeHelloPara(msg->buflen, msg->buf);
            break;
    }

    return true;
}

bool CHelloThread::InitInstance()
{
    field_1F0 = rand() % 0x1001 + 0x1001;
    memset(e_pHelloID, 0x74, sizeof(e_pHelloID));
    return CLnxThread::InitInstance();
}

void CHelloThread::OnTimer(int tflag) const
{
    if (OnTimerEnter(tflag)) {
        if (
            tflag == 0x70 &&
            !PostThreadMessage(ON_TIMER_MTYPE, 0x70, reinterpret_cast<void *>(-1))
        )
            OnTimerLeave(0x70);

    } else
        g_logSystem.AppendText(
            "CHelloThread::OnTimer(timerFlag=%d),return", tflag
        );
}

struct HelloPacket *CHelloThread::CreateHelloPacket(unsigned &packet_len)
{
    struct HelloPacket *ret = new struct HelloPacket;
    unsigned cur_crcid = 0;

    if (!ret) {
        packet_len = 0;
        return nullptr;
    }

    CtrlThread->GetAdapterMac(
        reinterpret_cast<struct ether_addr *>(&ret->ether_header.ether_shost)
    );
    *reinterpret_cast<struct ether_addr *>(&ret->ether_header.ether_dhost) =
        CtrlThread->field_550;
    ret->ether_header.ether_type = htons(ETH_P_PAE);
    ret->code = 1;
    ret->id = 0xBF;
    ret->length =
        htons(
            sizeof(struct HelloPacket) - offsetof(struct HelloPacket, id)
        );
    ret->field_14.field_0 = 0x6011113;
    ret->field_1E.field_0 = 0x6011113;
    ret->field_28 = 0x3011113;
    ret->field_14.field_4 = htonl(hello_para + (cur_crcid = GetCurCRCID()));
    ret->field_1E.field_4 = htonl(cur_crcid + 0x74 - e_pHelloID[hello_id_offset++]);
    hello_id_offset %= 16;
    encode(
        &ret->type,
        sizeof(struct HelloPacket) - offsetof(struct HelloPacket, type)
    );
    packet_len = sizeof(struct HelloPacket);
    return ret;
}

void CHelloThread::CreateHelloTimer(unsigned interval)
{
    memset(e_pHelloID, 0x74, sizeof(e_pHelloID));

    if (hello_timerid) {
        if (interval) {
            hello_timer_interval = interval;
            KillTimer(hello_timerid);
        }

    } else
        hello_timer_interval = interval;

    hello_timerid = SetTimer(nullptr, 0x70, hello_timer_interval, nullptr);
    OnHelloTimer(0x70, reinterpret_cast<void *>(-1));
}

unsigned CHelloThread::GetCurCRCID()
{
    unsigned v1 = e_pHelloID[hello_id_offset];

    if (v1 == 0x74)
        return ++field_1F0;

    for (unsigned i = 0; i < 16; i++)
        if (e_pHelloID[i] == 0x74)
            e_pHelloID[i] = v1;

    return ++field_1F0;
}

void CHelloThread::OnChangeHelloPara(unsigned long buflen, void *buf)
{
    if (buf)
        hello_para = reinterpret_cast<unsigned long>(buf);

    CreateHelloTimer(buflen);
}

void CHelloThread::OnHelloTimer(
    unsigned long buflen,
    [[maybe_unused]] void *buf
)
{
    unsigned rbuflen = 0;
    struct HelloPacket *hello_packet = CreateHelloPacket(rbuflen);

    if (!hello_packet) {
        OnTimerLeave(buflen);
        return;
    }

    if (CtrlThread->field_210 && CtrlThread->send_packet_thread) {
        if (
            !CtrlThread->send_packet_thread->PostThreadMessage(
                SEND_MESSAGE_MTYPE, rbuflen, hello_packet
            )
        )
            g_log_Wireless.AppendText(
                "CHelloThread::OnHelloTimer Post message error"
            );

        g_log_Wireless.AppendText("CHelloThread::OnHelloTimer POST HELLO");

    } else {
        logFile.AppendText(
            "CtrlThread->m_machineThread->m_sendPacketThread!=NULL"
        );
        delete hello_packet;
    }

    OnTimerLeave(buflen);
}

void CHelloThread::OnSetHelloTimerPlease(unsigned long, void *)
{
    CreateHelloTimer(hello_timer_interval);
}
