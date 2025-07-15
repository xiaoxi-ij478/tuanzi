#include "all.h"
#include "dnsquery.h"
#include "netutil.h"
#include "global.h"
#include "cmdutil.h"
#include "threadutil.h"
#include "proxydetectthread.h"

CProxyDetectThread::CProxyDetectThread() :
    isproser(),
    adapter_name(),
    hostmac(),
    ipaddr(),
    thread_key(),
    mtype(-1),
    kind(),
    started(),
    timerid()
{}

CProxyDetectThread::~CProxyDetectThread()
{
    if (timerid)
        KillTimer(timerid);
}

bool CProxyDetectThread::InitInstance()
{
    char errbuf[128] = {};
    g_logFile_proxy.AppendText("CProxyDetectThread::InitInstance");
    started = false;

    if (CDNSQuery::StartQueryThread(errbuf)) {
        SetClassName("CProxyDetectThread");
        return CLnxThread::InitInstance();
    }

    g_logFile_proxy.AppendText("start dns query thread error:%s\n", errbuf);
    return false;
}

void CProxyDetectThread::DispathMessage(struct LNXMSG *msg)
{
    int detect_ret = 0;

    switch (msg->mtype) {
        case START_DETECT_MTYPE:
            started = true;
            isproser.Start(adapter_name, &hostmac, kind);

            if (timerid)
                KillTimer(timerid);

            timerid = SetTimer(PROXY_DETECT_TIMER_MTYPE, 300000);
            break;

        case STOP_DETECT_MTYPE:
            started = false;

            if (timerid)
                KillTimer(timerid);

            timerid = 0;
            break;

        case RECV_PACKET_RETURN_MTYPE:
            if (!started) {
                reinterpret_cast<char *>(msg->arg2);
                break;
            }

            detect_ret =
                isproser.Detect(
                    reinterpret_cast<char *>(msg->arg2),
                    msg->arg1
                );

            if (detect_ret > 0) {
                g_logFile_proxy.AppendText("检测到代理服务:%d", detect_ret);
                rj_printf_debug("Detect proxy %d\n", detect_ret);
                started = false;
                ::PostThreadMessage(thread_key, mtype, detect_ret, 0);
            }

            delete[] reinterpret_cast<char *>(msg->arg2);
            break;
    }
}

void CProxyDetectThread::OnTimer(int tflag) const
{
    if (OnTimerEnter(tflag)) {
        if (tflag == PROXY_DETECT_TIMER_MTYPE)
            isproser.OnTimer(0, 0);

        OnTimerLeave(tflag);

    } else
        g_logSystem.AppendText(
            "CProxyDetectThread::OnTimer(timerFlag=%d),return",
            tflag
        );
}

bool CProxyDetectThread::ExitInstance()
{
    g_logFile_proxy.AppendText("CProxyDetectThread::ExitInstance");
    CDNSQuery::StopQueryThread();

    if (timerid) {
        KillTimer(timerid);
        timerid = 0;
    }

    return CLnxThread::ExitInstance();
}

bool CProxyDetectThread::GetFakeInfo(
    in_addr_t *ipaddr,
    struct ether_addr *macaddr
) const
{
    return isproser.GetFakeMacInfo(ipaddr, macaddr);
}

bool CProxyDetectThread::StartDetect(
    const char *adapter_name_l,
    key_t thread_key_l,
    unsigned mtype_l,
    int kind_l,
    char *errbuf
)
{
    struct DHCPIPInfo dhcp_ipinfo = {};

    if (strlen(src) > 512) {
        if (errbuf)
            strcpy(errbuf, "para adapter name is too long");

        return false;
    }

    CtrlThread->GetDHCPInfoParam(dhcp_ipinfo);
    strcpy(adapter_name, adapter_name_l);
    thread_key = thread_key_l;
    mtype = mtype_l;
    kind = kind_l;
    ipaddr = dhcp_ipinfo.ip4_ipaddr;
    hostmac = dhcp_ipinfo.adapter_mac;

    if (PostThreadMessage(START_DETECT_MTYPE, 0, 0))
        return true;

    if (errbuf)
        strcpy(errbuf, "Post start message failure");

    return false;
}

bool CProxyDetectThread::StopDetect()
{
    return PostThreadMessage(STOP_DETECT_MTYPE, 0, 0);
}
