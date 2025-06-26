#include "all.h"
#include "encodeutil.h"
#include "msgutil.h"
#include "threadutil.h"
#include "cmdutil.h"
#include "timeutil.h"
#include "util.h"
#include "mtypes.h"
#include "changelanguage.h"
#include "global.h"
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
            ParseSMPData(smp_data, smp_datalen);
            break;
#define PROCESS_DATA(id, func_name) \
case (id): \
    other_data = new unsigned char[buflen - 4]; \
    memcpy(other_data, &buf[4], buflen - 4); \
    (func_name)(other_data, buflen - 4); \
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
#define PUT_TYPE(type) ask_buf[cur_pos++] = (type)
#define PUT_LENGTH(length) \
    do { \
        *reinterpret_cast<unsigned short *>(&ask_buf[cur_pos]) = htons(length); \
        cur_pos += 2; \
    } while (0)
#define PUT_DATA(buf, buflen) \
    do { \
        memcpy(&ask_buf[cur_pos], (buf), (buflen)); \
        cur_pos += (buflen); \
    } while (0)
#define PUT_DATA_IMMEDIATE_BYTE(byte) ask_buf[cur_pos++] = (byte)
    PUT_TYPE(1);
    PUT_LENGTH(1);
    PUT_DATA_IMMEDIATE_BYTE(41);
    PUT_TYPE(2);
    PUT_LENGTH(strlen(dir_smp_para.field_0));
    PUT_DATA(dir_smp_para.field_0, strlen(dir_smp_para.field_0));
    PUT_TYPE(3);
    PUT_LENGTH(dir_smp_para.field_8A_len);
    PUT_DATA(dir_smp_para.field_8A, dir_smp_para.field_8A_len);
#undef PUT_TYPE
#undef PUT_LENGTH
#undef PUT_DATA
#undef PUT_DATA_IMMEDIATE_BYTE
    dir_thread->sendMessageWithTimeout(
        smp_init_data_gsn_sender_id,
        ask_buf,
        cur_pos,
        10
    );
    logFile_debug.AppendText("AskForSmpInitData end");
    return true;
}

bool CDirectTranSrv::DeInitDirectEnvironment() const
{
    logFile_debug.AppendText("DeInitDirectEnvironment call");
    Destroy();
    logFile_debug.AppendText("DeInitDirectEnvironment end");
    return true;
}

bool CDirectTranSrv::DeInit_Sam() const
{
    WAIT_HANDLE wait_handle;
    logFile_debug.AppendText("DeInit_Sam called");

    if (!dir_thread)
        return true;

    dir_thread->StopRun();
    ::PostThreadMessage(
        thread_id,
        ON_DEINIT_SAM_MTYPE,
        reinterpret_cast<WAIT_HANDLE *>(&wait_handle),
        nullptr
    );
    sam_or_smp_inited = false;

    if (WaitForSingleObject(&wait_handle, 10000) == ETIMEDOUT) {
        OnDeInit_SAM(0, nullptr);
        logFile_debug.AppendText("强杀直通报文发送线程[正常流程]");
    }

    return true;
}

bool CDirectTranSrv::DeInit_Smp() const
{
    WAIT_HANDLE wait_handle;
    logFile_debug.AppendText("DeInit_Smp called");

    if (!dir_thread)
        return true;

    dir_thread->StopRun();
    ::PostThreadMessage(
        thread_id,
        ON_DEINIT_SMP_MTYPE,
        reinterpret_cast<WAIT_HANDLE *>(&wait_handle),
        nullptr
    );
    sam_or_smp_inited = false;

    if (WaitForSingleObject(&wait_handle, 10000) == ETIMEDOUT) {
        OnDeInit_SMP(0, nullptr);
        logFile_debug.AppendText("强杀直通报文发送线程[正常流程]");
    }

    return true;
}

void CDirectTranSrv::Destroy() const
{
    logFile_debug.AppendText("CDirectTranSrv::Destroy called");
    EnterCriticalSection(&destroy_mutex);

    if (dir_thread) {
        logFile_debug.AppendText("\t m_pDirThread StopRun");
        dir_thread->StopRun();

        if (handshake_gsn_sender_id != -1) {
            dir_thread->CloseGSNSender(handshake_gsn_sender_id);
            handshake_gsn_sender_id = -1;
        }

        if (sam_gsn_receiver_id != -1)
            dir_thread->CloseGSNReceiver(sam_gsn_receiver_id);

        if (smp_gsn_receiver_id != -1)
            dir_thread->CloseGSNReceiver(smp_gsn_receiver_id);

        if (sam_init_timerid) {
            KillTimer(sam_init_timerid);
            sam_init_timerid = 0;
        }

        if (hello_timer) {
            KillTimer(hello_timer)
            hello_timer = 0;
        }

        dir_thread->SafeExitThread(10000);
        dir_thread = nullptr;
    }

    sam_or_smp_inited = false;
    logFile_debug.AppendText("CDirectTranSrv::Destroy end");
    LeaveCriticalSection(&destroy_mutex);
}

void CDirectTranSrv::DoWithAuthResult(bool should_do) const
{
    if (!should_do)
        return;

    logFile_debug.AppendText("DoWithAuthResult - AskForSmpInitData");

    if (smp_init_timerid) {
        KillTimer(smp_init_timerid);
        smp_init_timerid = 0;
    }

    timer_ask = 0;
    AskForSmpInitData();
    timer_ask++;
    logFile_debug.AppendText(
        "m_nTimerSMP_Init=%d",
        smp_init_timerid = SetTimer(0x6A, 30000)
    );
}

unsigned long CDirectTranSrv::GetSMPTimestamp() const
{
    return GetTickCount() + smp_utc_time - smp_timestamp;
}

int CDirectTranSrv::GetXmlChild_Node_INT(
    const TiXmlNode *node,
    const std::string &value,
    const std::string &
) const
{
    TiXmlElement *el = nullptr;
    const char *text = nullptr;

    if (!node)
        return 0;

    if (!(el = node->FirstChildElement(value)))
        return 0;

    if (!(el = el->ToElement()))
        return 0;

    if (!(text = el->GetText()))
        return 0;

    return strtol(text, nullptr, 10);
}

std::string CDirectTranSrv::GetXmlChild_Node_STR(
    const TiXmlNode *node,
    const std::string &value,
    const std::string &
) const
{
    std::string ret;
    TiXmlElement *el = nullptr;
    const char *text = nullptr;

    if (value.empty())
        return ret;

    if (!node)
        return ret;

    if (!(el = node->FirstChildElement(value)))
        return ret;

    if (!(el = el->ToElement()))
        return ret;

    if (!(text = el->GetText()))
        return ret;

    return ret = text;
}

bool CDirectTranSrv::HandshakeToSAM() const
{
    unsigned char handshake_buf[1024] = {};
    unsigned cur_pos = 0;

    if (handshake_gsn_sender_id != -1)
        dir_thread->CloseGSNSender(handshake_gsn_sender_id);

    handshake_gsn_sender_id =
        dir_thread->GSNSender(
            dir_trans_srvpara.su_ipaddr,
            dir_trans_srvpara.su_port,
            dir_trans_srvpara.sam_ipaddr,
            dir_trans_srvpara.sam_port
        );
#define PUT_TYPE(type) handshake_buf[cur_pos++] = (type)
#define PUT_LENGTH(length) handshake_buf[cur_pos++] = (length)
#define PUT_DATA(buf, buflen) \
    do { \
        memcpy(&handshake_buf[cur_pos], (buf), (buflen)); \
        cur_pos += (buflen); \
    } while (0)
#define PUT_DATA_IMMEDIATE_BYTE(byte) handshake_buf[cur_pos++] = (byte)
    PUT_TYPE(1);
    PUT_LENGTH(1);
    PUT_DATA_IMMEDIATE_BYTE(2);
    PUT_TYPE(3);
    PUT_LENGTH(strlen(dir_trans_srvpara.field_8));
    PUT_DATA(dir_trans_srvpara.field_8, strlen(dir_trans_srvpara.field_8));
    PUT_TYPE(4);
    PUT_LENGTH(4);
    PUT_DATA(&dir_trans_srvpara.su_ipaddr, 4);
    PUT_TYPE(5);
    PUT_LENGTH(6);
    PUT_DATA(&dir_trans_srvpara.field_8C, 6);
#undef PUT_TYPE
#undef PUT_LENGTH
#undef PUT_DATA
#undef PUT_DATA_IMMEDIATE_BYTE
    logFile_debug.AppendText("发SAM心跳报文");
    dir_thread.PostPacketSAMHeartbeatNoResponse(
        handshake_gsn_sender_id,
        handshake_buf
        cur_pos
    );
    return true;
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
