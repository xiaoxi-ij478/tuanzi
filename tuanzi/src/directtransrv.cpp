#include "all.h"
#include "mtypes.h"
#include "changelanguage.h"
#include "encodeutil.h"
#include "msgutil.h"
#include "util.h"
#include "global.h"
#include "cmdutil.h"
#include "directtransrv.h"

CDirectTranSrv::CDirectTranSrv() :
    sam_or_smp_inited(),
    destroy_mutex(),
    dir_thread(),
    dir_smp_para(),
    field_360(),
    field_361(),
    request_init_data_now(true),
    dir_trans_srvpara(),
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
    InitializeCriticalSection(&destroy_mutex);
    SetClassName("CDirectTranSrv");
}

CDirectTranSrv::~CDirectTranSrv()
{
    DeleteCriticalSection(&destroy_mutex);
}

void CDirectTranSrv::DispathMessage(struct LNXMSG *msg)
{
    switch (msg->mtype) {
        case ON_RECVPACKET_SAM_MTYPE:
            OnRecvPacket_SAM(msg->buflen, msg->buf);
            break;

        case ON_INIT_SAM_MTYPE:
            OnInit_SAM(msg->buflen, msg->buf);
            break;

        case ON_DEINIT_SAM_MTYPE:
            OnDeInit_SAM(msg->buflen, msg->buf);
            break;

        case ON_RECVPACKET_SMP_MTYPE:
            OnRecvPacket_SMP(msg->buflen, msg->buf);
            break;

        case ON_INIT_SMP_MTYPE:
            OnInit_SMP(msg->buflen, msg->buf);
            break;

        case ON_DEINIT_SMP_MTYPE:
            OnDeInit_SMP(msg->buflen, msg->buf);
            break;

        case ON_POST_SMP:
            OnPost_SMP(msg->buflen, msg->buf);
            break;

        case ON_POST_SAM:
            OnPost_SAM(msg->buflen, msg->buf);
            break;

        case ON_POST_NOREPONSE_SMP:
            OnPostNoResponse_SMP(msg->buflen, msg->buf);
            break;

        case ON_POST_NOREPONSE_SAM:
            OnPostNoResponse_SAM(msg->buflen, msg->buf);
            break;
    }
}

bool CDirectTranSrv::ExitInstance()
{
    DeInitDirectEnvironment();
    sam_or_smp_inited = false;
    return CLnxThread::ExitInstance();
}

bool CDirectTranSrv::InitInstance()
{
    InitDirectEnvironment();
    return CLnxThread::InitInstance();
}

void CDirectTranSrv::OnTimer(int tflag) const
{
    if (OnTimerEnter(tflag)) {
        if (!PostThreadMessage(ON_TIMER_MTYPE, tflag, reinterpret_cast<void *>(-1)))
            OnTimerLeave(tflag);

    } else
        g_logSystem.AppendText(
            "CDirectTranSrv::OnTimer(timerFlag=%d),return",
            tflag
        );
}

bool CDirectTranSrv::AnalyzePrivate_SAM(
    unsigned char *buf,
    unsigned buflen
) const
{
    unsigned char *logoff_buf = nullptr, *force_offline_buf = nullptr;
    bool advance_pos = true;
    std::string notify_str, force_offline_str, svr_switch_result_str;

    // the format is TLV
    for (
        unsigned pos = 0;
        pos < buflen;
        advance_pos && (pos += buf[pos + 1] + 2), advance_pos = true
    ) {
        if (buf[pos] != 1)
            continue;

        switch (buf[pos + 2]) {
            case 1:
                if (buf[pos + 3] != 2)
                    break;

                ConvertGBKToUtf8(
                    notify_str,
                    reinterpret_cast<char *>(&buf[pos + 5]),
                    buf[pos + 4]
                );
                AddMsgItem(5, notify_str);
                g_uilog.AppendText(
                    "AddMsgItem shownotify CDirectTranSrv::AnalyzePrivate_SAM,strInfo=%s",
                    notify_str.c_str()
                );
                shownotify(notify_str, CChangeLanguage::Instance().LoadString(95), 0);
                break;

            case 3:
                if (buf[pos + 3] != 6)
                    break;

                memcpy(
                    logoff_buf = new unsigned char[buf[pos + 4]],
                    &buf[pos + 5],
                    buf[pos + 4]
                );
                SimulateSuLogoff(logoff_buf, buf[pos + 4]);
                break;

            case 5:
                if (buf[pos + 3] != 8)
                    break;

                ConvertGBKToUtf8(
                    force_offline_str,
                    reinterpret_cast<char *>(&buf[pos + 5]),
                    buf[pos + 4]
                );
                memcpy(
                    force_offline_buf = new unsigned char[force_offline_str.length() + 1],
                    force_offline_str.c_str(),
                    force_offline_str.length()
                );
                logFile_debug.AppendText(
                    "强制下线解析接口(%s)",
                    force_offline_buf
                );
                CtrlThread->PostThreadMessage(
                    FORCE_OFFLINE_MTYPE,
                    force_offline_buf,
                    force_offline_str.length() + 1
                );
                break;

            case 7:
                if (buf[pos + 3] != 10)
                    break;

                svr_switch_result_str = AsciiToStr(&buf[pos + 5], buf[pos + 4]);
                RcvSvrSwitchResult();
                // TODO
                break;

            case 9:
                ParseDHCPAuthResult_ForSAM(buf, buflen, pos);
                advance_pos = false;
                break;
        }
    }

    return true;
}

bool CDirectTranSrv::AnalyzePrivate_SMP(
    unsigned char *buf,
    unsigned buflen
) const
{
    unsigned short smp_datalen = 0;
    unsigned char *smp_data = nullptr, *other_data = nullptr;

    if (buf[0] != 1 || ntohs(*reinterpret_cast<unsigned short *>(&buf[1])) != 1)
        return true;

    switch (buf[3]) {
        case 1:
            if (buf[4] != 11)
                break;

            smp_datalen = ntohs(*reinterpret_cast<unsigned short *>(&buf[5]));
            smp_data = new unsigned char[smp_datalen + 1];
            memcpy(smp_data, &buf[7], smp_datalen);
            ParseSMPData(smp_data);
            break;
#define PROCESS_DATA(id, func_name) \
case id: \
    other_data = new unsigned char[buflen - 4]; \
    memcpy(other_data, &buf[4], buflen - 4); \
    func_name(other_data, buflen - 4); \
    break
            PROCESS_DATA(2, ParseMsgAndPro);
            PROCESS_DATA(3, ParseLogoff);
            PROCESS_DATA(4, ParseReAuth);
            PROCESS_DATA(5, ParseGetHIStatusNow);
            PROCESS_DATA(6, ParseGetHostInfoNow);
            PROCESS_DATA(7, ParseLogoffOhers);
#undef PROCESS_DATA

        case 42:
            ParseModifyPWResult(&buf[4], buflen - 4);
            break;

        case 46:
            ParseDHCPAuthResult_ForSMP(&buf[4], buflen - 4);
            break;
    }

    return true;
}

bool CDirectTranSrv::AskForSmpInitData() const
{
    unsigned char ask_buf[1024] = {};
    unsigned cur_pos = 0;
    logFile_debug.AppendText("AskForSmpInitData called");
    assert(dir_thread);

    if (smp_init_data_gsn_sender_id != -1)
        dir_thread->CloseGSNSender(smp_init_data_gsn_sender_id);

    smp_init_data_gsn_sender_id =
        dir_thread->GSNSender(
            dir_smp_para.su_ipaddr,
            dir_smp_para.su_port,
            dir_smp_para.smp_ipaddr,
            dir_smp_para.smp_port
        );
#define PUT_TYPE(type) ask_buf[cur_pos++] = type
#define PUT_LENGTH(length) \
    do { \
        *reinterpret_cast<unsigned short *>(&ask_buf[cur_pos]) = htons(length); \
        cur_pos += 2; \
    } while (0)
#define PUT_DATA(buf, buflen) \
    do { \
        memcpy(&ask_buf[cur_pos], buf, buflen); \
        cur_pos += buflen; \
    } while (0)
#define PUT_DATA_IMMEDIATE_BYTE(byte) ask_buf[cur_pos++] = byte;
    PUT_TYPE(1);
    PUT_LENGTH(1);
    PUT_DATA_IMMEDIATE_BYTE(41);
    PUT_TYPE(2);
    PUT_LENGTH(strlen(dir_smp_para.field_0));
    PUT_DATA(dir_smp_para, field_0, strlen(dir_smp_para.field_0));
    PUT_TYPE(3);
    PUT_LENGTH(dir_smp_para.field_8A_len);
    PUT_DATA(dir_smp_para.field_8A, dir_smp_para.field_8A_len);
#undef PUT_TYPE
#undef PUT_LENGTH
#undef PUT_DATA
#undef PUT_DATA_IMMEDIATE_BYTE
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

std::string CDirectTranSrv::GetXmlChild_Node_STR(
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
    unsigned &next_buflen
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
