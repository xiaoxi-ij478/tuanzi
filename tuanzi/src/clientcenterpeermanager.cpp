#include "global.h"
#include "threadutil.h"
#include "xmlparser.h"
#include "httpconnection.h"
#include "confupdateutilinstance.h"
#include "clientcenterpeermanager.h"

CClientCenterPeerManager *CClientCenterPeerManager::instance = nullptr;

CClientCenterPeerManager::CClientCenterPeerManager() :
    process_connect_timerid(),
    process_connect_timer_interval(1),
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

unsigned CClientCenterPeerManager::getUpgradeType() const
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
        if (!PostThreadMessage(ON_TIMER_MTYPE, tflag, reinterpret_cast<void *>(-1)))
            OnTimerLeave(tflag);

    } else
        g_logContextControl.AppendText(
            "CClientCenterPeerManager::OnTimer(timerFlag=%d),return",
            tflag
        );
}

bool CClientCenterPeerManager::ExitInstance()
{
    OnStop(0, nullptr);
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
        control_center_info = *info;

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
    g_logContextControl.AppendText("Stop to connect client center");

    if (!process_connect_timerid)
        return;

    KillTimer(process_connect_timerid);
    process_connect_timerid = 0;
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
    int ret = 0;
    XML_PARSER xml_parser;
    std::string upgrade_path;
    std::string upgrade_full_url;
    std::ostringstream upgrade_full_url_oss;
    char *upgrade_full_url_buf = nullptr;

    if (ret = CConfUpdateUtilInstance::Instance().UpdateConfigure(result))
        g_logContextControl.AppendText(
            "Failed to update configure, error=%d .", ret
        );

    if (!xml_parser.Load_XML_String(result)) {
        delete[] result;
        g_logContextControl.AppendText(
            "CClientCenterPeerManager::ParseResult，解析时，文档出错了。"
        );
        return -1;
    }

    delete[] result;
    xml_parser.Go_to_Root();

    if (!xml_parser.Go_to_Child("UpgradeUrl"))
        return 0;

    g_logContextControl.AppendText(
        "CClientCenterPeerManager::ParseResult，解析到需要升级。"
    );
    xml_parser.GetAttributeValue("url", upgrade_path);
    upgrade_type = xml_parser.GetAttributeValueInt("upgradeType");
    g_logContextControl.AppendText(
        "CClientCenterPeerManager::ParseResult，升级的url 路径是:%s。",
        upgrade_path.c_str()
    );

    if (upgrade_path.empty())
        return 0;

    upgrade_full_url_oss << "http://" << control_center_info.domain
                         << ':' << control_center_info.port
                         << '/' << upgrade_path;
    upgrade_full_url = upgrade_full_url_oss.str();
    g_logContextControl.AppendText(
        "CClientCenterPeerManager::ParseResult，升级的全部url是:%s。",
        upgrade_full_url.c_str()
    );
    upgrade_full_url_buf = new char[upgrade_full_url.length() + 1];
    upgrade_full_url.copy(upgrade_full_url_buf, upgrade_full_url.length());
    ::PostThreadMessage(thread_key, NOTIFY_UPGRADE_MTYPE, -1, upgrade_full_url_buf);
    return 0;
}

void CClientCenterPeerManager::ProcessConnect()
{
    std::ostringstream url_oss;
    std::ostringstream query_param_oss;
    std::string url;
    unsigned timer_interval_l = 0;
    unsigned http_length = 0;
    unsigned http_read = 0;
    unsigned char *buf = nullptr;
    CHttpConnection http_client;
    url_oss << "http://" << control_center_info.domain
            << ':' << control_center_info.port
            << "/rjsdcctrl?";
    query_param_oss << "mac=" << control_center_info.mac
                    << "&ipv4=" << control_center_info.ipv4
                    << "&ipv61=" << control_center_info.ipv6[0]
                    << "&ipv62=" << control_center_info.ipv6[1]
                    << "&ipv63=" << control_center_info.ipv6[2]
                    << "&ipv64=" << control_center_info.ipv6[3]
                    << "&product=" << control_center_info.product
                    << "&mainver=" << control_center_info.major_ver
                    << "&subver=" << control_center_info.minor_ver;
    url_oss << EnCodeStr(query_param_oss.str());
    url = url_oss.str();
    g_logContextControl.AppendText(
        "CClientCenterPeerManager::ProcessConnect，构造的url是：%s",
        url.c_str()
    );
    http_client.setTimeout(10);
    http_client.addRequestHeader(
        "Accept: text/*\r\n"
        "User-Agent: HttpCall\r\n"
        "Accept-Language: en-us\r\n"
    );

    if (http_client.httpConnect(url.c_str()) == -1) {
        g_logContextControl.AppendText(
            "connect  error:%s.",
            http_client.getErrorText().c_str()
        );
        goto set_timer;
    }

    if ((http_length = http_client.getLength()) <= 0) {
        g_logContextControl.AppendText("getLength failed");
        goto http_close;
    }

    if (http_length > 1 * 1024 * 1024) { // 1G
        g_logContextControl.AppendText(
            "The content length(%d) is too big, ignore!",
            http_length);
        goto http_close;
    }

    buf = new unsigned char[http_length + 1];

    if ((http_read = http_client.httpRead(buf, http_length)) == http_length) {
        buf[http_length] = 0;
        goto http_close;
    }

    g_logContextControl.AppendText(
        "read failed, bytes:%d, len=%d",
        http_read,
        http_length
    );
    delete buf;
    buf = nullptr;
http_close:
    http_client.httpClose();

    if (!buf)
        goto set_timer;

    if (process_connect_timerid) {
        KillTimer(process_connect_timerid);
        process_connect_timerid = 0;
    }

    g_logContextControl.AppendText(
        "CClientCenterPeerManager::ProcessConnect，http请求返回成功"
    );
    g_logContextControl.AppendText("%s", buf);
    ParseResult(reinterpret_cast<char *>(buf));
    return;
set_timer:

    if (process_connect_timerid)
        return;

    srand(time(nullptr));
    timer_interval_l = rand() % 60 + 60;
    process_connect_timer_interval = timer_interval_l;
    process_connect_timerid = SetTimer(1, 1000 * timer_interval_l);
}

std::string CClientCenterPeerManager::EnCodeStr(const std::string &str)
{
    std::ostringstream oss;
    oss << std::hex << std::setfill('0');

    for (const unsigned char i : str)
        if (isalnum(i))
            oss << i;

        else if (i == ' ')
            oss << '+';

        else
            oss << '%'
                << std::setw(2) << static_cast<unsigned>(i)
                << std::setw(1);

    return oss.str();
}

std::string CClientCenterPeerManager::DeCodeStr(const std::string &str)
{
    std::ostringstream oss;

    for (auto it = str.cbegin(); it != str.cend(); it++) {
        if (*it == '%') {
            oss << static_cast<unsigned char>
                (std::stoul(str.substr(++it - str.cbegin(), 2)));
            it++;

        } else if (*it == '+')
            oss << ' ';

        else
            oss << *it;
    }

    return oss.str();
}
