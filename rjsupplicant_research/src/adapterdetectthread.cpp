#include "cmdutil.h"
#include "netutil.h"
#include "threadutil.h"
#include "global.h"
#include "adapterdetectthread.h"

#define POST_TO_CONTROL_THREAD(mtype) \
    ::PostThreadMessage( \
                         control_thread_key, \
                         control_thread_msgid, \
                         reinterpret_cast<void *>(mtype), \
                         0 \
                       )


CAdapterDetectThread::CAdapterDetectThread() :
    CLnxThread(),
    proxy_detect_timerid(0), nic_state_detect_timerid(0),
    socket_fd(-1), status(ADAPTER_INVALID)
{
    SetClassName("CAdapterDetectThread");
}

CAdapterDetectThread::~CAdapterDetectThread()
{ }

bool CAdapterDetectThread::StartDetect(
    const char *nic_name,
    const unsigned char macaddr[6],
    struct in_addr ipaddr,
    pthread_t thread_key,
    int msgid,
    bool disallow_multi_nic_ip,
    char *errmsg
)
{
    struct DetectNICInfo *info = nullptr;

    if (strlen(nic_name) > MAX_NIC_NAME_LEN) {
        if (errmsg)
            strcpy(errmsg, "para adapter name is too long");

        return false;
    }

    info = new struct DetectNICInfo;
    strcpy(info->nic_name, nic_name);
    memcpy(info->macaddr, macaddr, sizeof(info->macaddr));
    info->ipaddr = ipaddr;
    info->thread_key = thread_key;
    info->msgid = msgid;
    info->disallow_multi_nic_ip = disallow_multi_nic_ip;
    g_log_Wireless.AppendText(
        "CAdapterDetectThread thread id:%u; msg id:%d; adapter name:%s ip:%u",
        thread_key, msgid, nic_name, ipaddr.s_addr
    );

    if (!PostThreadMessage(
                START_DETECT_MTYPE,
                info,
                sizeof(struct DetectNICInfo)
            )) {
        delete info;
        return false;
    }

    return true;
}

bool CAdapterDetectThread::StopDetect(unsigned int flag)
{
    if (flag & 3) {
        if (PostThreadMessage(
                    STOP_DETECT_MTYPE,
                    reinterpret_cast<void *>(flag),
                    0
                ))
            return true;

        g_log_Wireless.AppendText("%s Post message failed", __func__);
        return false;
    }

    g_log_Wireless.AppendText(
        "CAdapterDetectThread::StopDetect para mask(%d) error",
        flag
    );
    return false;
}

bool CAdapterDetectThread::DispathMessage(struct LNXMSG *msg)
{
    switch (msg->mtype) {
        case START_DETECT_MTYPE:
            OnStartDetect(msg->buf);
            break;

        case STOP_DETECT_MTYPE:
            OnStopDetect(msg->buf);
            break;

        case ON_TIMER_MTYPE:
            OnTimer(msg->buf);
            break;
    }

    return false;
}

bool CAdapterDetectThread::ExitInstance()
{
    KillTimer(proxy_detect_timerid);
    KillTimer(nic_state_detect_timerid);

    if (socket_fd >= 0)
        close(socket_fd);

    socket_fd = -1;
    return CLnxThread::ExitInstance();
}

bool CAdapterDetectThread::InitInstance()
{
    if ((socket_fd = socket(AF_INET, SOCK_DGRAM, 0)) >= 0)
        return CLnxThread::InitInstance();

    rj_printf_debug("socket error");
    return false;
}

void CAdapterDetectThread::OnTimer(int tflag)
{
    if (OnTimerEnter(tflag)) {
        if (!PostThreadMessage(ON_TIMER_MTYPE, reinterpret_cast<void *>(tflag), -1))
            OnTimerLeave(tflag);

    } else
        g_logSystem.AppendText("CAdapterDetectThread::OnTimer(timerFlag=%d),return",
                               tflag);
}

void CAdapterDetectThread::MultipleAdaptesOrIPCheck()
{
    struct ifreq ifr;
    struct ethtool_value evalue;
    struct NICINFO *nic_infos = get_nics_info(nullptr);
    memset(&ifr, 0, sizeof(ifr));
    memset(&evalue, 0, sizeof(evalue));

    if (!nic_infos)
        return;

    for (
        struct NICINFO *cur_info = nic_infos;
        cur_info;
        cur_info = cur_info->next
    ) {
        if (!strcmp(nic_name, cur_info->ifname)) {
            g_log_Wireless.AppendText("nic name:%s", nic_name);

            if (memcmp(macaddr, cur_info->hwaddr, sizeof(macaddr))) {
                g_log_Wireless.AppendText("mac chaged\n");
                POST_TO_CONTROL_THREAD(MAC_CHANGED_MTYPE);
                break;
            }

            g_log_Wireless.AppendText("ipv4 count:%d", cur_info->ipaddr_count);

            for (
                struct NICINFO::IPAddrNode *cip = cur_info->ipaddrs;
                cip;
                cip = cip->next
            )
                g_log_Wireless.AppendText(
                    "ip:%d.%d.%d.%d",
                    cip->ipaddr.s_addr >> 24,
                    cip->ipaddr.s_addr >> 16 & 0xff,
                    cip->ipaddr.s_addr >> 8 & 0xff,
                    cip->ipaddr.s_addr & 0xff
                );

            if (cur_info->ipaddr_count > 1 && disallow_multi_nic_ip) {
                g_log_Wireless.AppendText("multiple ips\n");
                POST_TO_CONTROL_THREAD(MULTIPLE_IP_MTYPE);
                break;
            }

            if (!cur_info->ipaddrs) {
                g_log_Wireless.AppendText("ip chaged - no ip\n");
                POST_TO_CONTROL_THREAD(IP_CHANGED_MTYPE);
                break;
            }

            if (ipaddr.s_addr != cur_info->ipaddrs->ipaddr.s_addr) {
                g_log_Wireless.AppendText("ip chaged\n");
                POST_TO_CONTROL_THREAD(IP_CHANGED_MTYPE);
                break;
            }

        } else if (disallow_multi_nic_ip) {
            g_log_Wireless.AppendText("Other nic name:%s", cur_info->ifname);
            strncpy(ifr.ifr_name, cur_info->ifname, IFNAMSIZ - 1);
            evalue.cmd = ETHTOOL_GLINK;
            ifr.ifr_data = reinterpret_cast<__caddr_t>(&evalue);

            if (ioctl(socket_fd, SIOCGIFFLAGS, &ifr) < 0) {
                if (ioctl(socket_fd, SIOCETHTOOL, &ifr) >= 0 && evalue.data == 1) {
                    g_log_Wireless.AppendText("multiple adapters\n");
                    POST_TO_CONTROL_THREAD(MULTIPLE_ADAPTER_MTYPE);
                    break;
                }

            } else {
                if (
                    ifr.ifr_flags & IFF_UP &&
                    ifr.ifr_flags & IFF_RUNNING
                ) {
                    g_log_Wireless.AppendText(
                        "multiple adapters flags:%4x",
                        ifr.ifr_flags
                    );
                    POST_TO_CONTROL_THREAD(MULTIPLE_ADAPTER_MTYPE);
                    break;
                }

                g_log_Wireless.AppendText(
                    "SIOCGIFFLAGS flags:%4x",
                    ifr.ifr_flags
                );
            }
        }
    }

    free_nics_info(nic_infos);
}

void CAdapterDetectThread::OnStartDetect(void *arg)
{
    DetectNICInfo *info = static_cast<DetectNICInfo *>(arg);

    if (proxy_detect_timerid)
        KillTimer(proxy_detect_timerid);

    if (nic_state_detect_timerid)
        KillTimer(nic_state_detect_timerid);

    disallow_multi_nic_ip = info->disallow_multi_nic_ip;
    strcpy(nic_name, info->nic_name);
    memcpy(macaddr, info->macaddr, sizeof(macaddr));
    ipaddr = info->ipaddr;
    control_thread_key = info->thread_key;
    control_thread_msgid = info->msgid;
    status = ADAPTER_UP;
    proxy_detect_timerid = SetTimer(nullptr, 0x71, 60000, nullptr);
    nic_state_detect_timerid = SetTimer(nullptr, 0x72, 1000, nullptr);
    g_log_Wireless.AppendText(
        "%s timer nics IP=%u; timer nic state=%u",
        __func__,
        proxy_detect_timerid,
        nic_state_detect_timerid
    );
}

void CAdapterDetectThread::OnStopDetect(void *arg)
{
    unsigned long flag = reinterpret_cast<unsigned long>(arg);

    if (flag & STOP_PROXY_DETECT_TIMER_FLAG) {
        g_log_Wireless.AppendText("adapter detect thread stop proxy detect");
        KillTimer(proxy_detect_timerid);
    }

    if (flag & STOP_NIC_STATE_DETECT_TIMER_FLAG) {
        g_log_Wireless.AppendText("adapter detect thread stop nic state detect");
        KillTimer(proxy_detect_timerid);
    }
}

void CAdapterDetectThread::OnTimer(void *arg)
{
    unsigned long type = reinterpret_cast<unsigned long>(arg);

    switch (type) {
        case PROXY_DETECT_TIMER_MTYPE:
            if (proxy_detect_timerid)
                MultipleAdaptesOrIPCheck();

            break;

        case NIC_STATE_DETECT_TIMER_MTYPE:
            if (nic_state_detect_timerid)
                adapter_state_check();

            break;
    }

    OnTimerLeave(type);
}

void CAdapterDetectThread::adapter_state_check()
{
    struct ifreq ifr = { 0 };
    struct ethtool_value evalue = { 0 };
    strncpy(ifr.ifr_name, nic_name, IFNAMSIZ - 1);
    evalue.cmd = ETHTOOL_GLINK;
    ifr.ifr_data = reinterpret_cast<__caddr_t>(&evalue);

    if (ioctl(socket_fd, SIOCGIFFLAGS, &ifr) < 0) {
        g_log_Wireless.AppendText("ioctl SIOCGIFFLAGS error:%s", strerror(errno));

        if (ioctl(socket_fd, SIOCETHTOOL, &ifr) < 0) {
            g_log_Wireless.AppendText(
                "ioctl SIOCETHTOOL error(%d):%s",
                errno, strerror(errno)
            );

            if (errno == ENODEV && status != ADAPTER_ERROR) {
                status = ADAPTER_ERROR;
                POST_TO_CONTROL_THREAD(ADAPTER_ERROR_REPORT_MTYPE);
            }

        } else {
            g_log_Wireless.AppendText("edata.data %u\n", evalue.data);

            if (evalue.data == 1) {
                if (status != ADAPTER_UP) {
                    status = ADAPTER_UP;
                    POST_TO_CONTROL_THREAD(ADAPTER_UP_REPORT_MTYPE);
                    g_log_Wireless.AppendText("SIOCETHTOOL ADAPTER_LINK up\n");
                }

            } else if (status != ADAPTER_DOWN) {
                status = ADAPTER_DOWN;
                POST_TO_CONTROL_THREAD(ADAPTER_DOWN_REPORT_MTYPE);
                g_log_Wireless.AppendText("SIOCETHTOOL ADAPTER_LINK down\n");
            }
        }

    } else if (ifr.ifr_flags & IFF_UP) {
        if (status == ADAPTER_DISABLE) {
            status = ADAPTER_ENABLE;
            POST_TO_CONTROL_THREAD(ADAPTER_ENABLE_REPORT_MTYPE);
            g_log_Wireless.AppendText("ADAPTER_LINK enable\n");
        }

        if (ifr.ifr_flags & IFF_RUNNING) {
            if (status != ADAPTER_UP) {
                status = ADAPTER_UP;
                POST_TO_CONTROL_THREAD(ADAPTER_UP_REPORT_MTYPE);
                g_log_Wireless.AppendText("ADAPTER_LINK up\n");
            }

        } else if (status != ADAPTER_DOWN && status != ADAPTER_ENABLE) {
            status = ADAPTER_DOWN;
            POST_TO_CONTROL_THREAD(ADAPTER_DOWN_REPORT_MTYPE);
            g_log_Wireless.AppendText("ADAPTER_LINK down\n");
        }

    } else if (status != ADAPTER_DISABLE) {
        status = ADAPTER_DISABLE;
        POST_TO_CONTROL_THREAD(ADAPTER_DISABLE_REPORT_MTYPE);
        g_log_Wireless.AppendText("ADAPTER_LINK disable\n");
    }
}
