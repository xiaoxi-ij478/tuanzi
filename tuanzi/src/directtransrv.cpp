#include "all.h"
#include "directtransrv.h"

CDirectTranSrv::CDirectTranSrv() :
    sam_or_smp_inited(),
    destory_mutex(),
    dir_thread(),
    dir_smp_para(),
    field_360(),
    field_361(),
    request_init_data_now(true),
    dir_tran_srvpara(),
    sam_init_timerid(),
    sam_gsn_receiver_id(-1),
    sam_utc_time(),
    sam_timestamp(),
    hello_timer(),
    smp_gsn_receiver_id(-1),
    smp_utc_time(),
    smp_timestamp(),
    handshake_gsn_sender_id(),
    smp_init_timerid(),
    timer_ask(),
    smp_init_data_gsn_sender_id(),
    smp_init_packet()
{
    dir_smp_para.timeout = 3000;
    dir_smp_para.retry_count = 3;
    dir_smp_para.su_port = 80;
    dir_smp_para.version = 1;
    dir_trans_srvpara.version = 1;
    dir_trans_srvpara.timeout = 3000;
    dir_trans_srvpara.retry_count = 3;
    dir_trans_srvpara.sam_port = 8080;
    dir_trans_srvpara.su_port = 80;
    InitializeCriticalSection(&destory_mutex);
    SetClassName("CDirectTranSrv");
}

CDirectTranSrv::~CDirectTranSrv()
{
    DeleteCriticalSection(&destory_mutex);
}

void CDirectTranSrv::DispathMessage(struct LNXMSG *msg)
{switch (msg->mtype)
{
    case
}
}

bool CDirectTranSrv::ExitInstance()
{
}

bool CDirectTranSrv::InitInstance()
{
}

void CDirectTranSrv::OnTimer(int tflag) const
{
}

bool CDirectTranSrv::AnalyzePrivate_SAM(
    unsigned char *buf,
    unsigned buflen
) const
{
}

bool CDirectTranSrv::AnalyzePrivate_SMP(
    unsigned char *buf,
    unsigned buflen
) const
{
}

bool CDirectTranSrv::AskForSmpInitData() const
{
}

bool CDirectTranSrv::DeInitDirectEnvironment() const
{
}

bool CDirectTranSrv::DeInit_Sam() const
{
}

bool CDirectTranSrv::DeInit_Smp() const
{
}

void CDirectTranSrv::Destroy() const
{
}

void CDirectTranSrv::DoWithAuthResult(bool should_do) const
{
}

unsigned long CDirectTranSrv::GetSMPTimestamp() const
{
}

int CDirectTranSrv::GetXmlChild_Node_INT(
    TiXmlNode *node,
    const std::string &value,
    const std::string &
) const
{
}

string CDirectTranSrv::GetXmlChild_Node_STR(
    TiXmlNode *node,
    const std::string &value,
    const std::string &
) const
{
}

bool CDirectTranSrv::HandshakeToSAM() const
{
}

bool CDirectTranSrv::HandshakeToSMP() const
{
}

bool CDirectTranSrv::InitDirThread() const
{
}

bool CDirectTranSrv::InitDirectEnvironment() const
{
}

bool CDirectTranSrv::Init_Sam(
    struct tagDirectTranSrvPara *srv_para,
    bool wait
) const
{
}

bool CDirectTranSrv::Init_Smp(struct tagSmpParaDir *srv_para, bool wait) const
{
}

void CDirectTranSrv::OnDeInit_SAM(unsigned long buflen, void *buf) const
{
}

void CDirectTranSrv::OnDeInit_SMP(unsigned long buflen, void *buf) const
{
}

void CDirectTranSrv::OnInit_SAM(unsigned long buflen, void *buf) const
{
}

void CDirectTranSrv::OnInit_SMP(unsigned long buflen, void *buf) const
{
}

void CDirectTranSrv::OnPostNoResponse_SAM(unsigned long buflen, void *buf) const
{
}

void CDirectTranSrv::OnPostNoResponse_SMP(unsigned long buflen, void *buf) const
{
}

void CDirectTranSrv::OnPost_SAM(unsigned long buflen, void *buf) const
{
}

void CDirectTranSrv::OnPost_SMP(unsigned long buflen, void *buf) const
{
}

void CDirectTranSrv::OnRecvPacket_SAM(unsigned long buflen, void *buf) const
{
}

void CDirectTranSrv::OnRecvPacket_SMP(unsigned long buflen, void *buf) const
{
}

void CDirectTranSrv::OnTimer(unsigned long buflen, void *buf) const
{
}

void CDirectTranSrv::ParseACLParam(unsigned char *buf, unsigned buflen) const
{
}

void CDirectTranSrv::ParseDHCPAuthResult_ForSAM(
    unsigned char *buf,
    unsigned buflen,
    int &next_buflen
) const
{
}

void CDirectTranSrv::ParseDHCPAuthResult_ForSMP(
    unsigned char *buf,
    unsigned buflen
) const
{
}

void CDirectTranSrv::ParseGetHIStatusNow(
    unsigned char *buf,
    unsigned buflen
) const
{
}

void CDirectTranSrv::ParseGetHostInfoNow(
    unsigned char *buf,
    unsigned buflen
) const
{
}

void CDirectTranSrv::ParseLogoff(unsigned char *buf, unsigned buflen) const
{
}

void CDirectTranSrv::ParseLogoffOhers(unsigned char *buf, unsigned buflen) const
{
}

void CDirectTranSrv::ParseModifyPWResult(
    unsigned char *buf,
    unsigned buflen
) const
{
}

void CDirectTranSrv::ParseMsgAndPro(unsigned char *buf, unsigned buflen) const
{
}

void CDirectTranSrv::ParsePasswordSecurity(TiXmlDocument &xml) const
{
}

void CDirectTranSrv::ParseRM_Assist(unsigned char *buf, unsigned buflen) const
{
}

void CDirectTranSrv::ParseReAuth(unsigned char *buf, unsigned buflen) const
{
}

void CDirectTranSrv::ParseSMPData(unsigned char *buf, unsigned buflen) const
{
}

bool CDirectTranSrv::PostToSam(unsigned char *buf, unsigned buflen) const
{
}

bool CDirectTranSrv::PostToSamWithNoResponse(
    unsigned char *buf,
    unsigned buflen
) const
{
}

bool CDirectTranSrv::PostToSmp(unsigned char *buf, unsigned buflen) const
{
}

bool CDirectTranSrv::PostToSmpWithNoResponse(
    unsigned char *buf,
    unsigned buflen
) const
{
}

bool CDirectTranSrv::SendToSam(
    unsigned char *buf,
    unsigned buflen,
    unsigned timeout
) const
{
}

bool CDirectTranSrv::SendToSamWithNoResponse(
    unsigned char *buf,
    unsigned buflen,
    unsigned timeout
) const
{
}

bool CDirectTranSrv::SendToSmp(
    unsigned char *buf,
    unsigned buflen,
    unsigned timeout
) const
{
}

bool CDirectTranSrv::SendToSmpWithNoResponse(
    unsigned char *buf,
    unsigned buflen,
    unsigned timeout
) const
{
}
