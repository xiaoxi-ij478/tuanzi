#include "all.h"
#include "global.h"
#include "util.h"
#include "mtypes.h"
#include "sendpacketthread.h"
#include "contextcontrolthread.h"
#include "hellothread.h"

CHelloThread::CHelloThread() :
    msgid(-1),
    hello_timer_interval(),
    hello_para(),
    hello_timerid(),
    field_1E8(),
    hello_id_offset(),
    field_1F0()
{
    SetClassName("CHelloThread");
}

void CHelloThread::DispathMessage(struct LNXMSG *msg)
{
    switch (msg->mtype) {
            HANDLE_MTYPE(ON_TIMER_MTYPE, OnHelloTimer);
            HANDLE_MTYPE(SET_HELLOTIMER_PLEASE_MTYPE, OnSetHelloTimerPlease);
            HANDLE_MTYPE(CHANGE_HELLOPARA_MTYPE, OnChangeHelloPara);
    }
}

bool CHelloThread::InitInstance()
{
    field_1F0 = rand() % 0x1001 + 0x1001;
    memset(e_pHelloID, 0x74, sizeof(e_pHelloID));
    return CLnxThread::InitInstance();
}

void CHelloThread::OnTimer(int tflag)
{
    if (OnTimerEnter(tflag)) {
        if (
            tflag == HELLO_TIMER_MTYPE &&
            !PostThreadMessage(ON_TIMER_MTYPE, HELLO_TIMER_MTYPE, -1)
        )
            OnTimerLeave(HELLO_TIMER_MTYPE);

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
        reinterpret_cast<struct ether_addr *>(ret->ether_header.ether_shost)
    );
    *reinterpret_cast<struct ether_addr *>(ret->ether_header.ether_dhost) =
        CtrlThread->dstaddr;
    ret->ether_header.ether_type = htons(ETH_P_PAE);
    ret->code = 1;
    ret->id = 0xBF;
    ret->length = htons(
                      sizeof(struct HelloPacket) - offsetof(struct HelloPacket, id)
                  );
    ret->field_14.field_0 = 0x6011113;
    ret->field_1E.field_0 = 0x6011113;
    ret->field_28 = 0x3011113;
    ret->field_14.field_4 = htonl(hello_para + (cur_crcid = GetCurCRCID()));
    ret->field_1E.field_4 = htonl(cur_crcid + 0x74 - e_pHelloID[hello_id_offset++]);
    hello_id_offset &= 15;
    encode(
        reinterpret_cast<char *>(&ret->type),
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

    hello_timerid =
        SetTimer(nullptr, HELLO_TIMER_MTYPE, hello_timer_interval, nullptr);
    OnHelloTimer(HELLO_TIMER_MTYPE, -1);
}

unsigned CHelloThread::GetCurCRCID()
{
    unsigned id = e_pHelloID[hello_id_offset];

    if (id == 0x74)
        return ++field_1F0;

    for (unsigned i = 0; i < 16; i++)
        if (e_pHelloID[i] == 0x74)
            e_pHelloID[i] = id;

    return ++field_1F0;
}

DEFINE_DISPATH_MESSAGE_HANDLER(OnChangeHelloPara, CHelloThread)
{
    UNUSED_VAR(arg2);

    if (arg1)
        hello_para = arg1;

    CreateHelloTimer(arg1);
}

DEFINE_DISPATH_MESSAGE_HANDLER(OnHelloTimer, CHelloThread)
{
    UNUSED_VAR(arg2);
    unsigned rbuflen = 0;
    struct HelloPacket *hello_packet = CreateHelloPacket(rbuflen);

    if (!hello_packet) {
        OnTimerLeave(arg1);
        return;
    }

    if (CtrlThread->machine_thread && CtrlThread->send_packet_thread) {
        if (
            !CtrlThread->send_packet_thread->PostThreadMessage(
                SEND_MESSAGE_MTYPE,
                rbuflen,
                reinterpret_cast<unsigned long>(hello_packet)
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

    OnTimerLeave(arg1);
}

DEFINE_DISPATH_MESSAGE_HANDLER(OnSetHelloTimerPlease, CHelloThread)
{
    UNUSED_VAR(arg1);
    UNUSED_VAR(arg2);
    CreateHelloTimer(hello_timer_interval);
}
