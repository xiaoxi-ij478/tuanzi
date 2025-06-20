#include "all.h"
#include "cmdutil.h"
#include "netutil.h"
#include "threadutil.h"
#include "adapterdetectthread.h"

#define POST_TO_CONTROL_THREAD(mtype) \
    ::PostThreadMessage( \
                         control_thread_key, \
                         control_thread_msgid, \
                         mtype, \
                         nullptr \
                       )

CAdapterDetectThread::CAdapterDetectThread() :
    CLnxThread(),
    control_thread_key(),
    control_thread_msgid(),
    ipaddr(),
    macaddr(),
    disallow_multi_nic_ip(),
    proxy_detect_timerid(),
    nic_state_detect_timerid(),
    socket_fd(-1),
    status(ADAPTER_INVALID)
{
    SetClassName("CAdapterDetectThread");
}

CAdapterDetectThread::~CAdapterDetectThread()
{}

bool CAdapterDetectThread::StartDetect(
    const char *nic_name_l,
    const struct ether_addr *macaddr_l,
    struct in_addr ipaddr_l,
    key_t thread_key_l,
    int msgid,
    bool disallow_multi_nic_ip_l,
    char *errmsg
) const
{
    struct DetectNICInfo *info = nullptr;

    if (strlen(nic_name_l) > MAX_NIC_NAME_LEN) {
        if (errmsg)
            strcpy(errmsg, "para adapter name is too long");

        return false;
    }

    info = new struct DetectNICInfo;
    strcpy(info->nic_name, nic_name_l);
    info->macaddr = *macaddr_l;
    info->ipaddr = ipaddr_l;
    info->thread_key = thread_key_l;
    info->msgid = msgid;
    info->disallow_multi_nic_ip = disallow_multi_nic_ip_l;
    g_log_Wireless.AppendText(
        "CAdapterDetectThread thread id:%u; msg id:%d; adapter name:%s ip:%u",
        thread_key_l, msgid, nic_name_l, ipaddr.s_addr
    );

    if (
        !PostThreadMessage(
            START_DETECT_MTYPE,
            reinterpret_cast<unsigned long>(info),
            nullptr
        )
    ) {
        delete info;
        return false;
    }

    return true;
}

bool CAdapterDetectThread::StopDetect(unsigned flag) const
{
    if (flag & 3) {
        if (PostThreadMessage(STOP_DETECT_MTYPE, flag, nullptr))
            return true;

        g_log_Wireless.AppendText("%s Post message failed", "StopDetect");
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
            OnStartDetect(msg->buflen, msg->buf);
            break;

        case STOP_DETECT_MTYPE:
            OnStopDetect(msg->buflen, msg->buf);
            break;

        case ON_TIMER_MTYPE:
            OnTimer(msg->buflen, msg->buf);
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

void CAdapterDetectThread::OnTimer(int tflag) const
{
    if (OnTimerEnter(tflag)) {
        if (!PostThreadMessage(ON_TIMER_MTYPE, tflag, reinterpret_cast<void *>(-1)))
            OnTimerLeave(tflag);

    } else
        g_logSystem.AppendText(
            "CAdapterDetectThread::OnTimer(timerFlag=%d),return",
            tflag
        );
}

void CAdapterDetectThread::MultipleAdaptesOrIPCheck() const
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

            if (
                memcmp(
                    &macaddr,
                    &cur_info->hwaddr,
                    sizeof(macaddr)
                )
            ) {
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
            ifr.ifr_data = reinterpret_cast<caddr_t>(&evalue);

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

void CAdapterDetectThread::OnStartDetect(
    unsigned long buflen,
    [[maybe_unused]] void *buf
)
{
    struct DetectNICInfo *info = reinterpret_cast<struct DetectNICInfo *>(buflen);

    if (proxy_detect_timerid)
        KillTimer(proxy_detect_timerid);

    if (nic_state_detect_timerid)
        KillTimer(nic_state_detect_timerid);

    disallow_multi_nic_ip = info->disallow_multi_nic_ip;
    strcpy(nic_name, info->nic_name);
    macaddr = info->macaddr;
    ipaddr = info->ipaddr;
    control_thread_key = info->thread_key;
    control_thread_msgid = info->msgid;
    status = ADAPTER_UP;
    proxy_detect_timerid = SetTimer(nullptr, 0x71, 60000, nullptr);
    nic_state_detect_timerid = SetTimer(nullptr, 0x72, 1000, nullptr);
    g_log_Wireless.AppendText(
        "%s timer nics IP=%u; timer nic state=%u",
        "OnStartDetect",
        proxy_detect_timerid,
        nic_state_detect_timerid
    );
}

void CAdapterDetectThread::OnStopDetect(
    unsigned long buflen,
    [[maybe_unused]] void *buf
)
{
    if (buflen & STOP_PROXY_DETECT_TIMER_FLAG) {
        g_log_Wireless.AppendText("adapter detect thread stop proxy detect");
        KillTimer(proxy_detect_timerid);
    }

    if (buflen & STOP_NIC_STATE_DETECT_TIMER_FLAG) {
        g_log_Wireless.AppendText("adapter detect thread stop nic state detect");
        KillTimer(proxy_detect_timerid);
    }
}

void CAdapterDetectThread::OnTimer(
    unsigned long buflen,
    [[maybe_unused]] void *buf
)
{
    switch (buflen) {
        case PROXY_DETECT_TIMER_MTYPE:
            if (proxy_detect_timerid)
                MultipleAdaptesOrIPCheck();

            break;

        case NIC_STATE_DETECT_TIMER_MTYPE:
            if (nic_state_detect_timerid)
                adapter_state_check();

            break;
    }

    OnTimerLeave(buflen);
}

void CAdapterDetectThread::adapter_state_check()
{
    struct ifreq ifr = {};
    struct ethtool_value evalue = {};
    strncpy(ifr.ifr_name, nic_name, IFNAMSIZ - 1);
    evalue.cmd = ETHTOOL_GLINK;
    ifr.ifr_data = reinterpret_cast<caddr_t>(&evalue);

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
