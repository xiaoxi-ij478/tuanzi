#include "global.h"
#include "threadutil.h"
#include "clientcenterpeermanager.h"

CClientCenterPeerManager *CClientCenterPeerManager::instance = nullptr;

CClientCenterPeerManager::CClientCenterPeerManager() :
    timerid(),
    timer_interval(1),
    control_center_info(),
    thread_key(),
    upgrade_type()
{
    SetClassName("CClientCenterPeerManager");
}

CClientCenterPeerManager::~CClientCenterPeerManager()
{}

bool CClientCenterPeerManager::Start(key_t thread_key_l)
{
    if (instance)
        return false;

    instance = new CClientCenterPeerManager;
    instance->dont_know_always_false = false;
    instance->thread_key = thread_key_l;

    if (instance->CreateThread(nullptr, false)) {
        if (instance->StartThread()) {
            g_logContextControl.AppendText("StartThread，启动线程失败");
            instance->SafeExitThread(10000);
            instance = nullptr;
            return true;

        } else {
            g_logContextControl.AppendText("StartThread，创建线程成功");
            return true;
        }

    } else {
        delete instance;
        instance = nullptr;
        g_logContextControl.AppendText("CreateThread，创建线程失败");
        return false;
    }
}

bool CClientCenterPeerManager::StartConnect(
    struct _START_CENTERCONTROL_START_ *info
)
{
    struct _START_CENTERCONTROL_START_ *tmpinfo = nullptr;

    if (!instance) {
        g_logContextControl.AppendText("StartConnect: thread is not running.");
        return false;
    }

    tmpinfo = new struct _START_CENTERCONTROL_START_;

    if (tmpinfo->ipv4 != info->ipv4)
        *tmpinfo = *info;

    g_logContextControl.AppendText("Thread id:%u", instance->thread_id);

    if (
        ::PostThreadMessage(
            instance->thread_id,
            START_CONNECT_MTYPE,
            reinterpret_cast<unsigned long>(tmpinfo),
            nullptr
        )
    ) {
        g_logContextControl.AppendText("StartConnect: post message success.");
        return true;
    }

    g_logContextControl.AppendText("StartConnect: post message failed.");
    delete tmpinfo;
    return false;
}

void CClientCenterPeerManager::Stop()
{
    if (!instance)
        return;

    instance->SafeExitThread(10000);
    instance = nullptr;
}

bool CClientCenterPeerManager::StopConnect()
{
    if (!instance) {
        g_logContextControl.AppendText("StartConnect: thread is not running.");
        return true;
    }

    if (instance->PostThreadMessage(STOP_CONNECT_MTYPE, 0, nullptr))
        return true;

    else {
        g_logContextControl.AppendText("StopConnect: post message failed.");
        return false;
    }
}

unsigned int CClientCenterPeerManager::getUpgradeType()
{
    return upgrade_type;
}

bool CClientCenterPeerManager::DispathMessage(struct LNXMSG *msg)
{
    g_logContextControl.AppendText("DispathMessage %d", msg->mtype);

    switch (msg->mtype) {
        case START_CONNECT_MTYPE:
            OnStart(msg->buflen, msg->buf);
            break;

        case STOP_CONNECT_MTYPE:
            OnStop(msg->buflen, msg->buf);
            break;

        case ON_TIMER_MTYPE:
            OnTimer(msg->buflen, msg->buf);
            break;
    }

    return false;
}

void CClientCenterPeerManager::OnTimer(int tflag) const
{
    if (OnTimerEnter(tflag)) {
        if (!PostThreadMessage(ON_TIMER_MTYPE, tflag, -1))
            OnTimerLeave(tflag);

    } else
        g_logContextControl.AppendText(
            "CClientCenterPeerManager::OnTimer(timerFlag=%d),return",
            tflag
        );
}

bool CClientCenterPeerManager::ExitInstance()
{
    OnStop();
    return CLnxThread::ExitInstance();
}

bool CClientCenterPeerManager::InitInstance()
{
    g_logContextControl.AppendText("InitInstance");
    g_logContextControl.AppendText("InitInstance Thread id:%u", thread_id);
    return true;
}

void CClientCenterPeerManager::OnStart(unsigned long buflen, void *buf)
{
    struct _START_CENTERCONTROL_START_ *info =
            reinterpret_cast<struct _START_CENTERCONTROL_START_ *>(buflen);
    g_logContextControl.AppendText("Start to connect client center");
    OnStop(0, nullptr);

    if (!buflen)
        return;

    if (&control_center_info != info)
        *control_center_info = *info;

    delete info;
    control_center_info.product = 0x3000000;

    if (
        !control_center_info.domain.empty() &&
        control_center_info.ipv4 &&
        control_center_info.port
    )
        ProcessConnect();
}

void CClientCenterPeerManager::OnStop(unsigned long buflen, void *buf)
{
    g_logContextControl.AppendText("Stop to connect client center",);

    if (!timerid)
        return;

    KillTimer(&timerid);
    timerid = 0;
}

void CClientCenterPeerManager::OnTimer(unsigned long buflen, void *buf)
{
    g_logContextControl.AppendText(
        "CContextControlThread::OnTimer nIDEvent = %d",
        buflen
    );

    if (buflen == 1) {
        g_logContextControl.AppendText("CClientCenterPeerManager::OnTimer，进来了");
        ProcessConnect();
    }

    OnTimerLeave(buflen);
    return;
}

int CClientCenterPeerManager::ParseResult(const char *result)
{
}

void CClientCenterPeerManager::ProcessConnect()
{
}

std::string CClientCenterPeerManager::EnCodeStr(std::string str)
{
}

std::string CClientCenterPeerManager::DeCodeStr(std::string str)
{
}
