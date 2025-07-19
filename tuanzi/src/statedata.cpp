#include "all.h"
#include "global.h"
#include "contextcontrolthread.h"
#include "statedata.h"

CStateData::CStateData() :
    logoff(),
    logoff_init(),
    disconnected(),
    field_B(),
    field_C(),
    req_id(),
    req_auth(),
    eap_failure(),
    eap_success(),
    auth_attempt_count(),
    recv_id2(),
    recv_id(),
    eap_md5_datalen(),
    eap_md5_data(),
    state(),
    prev_state(),
    hold_timeout(),
    connect_timeout(),
    auth_timeout(),
    connect_timer(),
    hold_timer(),
    auth_timer(),
    hold_count(),
    state_acquired(),
    state_authenticated(),
    state_authenticating(),
    state_connecting(),
    state_disconnected(),
    state_hold(),
    state_logoff()
{}

CStateData::~CStateData()
{}

bool CStateData::IsAuthenticated() const
{
    return eap_success && !disconnected && !logoff && !logoff_init;
}

bool CStateData::IsDisconnected() const
{
    return disconnected;
}

bool CStateData::IsHeld() const
{
    return eap_failure && !disconnected && !logoff && !logoff_init;
}

bool CStateData::IsLogOff() const
{
    return logoff && !logoff_init && !disconnected;
}

void CStateData::SetAuthWhile()
{
    auth_timeout = 0;
}

void CStateData::SetHeldWhile()
{
    hold_timeout = 0;
}

void CStateData::SetRecvID(char id)
{
    recv_id = id;
}

void CStateData::SetReqAuth()
{
    req_auth = true;
}

void CStateData::SetReqID()
{
    req_id = true;
}

void CStateData::SetStartWhen()
{
    CtrlThread->InitStartDstMac();
    CtrlThread->dstaddr = CtrlThread->start_dst_addr;
    g_log_Wireless.AppendText(
        "dest addr %02x:%02x:%02x:%02x:%02x:%02x",
        CtrlThread->dstaddr.ether_addr_octet[0],
        CtrlThread->dstaddr.ether_addr_octet[1],
        CtrlThread->dstaddr.ether_addr_octet[2],
        CtrlThread->dstaddr.ether_addr_octet[3],
        CtrlThread->dstaddr.ether_addr_octet[4],
        CtrlThread->dstaddr.ether_addr_octet[5]
    );
    connect_timeout = 0;
}

void CStateData::SetUserLogOff()
{
    logoff = true;
}
