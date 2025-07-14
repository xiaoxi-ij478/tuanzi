#include "all.h"
#include "statemachinethread.h"

CStateMachineThread::CStateMachineThread()
{
    //ctor
}

CStateMachineThread::~CStateMachineThread()
{
    //dtor
}

bool CStateMachineThread::InitInstance()
{
}

void CStateMachineThread::DispathMessage(struct LNXMSG *msg)
{
}

void CStateMachineThread::OnTimer(int tflag) const
{
}

bool CStateMachineThread::ExitInstance()
{
}

CStateVisual *CStateMachineThread::CreateState(enum STATES state) const
{
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
}

void CStateMachineThread::FailNotification(struct EAPOLFrame *eapol_frame) const
{
}

void CStateMachineThread::InitState() const
{
}

DEFINE_DISPATH_MESSAGE_HANDLER(OnPacketNotify, CStateMachineThread) const
{
}

DEFINE_DISPATH_MESSAGE_HANDLER(OnSayHello, CStateMachineThread) const
{
}

DEFINE_DISPATH_MESSAGE_HANDLER(OnSendLogoff, CStateMachineThread) const
{
}

DEFINE_DISPATH_MESSAGE_HANDLER(OnStartMachine, CStateMachineThread) const
{
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
