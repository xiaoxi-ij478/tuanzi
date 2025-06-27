#include "all.h"
#include "encodeutil.h"
#include "msgutil.h"
#include "threadutil.h"
#include "cmdutil.h"
#include "timeutil.h"
#include "netutil.h"
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
            HANDLE_MTYPE(ON_RECVPACKET_SAM_MTYPE, OnRecvPacket_SAM);
            HANDLE_MTYPE(ON_INIT_SAM_MTYPE, OnInit_SAM);
            HANDLE_MTYPE(ON_DEINIT_SAM_MTYPE, OnDeInit_SAM);
            HANDLE_MTYPE(ON_RECVPACKET_SMP_MTYPE, OnRecvPacket_SMP);
            HANDLE_MTYPE(ON_INIT_SMP_MTYPE, OnInit_SMP);
            HANDLE_MTYPE(ON_DEINIT_SMP_MTYPE, OnDeInit_SMP);
            HANDLE_MTYPE(ON_POST_SMP, OnPost_SMP);
            HANDLE_MTYPE(ON_POST_SAM, OnPost_SAM);
            HANDLE_MTYPE(ON_POST_NOREPONSE_SMP, OnPostNoResponse_SMP);
            HANDLE_MTYPE(ON_POST_NOREPONSE_SAM, OnPostNoResponse_SAM);
            HANDLE_MTYPE(ON_TIMER_MTYPE, OnTimer);
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
        if (!PostThreadMessage(ON_TIMER_MTYPE, tflag, -1))
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
        pos < buflen
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
case (id): (func_name)(&buf[4], buflen - 4); break
            PROCESS_DATA(2, ParseMsgAndPro);
            PROCESS_DATA(3, ParseLogoff);
            PROCESS_DATA(4, ParseReAuth);
            PROCESS_DATA(5, ParseGetHIStatusNow);
            PROCESS_DATA(6, ParseGetHostInfoNow);
            PROCESS_DATA(7, ParseLogoffOhers);
            PROCESS_DATA(42, ParseModifyPWResult);
            PROCESS_DATA(46, ParseDHCPAuthResult_ForSMP);
#undef PROCESS_DATA
    }

    return true;
}

bool CDirectTranSrv::AskForSmpInitData()
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

bool CDirectTranSrv::DeInitDirectEnvironment()
{
    logFile_debug.AppendText("DeInitDirectEnvironment call");
    Destroy();
    logFile_debug.AppendText("DeInitDirectEnvironment end");
    return true;
}

bool CDirectTranSrv::DeInit_Sam()
{
    WAIT_HANDLE wait_handle;
    logFile_debug.AppendText("DeInit_Sam called");

    if (!dir_thread)
        return true;

    dir_thread->StopRun();
    ::PostThreadMessage(
        thread_id,
        ON_DEINIT_SAM_MTYPE,
        reinterpret_cast<unsigned long>(&wait_handle),
        0
    );
    sam_or_smp_inited = false;

    if (WaitForSingleObject(&wait_handle, 10000) == ETIMEDOUT) {
        OnDeInit_SAM(0, 0);
        logFile_debug.AppendText("强杀直通报文发送线程[正常流程]");
    }

    return true;
}

bool CDirectTranSrv::DeInit_Smp()
{
    WAIT_HANDLE wait_handle;
    logFile_debug.AppendText("DeInit_Smp called");

    if (!dir_thread)
        return true;

    dir_thread->StopRun();
    ::PostThreadMessage(
        thread_id,
        ON_DEINIT_SMP_MTYPE,
        reinterpret_cast<unsigned long>(&wait_handle),
        0
    );
    sam_or_smp_inited = false;

    if (WaitForSingleObject(&wait_handle, 10000) == ETIMEDOUT) {
        OnDeInit_SMP(0, 0);
        logFile_debug.AppendText("强杀直通报文发送线程[正常流程]");
    }

    return true;
}

void CDirectTranSrv::Destroy()
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
            KillTimer(hello_timer);
            hello_timer = 0;
        }

        dir_thread->SafeExitThread(10000);
        dir_thread = nullptr;
    }

    sam_or_smp_inited = false;
    logFile_debug.AppendText("CDirectTranSrv::Destroy end");
    LeaveCriticalSection(&destroy_mutex);
}

void CDirectTranSrv::DoWithAuthResult(bool should_do)
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
        smp_init_timerid = SetTimer(ASK_SMP_INIT_DATA_MTYPE, 30000)
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
    const TiXmlElement *el = nullptr;
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
    const TiXmlElement *el = nullptr;
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

bool CDirectTranSrv::HandshakeToSAM()
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
    dir_thread->PostPacketSAMHeartbeatNoResponse(
        handshake_gsn_sender_id,
        handshake_buf,
        cur_pos
    );
    return true;
}

bool CDirectTranSrv::HandshakeToSMP()
{
    unsigned char handshake_buf[1024] = {};
    unsigned cur_pos = 0;
    logFile_debug.AppendText("HandshakeToSMP called");
    assert(dir_thread);

    if (handshake_gsn_sender_id != -1)
        dir_thread->CloseGSNSender(handshake_gsn_sender_id);

    handshake_gsn_sender_id =
        dir_thread->GSNSender(
            dir_smp_para.su_ipaddr,
            dir_smp_para.su_port,
            dir_smp_para.smp_ipaddr,
            dir_smp_para.smp_port
        );
#define PUT_TYPE(type) handshake_buf[cur_pos++] = (type)
#define PUT_LENGTH(length) \
    do { \
        *reinterpret_cast<unsigned short *>(&handshake_buf[cur_pos]) = htons(length); \
        cur_pos += 2; \
    } while (0)
#define PUT_DATA(buf, buflen) \
    do { \
        memcpy(&handshake_buf[cur_pos], (buf), (buflen)); \
        cur_pos += (buflen); \
    } while (0)
#define PUT_DATA_IMMEDIATE_BYTE(byte) handshake_buf[cur_pos++] = (byte)
    PUT_TYPE(1);
    PUT_LENGTH(1);
    PUT_DATA_IMMEDIATE_BYTE(35);
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

    if (!dir_smp_para.hello_response)
        dir_thread->PostPacketNoResponse(
            handshake_gsn_sender_id,
            handshake_buf,
            cur_pos
        );

    else {
        if (
            dir_thread->sendMessageWithTimeout(
                handshake_gsn_sender_id,
                handshake_buf,
                cur_pos,
                10
            )
        )
            dir_smp_para.hello_fail_time = 0;

        else {
            logFile_debug.AppendText(
                "HandshakeToSMP m_dirSmpPara.nHelloFailTime =%d",
                ++dir_smp_para.hello_fail_time
            );

            if (dir_smp_para.hello_fail_time > 2) {
                if (hello_timer) {
                    KillTimer(hello_timer);
                    hello_timer = 0;
                }

                g_log_Wireless.AppendText("send hello to authsvr failed");

                if (!field_361) {
                    g_log_Wireless.AppendText("直通心跳超时重认证");
                    CtrlThread->PostThreadMessage(ASK_SMP_INIT_DATA_MTYPE, 2, nullptr);
                }
            }
        }
    }

    logFile_debug.AppendText("HandshakeToSMP end");
    return true;
}

bool CDirectTranSrv::InitDirThread()
{
    logFile_debug.AppendText("InitDirThread call");

    if (dir_thread) {
        Destroy();

        if (dir_thread) {
            logFile_debug.AppendText("InitDirThread end");
            return true;
        }
    }

    if (!(dir_thread = new CDirTranThread) || !dir_thread->DirTranThreadInit()) {
        logFile_debug.AppendText("直通协议线程初始化失败");
        dir_thread = nullptr;
        return false;
    }

    dir_thread->CreateThread(nullptr, false);

    if (!dir_thread->StartThread()) {
        logFile_debug.AppendText("InitDirThread end");
        return true;
    }

    g_logSystem.AppendText(
        "CDirectTranSrv::InitDirThread() pDirThread->StartThread failed"
    );
    return false;
}

bool CDirectTranSrv::InitDirectEnvironment()
{
    logFile_debug.AppendText("InitDirectEnvironment call");

    if (sam_init_timerid) {
        KillTimer(sam_init_timerid);
        sam_init_timerid = 0;
    }

    if (hello_timer) {
        KillTimer(hello_timer);
        hello_timer = 0;
    }

    if (dir_thread) {
        Destroy();

        if (dir_thread) {
            logFile_debug.AppendText("InitDirectEnvironment end");
            return true;
        }
    }

    if (!(dir_thread = new CDirTranThread) || !dir_thread->DirTranThreadInit()) {
        logFile_debug.AppendText("直通协议线程初始化失败");
        dir_thread = nullptr;
        return false;
    }

    dir_thread->CreateThread(nullptr, false);

    if (!dir_thread->StartThread()) {
        logFile_debug.AppendText("InitDirectEnvironment end");
        return true;
    }

    g_logSystem.AppendText(
        "CDirectTranSrv::InitDirectEnvironment() pDirThread->StartThread failed"
    );
    return false;
}

bool CDirectTranSrv::Init_Sam(
    struct tagDirectTranSrvPara *dir_srv_para, // pDirSrvPara
    bool wait
)
{
    WAIT_HANDLE wait_handle;
    assert(dir_srv_para);
    dir_trans_srvpara = *dir_srv_para;

    if (wait) {
        ::PostThreadMessage(
            thread_id,
            ON_INIT_SAM_MTYPE,
            reinterpret_cast<unsigned long>(&wait_handle),
            0
        );
        WaitForSingleObject(&wait_handle, 0);

    } else
        ::PostThreadMessage(thread_id, ON_INIT_SAM_MTYPE, 0, 0);

    return true;
}

bool CDirectTranSrv::Init_Smp(
    struct tagSmpParaDir *smp_para, // pSmpPara
    bool wait
)
{
    WAIT_HANDLE wait_handle;
    assert(smp_para);
    dir_smp_para = *smp_para;

    if (wait) {
        ::PostThreadMessage(
            thread_id,
            ON_INIT_SMP_MTYPE,
            reinterpret_cast<unsigned long>(&wait_handle),
            0
        );
        WaitForSingleObject(&wait_handle, 0);

    } else
        ::PostThreadMessage(thread_id, ON_INIT_SAM_MTYPE, 0, 0);

    return true;
}

DEFINE_DISPATH_MESSAGE_HANDLER(OnDeInit_SAM, CDirectTranSrv)
{
    logFile_debug.AppendText("OnDeInit_SAM call");
    EnterCriticalSection(&destroy_mutex);

    if (dir_thread) {
        dir_thread->StopRun();

        if (handshake_gsn_sender_id != -1) {
            dir_thread->CloseGSNSender(handshake_gsn_sender_id);
            handshake_gsn_sender_id = -1;
        }

        if (sam_gsn_receiver_id != -1)
            dir_thread->CloseGSNReceiver(sam_gsn_receiver_id);

        if (sam_init_timerid) {
            KillTimer(sam_init_timerid);
            sam_init_timerid = 0;
        }
    }

    sam_or_smp_inited = false;
    LeaveCriticalSection(&destroy_mutex);

    if (arg1)
        SetEvent(reinterpret_cast<WAIT_HANDLE *>(arg1), true);
}

DEFINE_DISPATH_MESSAGE_HANDLER(OnDeInit_SMP, CDirectTranSrv)
{
    EnterCriticalSection(&destroy_mutex);

    if (dir_thread) {
        dir_thread->StopRun();

        if (handshake_gsn_sender_id != -1) {
            dir_thread->CloseGSNSender(handshake_gsn_sender_id);
            handshake_gsn_sender_id = -1;
        }

        if (smp_gsn_receiver_id != -1)
            dir_thread->CloseGSNReceiver(smp_gsn_receiver_id);

        if (hello_timer) {
            KillTimer(hello_timer);
            hello_timer = 0;
        }
    }

    sam_or_smp_inited = false;
    LeaveCriticalSection(&destroy_mutex);

    if (arg1)
        SetEvent(reinterpret_cast<WAIT_HANDLE *>(arg1), true);
}

DEFINE_DISPATH_MESSAGE_HANDLER(OnInit_SAM, CDirectTranSrv)
{
    char ip_buf[32] = {};
    struct tagDirectCom_ProtocalParam proto_param = {
        dir_trans_srvpara.sam_ipaddr,
        dir_trans_srvpara.sam_port,
        dir_trans_srvpara.su_ipaddr,
        dir_trans_srvpara.gateway_ipaddr,
        dir_trans_srvpara.retry_count,
        dir_trans_srvpara.timeout,
        {},
        {},
        true,
        htonLONGLONG(dir_trans_srvpara.utc_time),
        GetTickCount(),
        dir_trans_srvpara.field_C5,
        dir_trans_srvpara.version
    };
    memcpy(
        &proto_param.keybuf,
        dir_trans_srvpara.keybuf,
        sizeof(proto_param.keybuf)
    );
    memcpy(
        &proto_param.ivbuf,
        dir_trans_srvpara.ivbuf,
        sizeof(proto_param.ivbuf)
    );

    if (!dir_thread) {
        if (arg1)
            SetEvent(reinterpret_cast<WAIT_HANDLE *>(arg1), false);

        return;
    }

    dir_thread->SetDirParaXieYi(proto_param);
    sam_utc_time = proto_param.utc_time;
    sam_timestamp = proto_param.timestamp;
    logFile_debug.AppendText("SAM直通协议参数如下：");
    logFile_debug.AppendText("直通协议版本: %d.", dir_trans_srvpara.version);
    logFile_debug.AppendText("重传次数：%d", dir_trans_srvpara.retry_count);
    logFile_debug.AppendText("超时时间：%d", dir_trans_srvpara.timeout);
    logFile_debug.AppendText(
        "DES key：%s",
        HexToString(
            dir_trans_srvpara.keybuf,
            sizeof(dir_trans_srvpara.keybuf)
        ).c_str()
    );
    logFile_debug.AppendText(
        "IV key：%s",
        HexToString(
            dir_trans_srvpara.ivbuf,
            sizeof(dir_trans_srvpara.ivbuf)
        ).c_str()
    );
    logFile_debug.AppendText(
        "UTC time：%s",
        HexToString(
            reinterpret_cast<unsigned char *>(&dir_trans_srvpara.utc_time),
            sizeof(dir_trans_srvpara.utc_time)
        ).c_str()
    );
    logFile_debug.AppendText(
        "SU PORT：%d",
        dir_trans_srvpara.su_port
    );
    sprintf(
        ip_buf,
        "%d.%d.%d.%d",
        dir_trans_srvpara.gateway_ipaddr & 0xff,
        dir_trans_srvpara.gateway_ipaddr >> 8 & 0xff,
        dir_trans_srvpara.gateway_ipaddr >> 16 & 0xff,
        dir_trans_srvpara.gateway_ipaddr >> 24
    );
    logFile_debug.AppendText("GetWay IP：%s", ip_buf);
    sprintf(
        ip_buf,
        "%d.%d.%d.%d",
        dir_trans_srvpara.su_ipaddr & 0xff,
        dir_trans_srvpara.su_ipaddr >> 8 & 0xff,
        dir_trans_srvpara.su_ipaddr >> 16 & 0xff,
        dir_trans_srvpara.su_ipaddr >> 24
    );
    logFile_debug.AppendText("SU IP：%s", ip_buf);
    sprintf(
        ip_buf,
        "%d.%d.%d.%d",
        dir_trans_srvpara.sam_ipaddr & 0xff,
        dir_trans_srvpara.sam_ipaddr >> 8 & 0xff,
        dir_trans_srvpara.sam_ipaddr >> 16 & 0xff,
        dir_trans_srvpara.sam_ipaddr >> 24
    );
    logFile_debug.AppendText(
        "SAM IP：%s,侦听端口：%d",
        ip_buf,
        dir_trans_srvpara.sam_port
    );
    sam_or_smp_inited = true;
    dir_thread->StartRun();

    if (dir_trans_srvpara.field_0) {
        if (sam_gsn_receiver_id != -1)
            dir_thread->CloseGSNReceiver(sam_gsn_receiver_id);

        sam_gsn_receiver_id =
            dir_thread->GSNReceiver(
                dir_trans_srvpara.sam_ipaddr,
                dir_trans_srvpara.sam_port,
                dir_trans_srvpara.su_ipaddr,
                dir_trans_srvpara.su_port,
                thread_id,
                ON_RECVPACKET_SAM_MTYPE
            );
    }

    logFile_debug.AppendText(
        "bUseHandshake2Sam：%d,m_dirTranSrvPara.dwTimerToSAM：%d",
        dir_trans_srvpara.use_handshake_to_sam,
        dir_trans_srvpara.timer_to_sam
    );

    if (dir_trans_srvpara.use_handshake_to_sam) {
        sam_init_timerid =
            SetTimer(
                HANDSHAKE_TO_SAM_MTYPE,
                1000 * dir_trans_srvpara.timer_to_sam
            );
        OnTimer(HANDSHAKE_TO_SAM_MTYPE);
    }

    if (arg1)
        SetEvent(reinterpret_cast<WAIT_HANDLE *>(arg1), false);
}

DEFINE_DISPATH_MESSAGE_HANDLER(OnInit_SMP, CDirectTranSrv)
{
    char ip_buf[32] = {};
    struct tagDirectCom_ProtocalParam proto_param = {
        dir_smp_para.smp_ipaddr,
        dir_smp_para.smp_port,
        dir_smp_para.su_ipaddr,
        dir_smp_para.gateway_ipaddr,
        dir_smp_para.retry_count,
        dir_smp_para.timeout,
        {},
        {},
        dir_smp_para.utc_time,
        htonLONGLONG(dir_smp_para.utc_time),
        GetTickCount(),
        field_360,
        dir_smp_para.version
    };
    memcpy(&proto_param.keybuf, dir_smp_para.keybuf, sizeof(proto_param.keybuf));
    memcpy(&proto_param.ivbuf, dir_smp_para.ivbuf, sizeof(proto_param.ivbuf));

    if (!dir_thread) {
        if (arg1)
            SetEvent(reinterpret_cast<WAIT_HANDLE *>(arg1), false);

        return;
    }

    dir_thread->SetDirParaXieYi(proto_param);
    smp_utc_time = proto_param.utc_time;
    smp_timestamp = proto_param.timestamp;
    logFile_debug.AppendText("SMP直通协议参数如下：");
    logFile_debug.AppendText("直通版本: %d.", dir_smp_para.version);
    logFile_debug.AppendText("重传次数：%d", dir_smp_para.retry_count);
    logFile_debug.AppendText("超时时间：%d", dir_smp_para.timeout);
    logFile_debug.AppendText(
        "DES key：%s",
        HexToString(
            dir_smp_para.keybuf,
            sizeof(dir_smp_para.keybuf)
        ).c_str()
    );
    logFile_debug.AppendText(
        "IV key：%s",
        HexToString(
            dir_smp_para.ivbuf,
            sizeof(dir_smp_para.ivbuf)
        ).c_str()
    );
    logFile_debug.AppendText(
        "UTC time：%s",
        HexToString(
            reinterpret_cast<unsigned char *>(&dir_smp_para.utc_time),
            sizeof(dir_smp_para.utc_time)
        ).c_str()
    );
    logFile_debug.AppendText("Su port：%d", dir_smp_para.su_port);
    createUdpBindSocket(dir_smp_para.su_port);
    sprintf(
        ip_buf,
        "%d.%d.%d.%d",
        dir_smp_para.gateway_ipaddr & 0xff,
        dir_smp_para.gateway_ipaddr >> 8 & 0xff,
        dir_smp_para.gateway_ipaddr >> 16 & 0xff,
        dir_smp_para.gateway_ipaddr >> 24
    );
    logFile_debug.AppendText("网关IP：%s", ip_buf);
    sprintf(
        ip_buf,
        "%d.%d.%d.%d",
        dir_smp_para.su_ipaddr & 0xff,
        dir_smp_para.su_ipaddr >> 8 & 0xff,
        dir_smp_para.su_ipaddr >> 16 & 0xff,
        dir_smp_para.su_ipaddr >> 24
    );
    logFile_debug.AppendText("Su本机IP：%s", ip_buf);
    sprintf(
        ip_buf,
        "%d.%d.%d.%d",
        dir_smp_para.smp_ipaddr & 0xff,
        dir_smp_para.smp_ipaddr >> 8 & 0xff,
        dir_smp_para.smp_ipaddr >> 16 & 0xff,
        dir_smp_para.smp_ipaddr >> 24
    );
    logFile_debug.AppendText(
        "SMP IP：%s,侦听端口：%d",
        ip_buf,
        dir_smp_para.smp_port
    );
    sam_or_smp_inited = true;
    dir_thread->StartRun();

    if (smp_gsn_receiver_id != -1)
        dir_thread->CloseGSNReceiver(smp_gsn_receiver_id);

    smp_gsn_receiver_id =
        dir_thread->GSNReceiver(
            dir_smp_para.smp_ipaddr,
            dir_smp_para.smp_port,
            dir_smp_para.su_ipaddr,
            dir_smp_para.su_port,
            thread_id,
            ON_RECVPACKET_SMP_MTYPE
        );
    timer_ask = 0;

    if (request_init_data_now) {
        logFile_debug.AppendText(
            "m_nTimerSMP_Init=%d",
            smp_init_timerid = SetTimer(ASK_SMP_INIT_DATA_MTYPE, 30000)
        );
        AskForSmpInitData();
        timer_ask++;

    } else
        logFile_debug.AppendText("request init data later.");

    if (arg1)
        SetEvent(reinterpret_cast<WAIT_HANDLE *>(arg1), false);
}

DEFINE_DISPATH_MESSAGE_HANDLER(OnPostNoResponse_SAM, CDirectTranSrv) const
{
    if (sam_or_smp_inited)
        dir_thread->PostPacketNoResponse(
            dir_thread->GSNSender(
                dir_trans_srvpara.su_ipaddr,
                dir_trans_srvpara.su_port,
                dir_trans_srvpara.sam_ipaddr,
                dir_trans_srvpara.sam_port
            ),
            arg1,
            arg2
        );

    delete[] reinterpret_cast<unsigned char *>(arg1);
}

DEFINE_DISPATH_MESSAGE_HANDLER(OnPostNoResponse_SMP, CDirectTranSrv) const
{
    if (sam_or_smp_inited)
        dir_thread->PostPacketNoResponse(
            dir_thread->GSNSender(
                dir_smp_para.su_ipaddr,
                dir_smp_para.su_port,
                dir_smp_para.smp_ipaddr,
                dir_smp_para.smp_port
            ),
            arg1,
            arg2
        );

    delete[] reinterpret_cast<unsigned char *>(arg1);
}

DEFINE_DISPATH_MESSAGE_HANDLER(OnPost_SAM, CDirectTranSrv) const
{
    if (!sam_or_smp_inited)
        return;

    assert(dir_thread);
    dir_thread->postMessage(
        dir_thread->GSNSender(
            dir_trans_srvpara.su_ipaddr,
            dir_trans_srvpara.su_port,
            dir_trans_srvpara.sam_ipaddr,
            dir_trans_srvpara.sam_port
        ),
        arg1,
        arg2
    );
    delete[] reinterpret_cast<unsigned char *>(arg1);
}

DEFINE_DISPATH_MESSAGE_HANDLER(OnPost_SMP, CDirectTranSrv) const
{
    if (!sam_or_smp_inited)
        return;

    assert(dir_thread);
    dir_thread->postMessage(
        dir_thread->GSNSender(
            dir_smp_para.su_ipaddr,
            dir_smp_para.su_port,
            dir_smp_para.smp_ipaddr,
            dir_smp_para.smp_port
        ),
        arg1,
        arg2
    );
    delete[] reinterpret_cast<unsigned char *>(arg1);
}

DEFINE_DISPATH_MESSAGE_HANDLER(OnRecvPacket_SAM, CDirectTranSrv) const
{
    logFile_debug.HexPrinter(arg1, arg2);
    AnalyzePrivate_SAM(arg1, arg2);
    delete[] reinterpret_cast<unsigned char *>(arg1);
}

DEFINE_DISPATH_MESSAGE_HANDLER(OnRecvPacket_SMP, CDirectTranSrv) const
{
    logFile_debug.HexPrinter(arg1, arg2);
    AnalyzePrivate_SMP(arg1, arg2);
    delete[] reinterpret_cast<unsigned char *>(arg1);
}

DEFINE_DISPATH_MESSAGE_HANDLER(OnTimer, CDirectTranSrv) const
{
    if (!dir_thread) {
        OnTimerLeave(arg1);
        return;
    }

    switch (arg1) {
        case HANDSHAKE_TO_SAM_MTYPE:
            HandshakeToSAM();
            logFile_debug.AppendText("HandshakeToSAM() call ");
            break;

        case HANDSHAKE_TO_SMP_MTYPE:
            HandshakeToSMP();
            break;

        case ASK_SMP_INIT_DATA_MTYPE:
            if (timer_ask < 3) {
                logFile_debug.AppendText(
                    "OnTimer - AskForSmpInitData "
                    "m_nTimerAsk=%d; m_nTimerSMP_Init=%d",
                    timer_ask++,
                    smp_init_timerid
                );
                AskForSmpInitData();
                break;
            }

            if (smp_init_timerid) {
                KillTimer(smp_init_timerid);
                smp_init_timerid = 0;
            }

            if (field_361)
                break;

            logFile_debug.AppendText(
                "Error - m_nTimerAsk >= 3, so it will LogOff_AskForInitData."
            );

            if (
                !CtrlThread->PostThreadMessage(
                    WM_OTHER_WANT_LOGOFF,
                    LOGOFF_SOMEWHAT_MTYPE,
                    1
                )
            )
                g_uilog.AppendText(
                    " CDirectTranSrv::OnTimer postMessage "
                    "WM_OTHER_WANT_LOGOFF error"
                );

            break;
    }

    OnTimerLeave(arg1);
}

void CDirectTranSrv::ParseACLParam(unsigned char *buf, unsigned buflen) const
{}

void CDirectTranSrv::ParseDHCPAuthResult_ForSAM(
    unsigned char *buf,
    unsigned buflen,
    unsigned &pos
) const
{
    std::string some_string;
    unsigned char *some_string_buf = nullptr;
    bool some_flag = false;
    assert(buf);

    for (; pos + buf[pos + 1] + 2 <= buflen; pos += buf[pos + 1] + 2)
        switch (buf[pos]) {
            case 2:
                ConvertGBKToUtf8(some_string, &buf[pos + 2], buf[pos + 1]);
                some_string_buf = new unsigned char[buf[pos + 1] + 1];
                memcpy(some_string_buf, some_string.c_str(), buf[pos + 1]);
                break;

            case 19:
                some_flag = buf[pos + 2];
                break;
        }

    if (
        !CtrlThread ||
        !CtrlThread->PostThreadMessage(
            ANNOUNCE_DHCP_RESULT_MTYPE,
            some_flag,
            some_string_buf
        )
    )
        delete[] some_string_buf;
}

void CDirectTranSrv::ParseDHCPAuthResult_ForSMP(
    unsigned char *buf,
    unsigned buflen
) const
{
    std::string some_string;
    unsigned char *some_string_buf = nullptr;
    bool some_flag = false;
    assert(buf);

    for (
        unsigned pos = 0, datalen = 0;
        pos + datalen + 3 <= buflen;
        datalen = ntohs(*reinterpret_cast<unsigned short *>(&buf[pos + 1])),
        pos += datalen + 3
    )
        switch (buf[pos]) {
            case 206:
                some_flag = buf[pos + 3];
                break;

            case 207:
                ConvertGBKToUtf8(some_string, &buf[pos + 3], datalen);
                some_string_buf = new unsigned char[datalen + 1];
                memcpy(some_string_buf, some_string.c_str(), datalen);
                break;
        }

    if (
        !CtrlThread ||
        !CtrlThread->PostThreadMessage(
            ANNOUNCE_DHCP_RESULT_MTYPE,
            some_flag,
            some_string_buf
        )
    ) {
        delete[] some_string_buf;
        return;
    }

    DoWithAuthResult(some_flag);
}

void CDirectTranSrv::ParseGetHIStatusNow(
    unsigned char *buf,
    unsigned buflen
) const
{}

void CDirectTranSrv::ParseGetHostInfoNow(
    unsigned char *buf,
    unsigned buflen
) const
{}

void CDirectTranSrv::ParseLogoff(unsigned char *buf, unsigned buflen) const
{
    std::string some_string;
    unsigned char *some_string_buf = nullptr;

    if (!buflen)
        return;

    for (
        unsigned pos = 0, datalen = 0;
        pos + datalen + 3 <= buflen;
        datalen = ntohs(*reinterpret_cast<unsigned short *>(&buf[pos + 1])),
        pos += datalen + 3
    )
        switch (buf[pos]) {
            case 31:
                if (datalen) {
                    ConvertGBKToUtf8(some_string, &buf[pos + 3], datalen);
                    some_string_buf = new unsigned char[datalen + 2];
                    memcpy(some_string_buf, some_string.c_str(), datalen);
                }

                if (
                    !CtrlThread->PostThreadMessage(
                        DO_FORCE_OFFLINE_MTYPE,
                        some_string_buf,
                        datalen ? datalen + 2 : 0
                    )
                )
                    delete[] some_string_buf;

                logFile_debug.AppendText(
                    "执行强制下线命令,strLogoffMsg=%s",
                    some_string.c_str()
                );
                break;
        }
}

void CDirectTranSrv::ParseLogoffOhers(unsigned char *buf, unsigned buflen) const
{
    unsigned char *some_string_buf = nullptr;

    if (!buflen)
        return;

    for (
        unsigned pos = 0, datalen = 0;
        pos + datalen + 3 <= buflen;
        datalen = ntohs(*reinterpret_cast<unsigned short *>(&buf[pos + 1])),
        pos += datalen + 3
    )
        switch (buf[pos]) {
            case 71:
                some_string_buf = new unsigned char[datalen];
                memcpy(some_string_buf, &buf[pos + 3], datalen);
                logFile_debug.AppendText("执行模拟其他用户下线命令");
                SimulateSuLogoff(some_string_buf, datalen);
                break;
        }
}

void CDirectTranSrv::ParseModifyPWResult(
    unsigned char *buf,
    unsigned buflen
) const
{
    unsigned char *some_string_buf = nullptr;
    bool some_flag = false;
    assert(buf && buflen);

    for (
        unsigned pos = 0, datalen = 0;
        pos + datalen + 3 <= buflen;
        datalen = ntohs(*reinterpret_cast<unsigned short *>(&buf[pos + 1])),
        pos += datalen + 3
    )
        switch (buf[pos]) {
            case 193:
                some_flag = buf[pos + 3];
                break;

            case 194:
                some_string_buf = new unsigned char[datalen + 1];
                memcpy(some_string_buf, &buf[pos + 3], datalen);
                break;
        }

    RcvModifyPasswordResult(some_flag, some_string_buf);
}

void CDirectTranSrv::ParseMsgAndPro(unsigned char *buf, unsigned buflen) const
{
    std::string message, url;
    bool has_message = false, has_url = false;

    for (
        unsigned pos = 0, datalen = 0;
        pos + datalen + 3 <= buflen;
        datalen = ntohs(*reinterpret_cast<unsigned short *>(&buf[pos + 1])),
        pos += datalen + 3
    )
        switch (buf[pos]) {
            case 21:
                has_message = true;
                ConvertGBKToUtf8(message, &buf[pos + 3], datalen);
                break;

            case 23:
                ConvertGBKToUtf8(url, &buf[pos + 3], datalen);
                [[fallthrough]];

            case 22:
                has_url = true;
                break;
        }

    if (has_message && !message.empty()) {
        AddMsgItem(1, message);
        g_uilog.AppendText(
            "ParseMsgAndPro AddMsgItem shownotify "
            "CDirectTranSrv::AnalyzePrivate_SMP(CString strMsg)=%s",
            message.c_str()
        );
        shownotify(message, CChangeLanguage::Instance().LoadString(95), 0);
    }

    if (has_url) {
        AddMsgItem(0, url);
        g_uilog.AppendText(
            "ParseMsgAndPro AddMsgItem shownotify "
            "CDirectTranSrv::AnalyzePrivate_SMP(CString url)=%s",
            url.c_str()
        );
        show_url(CChangeLanguage::Instance().LoadString(95), url);
    }
}

void CDirectTranSrv::ParsePasswordSecurity(TiXmlDocument &xml) const
{
    const TiXmlNode *ss_child = nullptr;
    const TiXmlNode *ssp_child = nullptr;
    const TiXmlNode *sspm_child = nullptr;
    const TiXmlNode *psd_child = nullptr;
    struct tagPasSecurityInfo secinfo = {
        false, 30, true, "", -1, false, -1
    };

    if ((ss_child = xml.FirstChildElement().FirstChild("self_service")))
        if ((ssp_child = ss_child.FirstChild("password"))) {
            secinfo.enable_modify_pw =
                GetXmlChild_Node_STR(
                    ssp_child,
                    "is_modify",
                    "password"
                ) == "true";

            if (secinfo.enable_modify_pw)
                secinfo.timeout =
                    GetXmlChild_Node_INT(ssp_child, "wait_time", "password");
        }

    if ((psd_child = xml.FirstChildElement().FirstChild("psw_security_detect"))) {
        secinfo.result =
            GetXmlChild_Node_INT(psd_child, "result", "psw_security_detect");
        secinfo.failcode =
            GetXmlChild_Node_INT(psd_child, "fail_code", "psw_security_detect");
        secinfo.force_offline =
            GetXmlChild_Node_INT(
                psd_child,
                "force_offline",
                "psw_security_detect"
            );
        secinfo.offline_wait_time =
            GetXmlChild_Node_INT(
                psd_child,
                "offline_wait_time",
                "psw_security_detect"
            );
    }

    if ((ss_child = xml.FirstChildElement().FirstChild("self_service")))
        if ((ssp_child = ss_child.FirstChild("password"))) {
            secinfo.enable_modify_pw =
                GetXmlChild_Node_STR(ssp_child, "is_modify", "password") == "true";
            secinfo.timeout =
                GetXmlChild_Node_INT(ssp_child, "wait_time", "password");

            if (ssp_child.FirstChildElement("modify_at_once")) {
                logFile_debug.AppendText(
                    "ParsePasswordSecurity modify_at_once element exist."
                );
                secinfo.result =
                    GetXmlChild_Node_STR(
                        ssp_child,
                        "modify_at_once",
                        "password"
                    ) != "true";
            }

            std::string &&password_modify_message =
                GetXmlChild_Node_STR(ssp_child, "modify_message", "password");
            ConvertGBKToUtf8(
                secinfo.password_modify_message,
                password_modify_message.c_str(),
                password_modify_message.length()
            );

            if (ssp_child.FirstChildElement("force_offline")) {
                logFile_debug.AppendText(
                    "ParsePasswordSecurity force_offline element exist."
                );
                secinfo.force_offline =
                    GetXmlChild_Node_STR(
                        ssp_child,
                        "force_offline",
                        "password"
                    ) == "true";
            }

            if (ssp_child.FirstChildElement("offline_wait_time")) {
                logFile_debug.AppendText(
                    "ParsePasswordSecurity offline_wait_time element exist."
                );
                secinfo.offline_wait_time =
                    GetXmlChild_Node_INT(
                        ssp_child,
                        "offline_wait_time",
                        "password"
                    );
            }
        }

    logFile_debug.AppendText(
        "ParsePasswordSecurity bEnableModifyPW=%d,timeOut=%d,result=%d,"
        "failCode=%d,forceOffline=%d,offlineWaitTime=%d",
        secinfo.enable_modify_pw,
        secinfo.timeout,
        secinfo.result,
        secinfo.failcode,
        secinfo.force_offline,
        secinfo.offline_wait_time
    );
    CPasswordModifier::SetPasswordSecurityInfo(secinfo);
}

void CDirectTranSrv::ParseRM_Assist(unsigned char *buf, unsigned buflen) const
{}

void CDirectTranSrv::ParseReAuth(unsigned char *buf, unsigned buflen) const
{
    delete[] buf;
    CtrlThread->PostThreadMessage(REAUTH_MTYPE, 0, 0);
}

void CDirectTranSrv::ParseSMPData(unsigned char *buf, unsigned buflen) const
{
}

bool CDirectTranSrv::PostToSam(unsigned char *buf, unsigned buflen) const
{
    bool ret = false;
    unsigned char *newbuf = new unsigned char[buflen];

    if (!newbuf)
        return ret;

    memcpy(newbuf, buf, buflen);

    if (!(ret =::PostThreadMessage(thread_id, ON_POST_SAM, newbuf, buflen)))
        delete[] newbuf;

    return ret;
}

bool CDirectTranSrv::PostToSamWithNoResponse(
    unsigned char *buf,
    unsigned buflen
) const
{
    unsigned char *newbuf = new unsigned char[buflen];

    if (!newbuf)
        return false;

    memcpy(newbuf, buf, buflen);
    return ::PostThreadMessage(thread_id, ON_POST_NOREPONSE_SAM, newbuf, buflen);
}

bool CDirectTranSrv::PostToSmp(unsigned char *buf, unsigned buflen) const
{
    bool ret = false;
    unsigned char *newbuf = new unsigned char[buflen];

    if (!newbuf)
        return ret;

    memcpy(newbuf, buf, buflen);

    if (!(ret =::PostThreadMessage(thread_id, ON_POST_SMP, newbuf, buflen)))
        delete[] newbuf;

    return ret;
}

bool CDirectTranSrv::PostToSmpWithNoResponse(
    unsigned char *buf,
    unsigned buflen
) const
{
    unsigned char *newbuf = new unsigned char[buflen];

    if (!newbuf)
        return false;

    memcpy(newbuf, buf, buflen);
    return ::PostThreadMessage(thread_id, ON_POST_NOREPONSE_SMP, newbuf, buflen);
}

bool CDirectTranSrv::SendToSam(
    unsigned char *buf,
    unsigned buflen,
    unsigned timeout
) const
{
    if (!sam_or_smp_inited)
        return false;

    assert(dir_thread);
    return dir_thread->sendMessageWithTimeout(
               dir_thread->GSNSender(
                   dir_trans_srvpara.su_ipaddr,
                   dir_trans_srvpara.su_port,
                   dir_trans_srvpara.sam_ipaddr,
                   dir_trans_srvpara.sam_port
               ),
               buf,
               buflen,
               timeout
           );
}

bool CDirectTranSrv::SendToSamWithNoResponse(
    unsigned char *buf,
    unsigned buflen,
    unsigned timeout
) const
{
    if (!sam_or_smp_inited)
        return false;

    return dir_thread->SendPacketNoResponse(
               dir_thread->GSNSender(
                   dir_trans_srvpara.su_ipaddr,
                   dir_trans_srvpara.su_port,
                   dir_trans_srvpara.sam_ipaddr,
                   dir_trans_srvpara.sam_port
               ),
               buf,
               buflen,
               timeout
           );
}

bool CDirectTranSrv::SendToSmp(
    unsigned char *buf,
    unsigned buflen,
    unsigned timeout
) const
{
    if (!sam_or_smp_inited)
        return false;

    assert(dir_thread);
    return dir_thread->sendMessageWithTimeout(
               dir_thread->GSNSender(
                   dir_smp_para.su_ipaddr,
                   dir_smp_para.su_port,
                   dir_smp_para.smp_ipaddr,
                   dir_smp_para.smp_port
               ),
               buf,
               buflen,
               timeout
           );
}

bool CDirectTranSrv::SendToSmpWithNoResponse(
    unsigned char *buf,
    unsigned buflen,
    unsigned timeout
) const
{
    if (!sam_or_smp_inited)
        return false;

    return dir_thread->SendPacketNoResponse(
               dir_thread->GSNSender(
                   dir_smp_para.su_ipaddr,
                   dir_smp_para.su_port,
                   dir_smp_para.smp_ipaddr,
                   dir_smp_para.smp_port
               ),
               buf,
               buflen,
               timeout
           );
}
