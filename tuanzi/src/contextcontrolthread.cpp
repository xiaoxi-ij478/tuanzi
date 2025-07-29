#include "all.h"
#include "cmdutil.h"
#include "msgutil.h"
#include "diskutil.h"
#include "timeutil.h"
#include "netutil.h"
#include "eapolutil.h"
#include "encodeutil.h"
#include "threadutil.h"
#include "sysutil.h"
#include "psutil.h"
#include "util.h"
#include "mtypes.h"
#include "md5checksum.h"
#include "global.h"
#include "userconfig.h"
#include "changelanguage.h"
#include "clientcenterpeermanager.h"
#include "passwordmodifier.h"
#include "suconfigfile.h"
#include "backoffreauthenticationmanager.h"
#include "rxpacketthread.h"
#include "proxydetectthread.h"
#include "adapterdetectthread.h"
#include "statemachinethread.h"
#include "directtransrv.h"
#include "sendpacketthread.h"
#include "helloprocessor.h"
#include "downloadthread.h"
#include "logfile.h"
#include "rgprivateproc.h"
#include "contextcontrolthread.h"

CContextControlThread::CContextControlThread() :
    auth_inited(),
    field_1D1(),
    wireless_signal(),
    field_1F0(),
    field_1F8(),
    read_packet_thread(),
    send_packet_thread(),
    machine_thread(),
    dir_tran_srv(),
    download_thread(),
    hello_processor(),
    adapter_detect_thread(),
    proxy_detect_thread(),
    configure_info(),
    field_400(),
    private_properties(),
    is_ruijie_nas(true),
    radius_server(9),
    dhcp_auth_step(),
    eap_type(),
    if_type(),
    bootp_timerid(),
    ip_offer_count(),
    field_4EC(),
    bootp_semaphore(),
    field_4F8(),
    direct_trans(),
    field_511(),
    sec_domain_name(),
    nic_in_use(),
    field_538(true),
    reconnect_fail(),
    connect_fail(),
    field_53B(),
    logoff_message(),
    success_info(),
    dstaddr(),
    start_dst_addr(),
    old_src_macaddr(),
    reconnect_timer(),
    field_570(),
    process_business_started(),
    field_1129(),
    field_112A(),
    diskid(),
    field_1138(),
    field_1139(),
    service_list_updated(),
    service_name(),
    has_auth_success(),
    field_1149(true),
    state(STATE_INVALID),
    field_1150(),
    wait_dhcp_auth_result_timerid(),
    open_sso_url_timer(),
    upgrade_url(),
    scan_completed(),
    field_11D0()
{
    configure_info.authparam_startnumber = 3;
    SetClassName("CContextControlThread");
}

CContextControlThread::~CContextControlThread()
{}

bool CContextControlThread::InitInstance()
{
    char diskid_buf[64] = {};
    unsigned i = 0;

    if (!CUserConfig::ReadSupplicantConf()) {
        ShowLocalMsg(
            CChangeLanguage::Instance().LoadString(2),
            CChangeLanguage::Instance().LoadString(246)
        );
    }

    if (getdiskid(diskid_buf, sizeof(diskid_buf))) {
        logFile.AppendText("get disk serial error");
        diskid.clear();

    } else {
        logFile.AppendText("get disk serial success:%s", diskid_buf);
        diskid = diskid_buf;
    }

    InitNICDevice();

    if (!InitInstance_InitAll()) {
        g_logSystem.AppendText(
            "CContextControlThread::InitInstance()-----InitInstance_InitAll failed"
        );
        return false;
    }

    for (i = 0; !InitNICDevice() && i < 3; i++)
        Sleep(5000);

    if (i == 3)
        ShowLocalMsg(CChangeLanguage::Instance().LoadString(1), "Error");

    private_properties.su_newest_ver = 0;
    private_properties.radius_type = 5;
    private_properties.account_info.clear();
    private_properties.persional_info.clear();
    private_properties.broadcast_str.clear();
    private_properties.fail_reason.clear();
    private_properties.su_upgrade_url.clear();
    private_properties.su_reauth_interv = 0;
    private_properties.radius_type = 0;
    private_properties.proxy_avoid = 0;
    private_properties.dialup_avoid = 0;
    private_properties.indicate_serv_ip = 0;
    private_properties.indicate_port = 0;
    private_properties.smp_ipaddr = 0;
    private_properties.proxy_dectect_kinds = 0;
    private_properties.hello_interv = 0;
    memset(
        private_properties.encrypt_key,
        0,
        sizeof(private_properties.encrypt_iv)
    );
    memset(
        private_properties.encrypt_iv,
        0,
        sizeof(private_properties.encrypt_iv)
    );
    private_properties.server_utc_time = 0;
    private_properties.svr_switch_result.clear();
    private_properties.services.clear();
    private_properties.user_login_url.clear();
    private_properties.msg_client_port = 80;
    private_properties.utrust_url.clear();
    private_properties.is_show_utrust_url = 1;
    private_properties.delay_second_show_utrust_url = 0;
    private_properties.parse_hello = 0;
    private_properties.direct_communication_highest_version_supported = true;
    private_properties.direct_comm_heartbeat_flags = 0;

    if (
        configure_info.public_authmode == "EAPWIRELESS" &&
        !configure_info.public_adapter.empty()
    )
        RefreshSignal(configure_info.public_adapter);

    g_log_Wireless.AppendText("Leaving CContextControlThread InitInstance\n");
    return CLnxThread::InitInstance();
}

bool CContextControlThread::ExitInstance()
{
    g_log_Wireless.AppendText("Enter CContextControlThread::ExitInstance");
    ExitExtance_ExitAll();
    adapter_detect_thread->SafeExitThread(10000);
    adapter_detect_thread = nullptr;
    g_log_Wireless.AppendText("CContextControlThread::ExitInstance adapte detect stop");
    proxy_detect_thread->SafeExitThread(10000);
    proxy_detect_thread = nullptr;
    CClientCenterPeerManager::Stop();
    SetEvent(&scan_completed, true);
    CloseHandle(&scan_completed);
    g_log_Wireless.AppendText("Leaving CContextControlThread::ExitInstance\n");
    return CLnxThread::ExitInstance();
}

void CContextControlThread::DispathMessage(struct LNXMSG *msg)
{
    switch (msg->mtype) {
            HANDLE_MTYPE(WM_OTHER_WANT_LOGOFF, OnOthersWantLogOff);
            HANDLE_MTYPE(SAY_HELLO_MTYPE, OnSayHello);
            HANDLE_MTYPE(CONNECT_NOTIFY_MTYPE, OnConnectNotify);
            HANDLE_MTYPE(STATE_MACHINE_RETURN_MTYPE, OnStateMachineReturn);
            HANDLE_MTYPE(PACKET_RETURN_MTYPE, OnPacketReturn);
            HANDLE_MTYPE(UPGRADE_RETURN_MTYPE, OnUpGradeReturn);
            HANDLE_MTYPE(DO_FORCE_OFFLINE_MTYPE, ONSAWantLogOff);
            HANDLE_MTYPE(SAY_SA_EVENT_MTYPE, OnSaySaEvent);
            HANDLE_MTYPE(SHOW_SA_MSG_MTYPE, OnShowSaMessage);
            HANDLE_MTYPE(REAUTH_MTYPE, OnSaWantReAuth);
            HANDLE_MTYPE(FORCE_OFFLINE_MTYPE, ONSAMWantLogOff);
            HANDLE_MTYPE(ADAPTER_STATE_MTYPE, OnAdaptersState);
            HANDLE_MTYPE(SIMULATE_SU_LOGOFF_MTYPE, OnSimulateLogoff);
            HANDLE_MTYPE(PATCH_LOGOFF_MTYPE, OnPatchLogoff);
            HANDLE_MTYPE(EAP_FAILURE_MTYPE, OnRecvFailure);
            HANDLE_MTYPE(COMPATIBLE_LOGOFF_MTYPE, OnLogoffWithUnknownReason);
            HANDLE_MTYPE(ANNOUNCE_DHCP_RESULT_MTYPE, OnRcvDHCPAuthResult);
            HANDLE_MTYPE(NOTIFY_UPGRADE_MTYPE, OnUpgradeClient);
            HANDLE_MTYPE(RECEIVED_PROXY_DETECT_RESULT_MTYPE, OnRcvProxyDetectResult);
            HANDLE_MTYPE(OPEN_SSO_URL_MTYPE, OnOpenSSOURL);
            HANDLE_MTYPE(ON_TIMER_MTYPE, OnTimer);
            HANDLE_MTYPE(CTRLTHREAD_START_STATE_MACHINE_MTYPE, OnStartMachine);
    }
}

void CContextControlThread::OnTimer(int tflag)
{
    if (OnTimerEnter(tflag)) {
        if (!PostThreadMessage(ON_TIMER_MTYPE, tflag, -1))
            OnTimerLeave(tflag);

    } else
        g_logSystem.AppendText(
            "CContextControlThread::OnTimer(timerFlag=%d),return",
            tflag
        );
}

unsigned CContextControlThread::Authenticate_InitAll()
{
    unsigned su_plat_init_ret = 0;
    char debug_file[256] = {};
    struct SuPlatformParam platform_param = {};

    if (
        IsWirelessAuth() &&
        get_nic_type(configure_info.public_adapter.c_str()) != ADAPTER_WIRELESS &&
        !auth_inited
    ) {
        strcpy(platform_param.driver_name, "wext");
        strcpy(platform_param.ifname, configure_info.public_adapter.c_str());
        sprintf(debug_file, "%slog/supf_debug.log", g_strAppPath.c_str());
//        platform_param.event_callback = supf_event_callback_fun;
        platform_param.debug_file =
#ifdef NDEBUG
            nullptr;
#else
            debug_file;
#endif // NDEBUG
        rj_printf_debug("Authenticate_InitAll before su_platform_init \n");
        su_plat_init_ret = su_platform_init(&platform_param);
        rj_printf_debug("Authenticate_InitAll after su_platform_init - END\n");

        if (su_plat_init_ret)
            return 3;

        auth_inited = true;
    }

    if (!read_packet_thread) {
        read_packet_thread = new CRxPacketThread;
        read_packet_thread->SetMainMsgID(GetMessageID());
        read_packet_thread->CreateThread(nullptr, false);

        if (
            !dir_tran_srv ||
            !dir_tran_srv->dir_thread ||
            !dir_tran_srv->dir_thread->udp_listenthread
        ) {
            g_logSystem.AppendText(
                "CContextControlThread::Authenticate_InitAll SetDirectMsgID FIALED "
            );
            return 4;
        }

        read_packet_thread->SetDirectMsgID(
            dir_tran_srv->dir_thread->udp_listenthread->GetMessageID()
        );
    }

    read_packet_thread->SetRxPacketAdapter(configure_info.public_adapter.c_str());

    if (read_packet_thread->StartRecvPacketThread() == -1)
        return 1;

    if (
        !send_packet_thread->SetSenderAdapter(
            CtrlThread->configure_info.public_adapter.c_str()
        )
    ) {
        read_packet_thread->StopRxPacketThread();
        g_logSystem.AppendText(
            "Authenticate_InitAll SetSenderAdapter failed %s",
            CtrlThread->configure_info.public_adapter.c_str()
        );
        return 1;
    }

    if (machine_thread) {
        machine_thread->SafeExitThread(0);
        machine_thread = nullptr;
    }

    machine_thread = new CStateMachineThread;

    if (
        machine_thread->CreateThread(nullptr, false) &&
        machine_thread->StartThread()
    ) {
        g_logSystem.AppendText(
            "CContextControlThread::Authenticate_InitAll "
            "machineThread->StartThread failed"
        );
        return 2;
    }

    StartAdapterStateCheck();
    return 0;
}

bool CContextControlThread::BeginStart()
{
    CtrlThread->GetAdapterMac(&old_src_macaddr);
    g_Logoff.AppendText(
        "CContextControlThread::BeginStart()---m_oldSrcMacAddr=%X-%X-%X-%X-%X-%X",
        old_src_macaddr.ether_addr_octet[0],
        old_src_macaddr.ether_addr_octet[1],
        old_src_macaddr.ether_addr_octet[2],
        old_src_macaddr.ether_addr_octet[3],
        old_src_macaddr.ether_addr_octet[4],
        old_src_macaddr.ether_addr_octet[5]
    );
    start_dst_addr.ether_addr_octet[0] = 0x01;
    start_dst_addr.ether_addr_octet[1] = 0xD0;
    start_dst_addr.ether_addr_octet[2] = 0xF8;
    start_dst_addr.ether_addr_octet[3] = 0x00;
    start_dst_addr.ether_addr_octet[4] = 0x00;
    start_dst_addr.ether_addr_octet[5] = 0x03;
    dstaddr = start_dst_addr;
    return true;
}

void CContextControlThread::CancelBOOTP()
{
    stop_dhclient_asyn();

    if (bootp_timerid) {
        if (!KillTimer(bootp_timerid))
            g_log_Wireless.AppendText("\t Kill offer ip timer error");

        bootp_timerid = 0;
        sem_wait(&bootp_semaphore);
        sem_destroy(&bootp_semaphore);
    }

    ip_offer_count = 0;
}

void CContextControlThread::CancelWaitDHCPAuthResultTimer()
{
    if (!wait_dhcp_auth_result_timerid)
        return;

    KillTimer(wait_dhcp_auth_result_timerid);
    wait_dhcp_auth_result_timerid = 0;
}

bool CContextControlThread::CheckSSAMessPacket(char *buf, unsigned buflen) const
{
    char username_buf[128] = {};
    char md5_buf[16] = {};
    CMD5Checksum md5_checksum_obj;
    std::string::size_type at_pos = 0;

    if (buflen < 32)
        return false;

    md5_checksum_obj.Update(buf, buflen - 16);
    at_pos = configure_info.last_auth_username.find('@');
    ConvertUtf8ToGBK(
        username_buf,
        sizeof(username_buf),
        configure_info.last_auth_username.c_str(),
        at_pos == std::string::npos ?
        at_pos :
        configure_info.last_auth_username.length()
    );
    md5_checksum_obj.Update(username_buf, strlen(username_buf));
    // which boy wrote this down? :)
    md5_checksum_obj.Update(
        "all pretty girls love linym",
        strlen("all pretty girls love linym")
    );
    md5_checksum_obj.Final2CharBuff(md5_buf, sizeof(md5_buf));
    return !memcmp(&buf[buflen - 16], md5_buf, sizeof(md5_buf));
}

void CContextControlThread::CheckSelf() const
{}

void CContextControlThread::ConnectClientCenter() const
{
    struct _START_CENTERCONTROL_START_ control_center_info = {};
    unsigned p = 0;
    unsigned major = 0, minor = 0;
    g_log_Wireless.AppendText("ConnectClientCenter");
    control_center_info.ipv4 = configure_info.dhcp_ipinfo.ip4_ipaddr;

    for (const char i : configure_info.dhcp_ipinfo.adapter_mac.ether_addr_octet) {
        char upper = i >> 4, lower = i & 0xF;

        if (/* upper >= 0 && */ upper <= 9)
            control_center_info.mac[p++] = upper + '0';

        else if (upper >= 10 && upper <= 15)
            control_center_info.mac[p++] = upper - 10 + 'A';

        if (/* lower >= 0 && */ lower <= 9)
            control_center_info.mac[p++] = lower + '0';

        else if (lower >= 10 && lower <= 15)
            control_center_info.mac[p++] = lower - 10 + 'A';
    }

    control_center_info.field_38 = false;
    control_center_info.domain = configure_info.client_manager_center_serveraddr;
    control_center_info.port =
        configure_info.client_manager_center_serverlistenport;
    GetSuInternalVersion(major, minor);
    control_center_info.major_ver = htonl(major);
    control_center_info.minor_ver = htonl(minor);
    CClientCenterPeerManager::StartConnect(&control_center_info);
}

void CContextControlThread::DeAuthenticate_ExitAll()
{
    struct tagPasSecurityInfo secinfo = {};
    StopAdapterStateCheck();

    if (IsWirelessAuth() && auth_inited) {
        g_log_Wireless.AppendText(
            "CContextControlThread::DeAuthenticate_ExitAll su_platform_deinit()"
        );
        su_platform_deinit();
        g_log_Wireless.AppendText(
            "CContextControlThread::DeAuthenticate_ExitAll su_platform_deinit() END"
        );
        auth_inited = false;
    }

    if (read_packet_thread) {
        read_packet_thread->SetProxyMsgID(-1);

        if (read_packet_thread) {
            g_log_Wireless.AppendText(
                "CContextControlThread::DeAuthenticate_ExitAll StopRxPacketThread "
            );

            if (read_packet_thread->StopRxPacketThread() == ETIMEDOUT) {
                g_log_Wireless.AppendText(
                    "CContextControlThread::DeAuthenticate_ExitAll TerminateThread "
                    "m_readPacketThread->m_nThreadID=%x",
                    read_packet_thread->thread_id
                );

                if (!TerminateThread(read_packet_thread->thread_id)) {
                    delete read_packet_thread;
                    read_packet_thread = nullptr;

                } else
                    g_log_Wireless.AppendText("TerminateThread read packet thread failed.");
            }
        }
    }

    g_log_Wireless.AppendText(
        "CContextControlThread::DeAuthenticate_ExitAll before  m_machineThread->StopThread"
    );

    if (machine_thread) {
        machine_thread->SafeExitThread(0);
        machine_thread = nullptr;
    }

    g_log_Wireless.AppendText(
        "CContextControlThread::DeAuthenticate_ExitAll before StopSendPacketThread"
    );

    if (send_packet_thread)
        send_packet_thread->StopSendPacketThread();

    private_properties.utrust_url.clear();
    secinfo.timeout = 30;
    secinfo.result = 1;
    secinfo.failcode = -1;
    secinfo.offline_wait_time = -1;
    CPasswordModifier::SetPasswordSecurityInfo(&secinfo);
    g_log_Wireless.AppendText("CContextControlThread::DeAuthenticate_ExitAll LEAVE");
}

void CContextControlThread::DeinitAll_Success()
{
    g_log_Wireless.AppendText("DeinitAll_Success ENTER");

    if (adapter_detect_thread)
        adapter_detect_thread->StopDetect(1);

    if (proxy_detect_thread)
        proxy_detect_thread->StopDetect();

    KillDirectSrv();

    if (hello_processor) {
        hello_processor->Exit();
        delete hello_processor;
        hello_processor = nullptr;
    }
}

void CContextControlThread::DoBOOTP()
{
    in_addr_t gateway = 0;
    g_log_Wireless.AppendText("DoBOOTP");
    SetConnectWindowText(OP_STATE_0);

    if (!get_gateway(&gateway, configure_info.public_adapter.c_str())) {
        g_log_Wireless.AppendText(
            "\t DoBOOTP get gateway failed, so del default gateway first."
        );
        del_default_gateway();
    }

    if (sem_init(&bootp_semaphore, 0, 0)) {
        g_log_Wireless.AppendText(
            "\t sem init error:%s",
            strerror(errno)
        );
        StopAuthentication(
            LOGOFF_REASON_FAIL_GETTING_DYNAMIC_IP,
            APP_QUIT_TYPE_2,
            true
        );
        return;
    }

    if (!dhclient_asyn(configure_info.public_adapter.c_str(), &bootp_semaphore)) {
        g_log_Wireless.AppendText("\t dhclient_asyn error");
        sem_destroy(&bootp_semaphore);
        StopAuthentication(
            LOGOFF_REASON_FAIL_GETTING_DYNAMIC_IP,
            APP_QUIT_TYPE_2,
            true
        );
        return;
    }

    if ((bootp_timerid = SetTimer(nullptr, PACKET_NOTIFY_MTYPE, 2000, nullptr))) {
        g_log_Wireless.AppendText("\t SetTimer error");
        stop_dhclient_asyn();
        sem_wait(&bootp_semaphore);
        sem_destroy(&bootp_semaphore);
        StopAuthentication(
            LOGOFF_REASON_FAIL_GETTING_DYNAMIC_IP,
            APP_QUIT_TYPE_2,
            true
        );
        return;
    }

    g_log_Wireless.AppendText("\t Set offer ip timer success %u", bootp_timerid);
}

void CContextControlThread::DoForSTATE_AUTHENTICATED()
{
    CSuConfigFile conffile;
    g_log_Wireless.AppendText(
        "Enter CContextControlThread::DoForSTATE_AUTHENTICATED"
    );
    g_uilog.AppendText(
        "CContextControlThread::DoForSTATE_AUTHENTICATED"
        "((WM_UPDATA_MINI_BY_STATE,0)=STATE_AUTHENTICATED"
    );
    state = STATE_AUTHENTICATED;
    theApp.GUI_update_connectdlg_by_states(STATE_AUTHENTICATED);
    RcvSvrSwitchResult(CtrlThread->private_properties.svr_switch_result);
    g_log_Wireless.AppendText("将认证的信息记录到配置文件中");
    conffile.Lock();

    if (conffile.Open()) {
        conffile.WritePrivateProfileString(
            "NETDETECT",
            "LastAuthUsername",
            configure_info.last_auth_username.c_str());
        conffile.Close();
    }

    conffile.Unlock();
    StartProcessBusiness(private_properties);
}

void CContextControlThread::DoUpgrade(
    unsigned version,
    std::string url,
    unsigned type
)
{
    static bool upgradingFromSAM = false;
    struct tagDownLoadPara download_para = {};
    g_log_Wireless.AppendText(
        "enter CContextControlThread::DoUpgrade start nVersion=%d,strUrl=%s",
        version,
        url.c_str()
    );
    replace_all_distinct(url, "\\", "/");
    url = makeLower(url);

    if (!IsUpgrade(version) || (!url.find("http://") && !url.find("ftp://"))) {
        if (type == 2)
            upgradingFromSAM = false;

        return;
    }

    if (type == 1 && upgradingFromSAM)
        return;

    else if (type == 2)
        upgradingFromSAM = true;

    upgrade_url = url;

    if (!url.find("http://")) {
        replace_all_distinct(url, "%20", " ");
        replace_all_distinct(url, "%23", "#");
    }

    if (download_thread) {
        Sleep(3);
        download_thread->PostThreadMessage(STOP_THREAD_MTYPE, 0, 0);
        download_thread = nullptr;
    }

    download_thread = new CDownLoadThread;
    download_thread->doing_upgrade = true;
    download_thread->CreateThread(nullptr, false);
    download_para.mtype = UPGRADE_RETURN_MTYPE;
//    download_para.thread_id=reinterpret_cast<pthread_t>(me); // ?????
    download_para.thread_id = reinterpret_cast<pthread_t>(thread_id);
    download_para.url = url;
    download_para.save_path = g_strAppPath + "upgrade/";
    download_para.save_filename = url.substr(0, url.rfind('/'));
    unlink((download_para.save_path + download_para.save_filename).c_str());
    download_thread->SetDlPara(download_para);
    download_thread->StartThread();
    g_log_Wireless.AppendText(
        "enter CContextControlThread::DoUpgrade end path=%s,strFileName=%s",
        download_para.save_path.c_str(),
        download_para.save_filename.c_str()
    );
}

void CContextControlThread::DoWithDHCPUpload()
{
    SetConnectWindowText(OP_STATE_1);
    g_log_Wireless.AppendText(
        "CContextControlThread::DoWithDHCPUpload()__StartDirectTrans"
    );
    StartDirectTrans(private_properties, true, false);
    g_log_Wireless.AppendText(
        "CContextControlThread::DoWithDHCPUpload()__SendUserinfoToAuthSvr"
    );

    if (SendUserinfoToAuthSvr()) {
        g_log_Wireless.AppendText("send userinfo to authsvr success");
        DoWithSendUserinfoSuccess();

    } else {
        g_log_Wireless.AppendText("send userinfo to authsvr failed");
        StopAuthentication(LOGOFF_REASON_COMM_FAIL_NO_RESPONSE, APP_QUIT_TYPE_2, true);
    }
}

bool CContextControlThread::DoWithGetDHCPInfo()
{
    unsigned i = 0;
    struct DHCPIPInfo dhcp_ipinfo = {};

    if (GetDHCPAuthStep() == 2)
        return true;

    for (i = 0; i < 3; i++) {
        if (!GetDHCPIPInfo(dhcp_ipinfo, false)) {
            g_log_Wireless.AppendText("DoWithGetDHCPInfo get ip info fail");
            Sleep(1000);
            continue;
        }

        g_log_Wireless.AppendText(
            "dhcp:%d\nip:%d.%d.%d.%d\ngateway:%d.%d.%d.%d",
            dhcp_ipinfo.dhcp_enabled,
            dhcp_ipinfo.ip4_ipaddr & 0xff,
            dhcp_ipinfo.ip4_ipaddr >> 8 & 0xff,
            dhcp_ipinfo.ip4_ipaddr >> 16 & 0xff,
            dhcp_ipinfo.ip4_ipaddr >> 24,
            dhcp_ipinfo.gateway & 0xff,
            dhcp_ipinfo.gateway >> 8 & 0xff,
            dhcp_ipinfo.gateway >> 16 & 0xff,
            dhcp_ipinfo.gateway >> 24
        );

        if (
            (
                !dhcp_ipinfo.dhcp_enabled &&
                IsGetDhcpIpp(&dhcp_ipinfo.ip4_ipaddr) &&
                IsGetDhcpIpp(&dhcp_ipinfo.gateway)
            ) ||
            dhcp_ipinfo.dhcp_enabled
        )
            break;
    }

    if (i == 3)
        return false;

    configure_info.dhcpmode_value = dhcp_ipinfo.dhcp_enabled;
    configure_info.dhcp_ipinfo = dhcp_ipinfo;

    if (configure_info.dhcp_ipinfo.dhcp_enabled) {
        configure_info.dhcp_ipinfo.ip4_ipaddr = 0;
        configure_info.dhcp_ipinfo.gateway = 0;
        configure_info.dhcp_ipinfo.ip4_netmask = 0;
    }

    return true;
}

void CContextControlThread::DoWithSendUserinfoSuccess()
{
    SetWaitDHCPAuthResultTimer();
}

int CContextControlThread::EnvironmentCheck()
{
    char tmpbuf[512] = {};
    char tmpbuf2[256] = {};
    bool network_manager_stopped = false;
    struct ifreq ifr = {};
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    std::vector<struct NICsStatus> nic_statuses;
    get_all_nics_statu(nic_statuses);

    if (check_process_run("NetworkManager")) {
        GetCurDataAndTime(tmpbuf2);
        sprintf(
            tmpbuf,
            CChangeLanguage::Instance().LoadString(262).c_str(),
            get_os_type() == OS_UBUNTU ? "network-manager" : "NetworkManager"
        );
        theApp.GUI_update_connect_text(std::string(tmpbuf2).append(tmpbuf));
        service_stop(get_os_type() == OS_UBUNTU ? "network-manager" : "NetworkManager");

        if (check_process_run("NetworkManager")) {
            GetCurDataAndTime(tmpbuf2);
            sprintf(
                tmpbuf,
                CChangeLanguage::Instance().LoadString(263).c_str(),
                get_os_type() == OS_UBUNTU ? "network-manager" : "NetworkManager"
            );
            g_log_Wireless.AppendText("%s", tmpbuf);
            std::string tmpstr = std::string(tmpbuf2).append(tmpbuf);
            g_log_Wireless.AppendText("%s", tmpstr.c_str());
            theApp.GUI_update_connect_text(tmpstr);

        } else
            network_manager_stopped = true;
    }

    if (IS_WLAN(EAP_TYPE_INVALID) && check_service_status("wpa_supplicant")) {
        GetCurDataAndTime(tmpbuf2);
        sprintf(
            tmpbuf,
            CChangeLanguage::Instance().LoadString(262).c_str(),
            "wpa_supplicant"
        );
        theApp.GUI_update_connect_text(std::string(tmpbuf2).append(tmpbuf));
        service_stop("wpa_supplicant");

        if (check_service_status("NetworkManager")) {
            GetCurDataAndTime(tmpbuf2);
            sprintf(
                tmpbuf,
                CChangeLanguage::Instance().LoadString(263).c_str(),
                "wpa_supplicant"
            );
            g_log_Wireless.AppendText("%s", tmpbuf);
            std::string tmpstr = std::string(tmpbuf2).append(tmpbuf);
            g_log_Wireless.AppendText("%s", tmpstr.c_str());
            theApp.GUI_update_connect_text(tmpstr);
        }

    } else if (!network_manager_stopped)
        return 0;

    GetCurDataAndTime(tmpbuf2);
    theApp.GUI_update_connect_text(
        std::string(tmpbuf2).append(CChangeLanguage::Instance().LoadString(264))
    );

    for (struct NICsStatus &nic_status : nic_statuses) {
        if (!nic_status.is_up)
            continue;

        strncpy(ifr.ifr_name, nic_status.nic_name, IFNAMSIZ);
        ioctl(fd, SIOCGIFFLAGS, &ifr);
        ifr.ifr_flags |= IFF_UP;
        ioctl(fd, SIOCSIFFLAGS, &ifr);

        if (check_nic_status(nic_status.nic_name) == ADAPTER_DOWN) {
            g_log_Wireless.AppendText("ifup cmd error,so try ifconfig up cmd");
            ioctl(fd, SIOCSIFFLAGS, &ifr);
        }
    }

    close(fd);
    return 0;
}

void CContextControlThread::ExitExtance_ExitAll()
{
    g_log_Wireless.AppendText("Enter CContextControlThread::ExitExtance_ExitAll");

    if (read_packet_thread) {
        read_packet_thread->ExitRxPacketThread();
        read_packet_thread->SafeExitThread(10000);
        read_packet_thread = nullptr;
    }

    if (dir_tran_srv) {
        dir_tran_srv->DeInit_Sam();
        g_log_Wireless.AppendText(
            "CContextControlThread::ExitExtance_ExitAll DeInit_Sam"
        );
        dir_tran_srv->DeInit_Smp();
        g_log_Wireless.AppendText(
            "CContextControlThread::ExitExtance_ExitAll DeInit_Smp"
        );
        dir_tran_srv->SafeExitThread(0);
        dir_tran_srv = nullptr;
        g_log_Wireless.AppendText(
            "Leaving CContextControlThread::ExitExtance_ExitAll "
            "m_pTreadDirSrv->StopThread"
        );
    }

    if (send_packet_thread) {
        send_packet_thread->SafeExitThread(10000);
        send_packet_thread = nullptr;
        g_log_Wireless.AppendText(
            "Leaving CContextControlThread::ExitExtance_ExitAll "
            "ExitSendPacketThread"
        );
    }

    if (auth_inited) {
        g_log_Wireless.AppendText(
            "CContextControlThread::ExitExtance_ExitAll su_platform_deinit()\n"
        );
        su_platform_deinit();
        g_log_Wireless.AppendText(
            "CContextControlThread::ExitExtance_ExitAll su_platform_deinit() - END\n"
        );
        auth_inited = false;
    }

    g_log_Wireless.AppendText("Leaving CContextControlThread::ExitExtance_ExitAll");
}

bool CContextControlThread::GetAdapterDHCPEnable() const
{
    return true;
}

void CContextControlThread::GetAdapterMac(struct ether_addr *dst) const
{
    *dst = configure_info.dhcp_ipinfo.adapter_mac;
}

unsigned CContextControlThread::GetDHCPAuthStep()
{
    if (IsRuijieNas() && IS_WIRED(EAP_TYPE_MD5) && IsDhcpAuth()) {
        if (dhcp_auth_step != 1 && dhcp_auth_step != 2)
            dhcp_auth_step = 1;

    } else if (dhcp_auth_step)
        dhcp_auth_step = 0;

    return dhcp_auth_step;
}

void CContextControlThread::GetDHCPInfoParam(struct DHCPIPInfo &dst) const
{
    dst = configure_info.dhcp_ipinfo;
}

in_addr_t CContextControlThread::GetNetOrderIPV4() const
{
    return configure_info.dhcp_ipinfo.ip4_ipaddr;
}

unsigned CContextControlThread::GetRadiusServer() const
{
    return radius_server;
}

std::string CContextControlThread::GetSAMMessBuff(
    char *buf,
    unsigned buflen
) const
{
    CMD5Checksum md5_checksum_obj;
    char md5_buf[16] = {};
    std::string ret;

    if (buflen <= 295) {
        g_log_Wireless.AppendText("Error Message Len\n");
        return ret;
    }

    if (buf[22] != 0x1C || buf[23] != 0x03 || buf[24] != 0x08) {
        g_log_Wireless.AppendText("Error Message Attr\n");
        return ret;
    }

    md5_checksum_obj.Update(buf, buflen - 16);
    md5_checksum_obj.Update(e_pMd5Chanllenge, sizeof(e_pMd5Chanllenge));
    md5_checksum_obj.Final2CharBuff(md5_buf, sizeof(md5_buf));

    if (memcmp(&buf[buflen - 16], md5_buf, sizeof(md5_buf))) {
        g_log_Wireless.AppendText("Error Message MD5");
        return ret;
    }

    ConvertGBKToUtf8(ret, &buf[27], strlen(&buf[27]));
    return ret;
}

bool CContextControlThread::IS_EAP_TYPE(enum EAP_TYPES type) const
{
    return eap_type == type;
}

bool CContextControlThread::IS_WIRED(enum EAP_TYPES type) const
{
    if (type == EAP_TYPE_INVALID)
        return if_type == 1;

    if (if_type == 1)
        return eap_type == type;

    return false;
}

bool CContextControlThread::IS_WLAN(enum EAP_TYPES type) const
{
    g_log_Wireless.AppendText("m_iftype=%d m_unEapType=%d", if_type, eap_type);

    if (type == EAP_TYPE_INVALID)
        return if_type == 2;

    if (if_type == 2)
        return eap_type == type;

    return false;
}

void CContextControlThread::InitAll_Success(
    const struct SuRadiusPrivate &private_prop
)
{
    char errbuf[512] = {};
    struct ether_addr macaddr = {};
    bool disallow_multi_nic_ip = false;
    g_log_Wireless.AppendText("InitAll_Success ENTER");

    switch (private_prop.parse_hello) {
        case -1:
            if (hello_processor) {
                g_log_Wireless.AppendText("Hello disable");
                hello_processor->ProcessorStop();
            }

            break;

        case 1:
            g_log_Wireless.AppendText("Hello Enable");

            if (!hello_processor)
                hello_processor = new CHelloProcessor;

            hello_processor->ProcessorRun(
                GetMessageID(),
                private_prop.parse_hello_inv,
                private_prop.parse_hello_id
            );
            break;
    }

    g_log_Wireless.AppendText(
        "StartDirectTrans IsRuijieNas %d; EAP TYPE %d; IF TYPE %d; GetRadiusServer %d;",
        IsRuijieNas(),
        eap_type,
        if_type,
        GetRadiusServer()
    );
    StartDirectTrans(private_prop, false, true);

    if (private_prop.proxy_avoid) {
        g_log_Wireless.AppendText(
            "Start Proxy Detect,type=%08x",
            private_prop.proxy_dectect_kinds
        );
        disallow_multi_nic_ip = ((private_prop.proxy_dectect_kinds ? : -1) >> 4) & 1;

        if (
            proxy_detect_thread->StartDetect(
                configure_info.public_adapter.c_str(),
                thread_id,
                RECEIVED_PROXY_DETECT_RESULT_MTYPE,
                private_prop.proxy_dectect_kinds ? : -1,
                errbuf
            )
        )
            read_packet_thread->SetProxyMsgID(
                proxy_detect_thread->GetMessageID()
            );

        else
            g_log_Wireless.AppendText("Start Proxy Detect err:%s", errbuf);
    }

    GetAdapterMac(&macaddr);

    if (
        !adapter_detect_thread->StartDetect(
            configure_info.public_adapter.c_str(),
            &macaddr,
            configure_info.dhcp_ipinfo.ip4_ipaddr,
            thread_id,
            ADAPTER_STATE_MTYPE,
            disallow_multi_nic_ip,
            errbuf
        )
    )
        g_log_Wireless.AppendText("Adapter Start Detect err:%s", errbuf);
}

void CContextControlThread::InitCheckSelf() const
{}

bool CContextControlThread::InitInstance_InitAll()
{
    CClientCenterPeerManager::Start(thread_id);
    g_log_Wireless.AppendText("\t 创建收包线程");
    read_packet_thread = nullptr;
    read_packet_thread = new CRxPacketThread;
    read_packet_thread->SetMainMsgID(GetMessageID());
    read_packet_thread->CreateThread(nullptr, false);
    g_log_Wireless.AppendText("\t 创建收包线程--END");
    send_packet_thread = new CSendPacketThread;

    if (!send_packet_thread) {
        g_logSystem.AppendText("CContextControlThread sendPacketThread new failed");
        return false;
    }

    send_packet_thread->CreateThread(nullptr, false);

    if (send_packet_thread->StartThread()) {
        g_logSystem.AppendText(
            "CContextControlThread sendPacketThread->StartThread failed"
        );
        return false;
    }

    if (!dir_tran_srv) {
        dir_tran_srv = new CDirectTranSrv;

        if (!dir_tran_srv)
            return 0;

        dir_tran_srv->CreateThread(nullptr, false);

        if (dir_tran_srv->StartThread()) {
            g_log_Wireless.AppendText("Dir Ser thread Start failed");
            return false;
        }
    }

    adapter_detect_thread = new CAdapterDetectThread;

    if (adapter_detect_thread->CreateThread(nullptr, false) != 1) {
        g_log_Wireless.AppendText("adapter detect thread create failed");
        return false;
    }

    if (adapter_detect_thread->StartThread()) {
        g_logSystem.AppendText("Adapter Detect Thread start failed");
        return false;
    }

    proxy_detect_thread = new CProxyDetectThread;

    if (proxy_detect_thread->CreateThread(nullptr, false) != 1) {
        g_log_Wireless.AppendText(
            "Proxy detect thread create failed");
        return false;
    }

    if (proxy_detect_thread->StartThread()) {
        g_logSystem.AppendText("Proxy Detect Thread start failed");
        return false;
    }

    return true;
}

bool CContextControlThread::InitNICDevice()
{
    g_log_Wireless.AppendText("Enter CContextControlThread::InitNICDevice");
    return GetNICInUse(nic_in_use, false);
}

void CContextControlThread::InitStartDstMac()
{
    struct ether_addr macaddr_special1 = { 0x01, 0x80, 0xC2, 0x00, 0x00, 0x03 };
    struct ether_addr macaddr_special2 = { 0x01, 0xD0, 0xF8, 0x00, 0x00, 0x03 };
    start_dst_addr =
        memcmp(
            &start_dst_addr,
            &macaddr_special2,
            sizeof(start_dst_addr)
        ) ?
        macaddr_special2 :
        macaddr_special1;
}

bool CContextControlThread::IsDhcpAuth() const
{
    return configure_info.dhcp_ipinfo.dhcp_enabled;
}

void CContextControlThread::IsIPOffered()
{
    if (sem_trywait(&bootp_semaphore)) {
        if (ip_offer_count++ > 20) {
            g_log_Wireless.AppendText("Error-等待dhcp offer超时");
            StopAuthentication(
                LOGOFF_REASON_FAIL_GETTING_DYNAMIC_IP,
                APP_QUIT_TYPE_2,
                true
            );
        }

        return;
    }

    g_log_Wireless.AppendText("dhclient already ret");
    sem_post(&bootp_semaphore);

    if (!GetDHCPIPInfo(configure_info.dhcp_ipinfo, false)) {
        g_log_Wireless.AppendText("OnTimer GetDHCPIPInfo error");

        if (ip_offer_count++ > 20) {
            g_log_Wireless.AppendText("Error-等待dhcp offer超时");
            StopAuthentication(
                LOGOFF_REASON_FAIL_GETTING_DYNAMIC_IP,
                APP_QUIT_TYPE_2,
                true
            );
        }

        return;
    }

    g_log_Wireless.AppendText(
        "IP:%d.%d.%d.%d; gate way:%d.%d.%d.%d",
        configure_info.dhcp_ipinfo.ip4_ipaddr & 0xff,
        configure_info.dhcp_ipinfo.ip4_ipaddr >> 8 & 0xff,
        configure_info.dhcp_ipinfo.ip4_ipaddr >> 16 & 0xff,
        configure_info.dhcp_ipinfo.ip4_ipaddr >> 24,
        configure_info.dhcp_ipinfo.gateway & 0xff,
        configure_info.dhcp_ipinfo.gateway >> 8 & 0xff,
        configure_info.dhcp_ipinfo.gateway >> 16 & 0xff,
        configure_info.dhcp_ipinfo.gateway >> 24
    );

    if (
        !IsGetDhcpIpp(&configure_info.dhcp_ipinfo.ip4_ipaddr) ||
        !IsGetDhcpIpp(&configure_info.dhcp_ipinfo.gateway)
    ) {
        if (ip_offer_count++ > 20) {
            g_log_Wireless.AppendText("Error-等待dhcp offer超时");
            StopAuthentication(
                LOGOFF_REASON_FAIL_GETTING_DYNAMIC_IP,
                APP_QUIT_TYPE_2,
                true
            );
        }

        return;
    }

    g_log_Wireless.AppendText("等待dhcp offer成功.");

    if (KillTimer(bootp_timerid)) {
        g_log_Wireless.AppendText("Kill offer ip timer success");
        bootp_timerid = 0;

    } else
        g_log_Wireless.AppendText("Error-Kill offer ip timer failure");

    ip_offer_count = 0;
    sem_destroy(&bootp_semaphore);

    if (GetDHCPAuthStep() == 1) {
        dhcp_auth_step = 2;
        StartStateMachine(false);

    } else {
        if (GetDHCPAuthStep() == 2) {
            g_log_Wireless.AppendText("Error-第2阶段不因该做获取ip操作");
            assert(false);
        }

        DoWithDHCPUpload();
    }
}

bool CContextControlThread::IsRuijieNas() const
{
    return IS_WIRED(EAP_TYPE_MD5) ? is_ruijie_nas : false;
}

bool CContextControlThread::IsServerlistUpdate(
    const std::vector<std::string> &new_list
) const
{
    return
        new_list.empty() ?
        false :
        configure_info.server_custom ?
        std::equal(
            new_list.cbegin(),
            new_list.cend(),
            configure_info.server_alt_names.cbegin()
        ) :
        true;
}

bool CContextControlThread::IsWirelessAuth() const
{
    return configure_info.public_authmode == "EAPWIRELESS";
}

void CContextControlThread::KillDirectSrv()
{
    if (!dir_tran_srv)
        return;

    if (dir_tran_srv->DeInit_Sam())
        direct_trans = false;

    if (dir_tran_srv->DeInit_Smp())
        direct_trans = false;
}

unsigned CContextControlThread::ModifyLogoffReason(
    // *INDENT-OFF*
    enum LOGOFF_REASON &reason
    // *INDENT-ON*
) const
{
    switch (reason) {
        case LOGOFF_REASON_MULTIPLE_IP:
            reason = LOGOFF_REASON_MULTIPLE_NIC;
            break;

        case LOGOFF_REASON_COMM_FAIL_TIMEOUT:
            reason = LOGOFF_REASON_FORCE_OFFLINE_3;
            break;

        case LOGOFF_REASON_IP_CHANGED:
        case LOGOFF_REASON_MAC_CHANGED:
            reason = LOGOFF_REASON_ADDRESS_CHANGED;
            break;

        case LOGOFF_REASON_NIC_NOT_CONNECTED:
        case LOGOFF_REASON_NIC_DISABLED:
        case LOGOFF_REASON_COMM_FAIL_NO_RESPONSE:
        case LOGOFF_REASON_FAIL_GETTING_DYNAMIC_IP:
            reason = LOGOFF_REASON_NORMAL_LOGOFF;
            break;

        default:
            break;
    }

    if (IsRuijieNas() && reason != LOGOFF_REASON_NORMAL_LOGOFF)
        reason = static_cast<enum LOGOFF_REASON>(reason - 100);

    return reason;
}

DEFINE_DISPATH_MESSAGE_HANDLER(ONSAMWantLogOff, CContextControlThread)
{
    if (reinterpret_cast<char *>(arg1)[0])
        logoff_message.assign(reinterpret_cast<char *>(arg1), arg2);

    else
        logoff_message = CChangeLanguage::Instance().LoadString(91);

    g_uilog.AppendText(
        "CContextControlThread::ONSAMWantLogOff(WM_SHOW_MAIN_WINDOW,0)=%s\n",
        logoff_message.c_str()
    );
    StopAuthentication(LOGOFF_REASON_FORCE_OFFLINE_2, APP_QUIT_TYPE_2, true);
}

DEFINE_DISPATH_MESSAGE_HANDLER(ONSAWantLogOff, CContextControlThread)
{
    if (reinterpret_cast<char *>(arg1)[0])
        logoff_message.assign(reinterpret_cast<char *>(arg1), arg2);

    else
        logoff_message = CChangeLanguage::Instance().LoadString(91);

    g_uilog.AppendText(
        "CContextControlThread::ONSAWantLogOff(WM_SHOW_MAIN_WINDOW,0)=%s",
        logoff_message.c_str()
    );

    switch (GetRadiusServer()) {
        case 9:
            StopAuthentication(LOGOFF_REASON_FORCE_OFFLINE_2, APP_QUIT_TYPE_2, true);
            break;

        case 5:
            StopAuthentication(LOGOFF_REASON_AUTH_FAIL, APP_QUIT_TYPE_2, true);
            break;
    }

    WriteTipInfoToLog(logoff_message, 5);
}

DEFINE_DISPATH_MESSAGE_HANDLER(OnAdaptersState, CContextControlThread)
{
    UNUSED_VAR(arg2);
    static in_addr_t gateway = 0; // szGateway
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    char speed_buf[100] = {};
    struct rtentry route = {};
    g_log_Wireless.AppendText("OnAdaptersState %d", arg1);

    switch (arg1) {
        case ADAPTER_UP_REPORT_MTYPE:
            if (IS_WLAN(EAP_TYPE_INVALID))
                break;

            if (gateway) {
                // route add default gw %d.%d.%d.%d 2>&-
                reinterpret_cast<struct sockaddr_in *>
                (&route.rt_gateway)->sin_addr.s_addr = htonl(gateway);
                ioctl(fd, SIOCADDRT, &route);
                gateway = 0;
            }

            dhcp_auth_step = 1;

            if (!IsRuijieNas())
                SendLogOffPacket(LOGOFF_REASON_NORMAL_LOGOFF, false);

            StartStateMachine(false);

            if (get_nic_speed(speed_buf, configure_info.public_adapter.c_str())) {
                std::string speed_str =
                    CChangeLanguage::Instance().LoadString(25) + speed_buf + " Mbps";
                AddMsgItem(5, speed_str);
                shownotify(
                    speed_str,
                    CChangeLanguage::Instance().LoadString(96),
                    10000
                );
            }

            break;

        case ADAPTER_DOWN_REPORT_MTYPE:
            if (IS_WLAN(EAP_TYPE_INVALID))
                break;

            theApp.GUI_update_LOGOFF(LOGOFF_REASON_NIC_NOT_CONNECTED, STATE_HOLD);
            StopAuthentication(LOGOFF_REASON_NORMAL_LOGOFF, APP_QUIT_TYPE_0, false);
            break;

        case ADAPTER_DISABLE_REPORT_MTYPE:
        case ADAPTER_ERROR_REPORT_MTYPE:
            gateway = configure_info.dhcp_ipinfo.gateway;
            theApp.GUI_update_LOGOFF(LOGOFF_REASON_NIC_DISABLED, STATE_HOLD);

            if (IS_WLAN(EAP_TYPE_INVALID))
                WlanDisconnect();

            StopAuthentication(LOGOFF_REASON_NORMAL_LOGOFF, APP_QUIT_TYPE_0, false);
            break;

        case ADAPTER_ENABLE_REPORT_MTYPE:
            if (IS_WIRED(EAP_TYPE_INVALID))
                break;

            if (gateway) {
                // route add default gw %d.%d.%d.%d 2>&-
                reinterpret_cast<struct sockaddr_in *>
                (&route.rt_gateway)->sin_addr.s_addr = htonl(gateway);
                ioctl(fd, SIOCADDRT, &route);
                gateway = 0;
            }

            StartStateMachine(false);
            break;

        case MULTIPLE_ADAPTER_MTYPE:
            StopAuthentication(LOGOFF_REASON_MULTIPLE_NIC, APP_QUIT_TYPE_2, true);
            break;

        case MULTIPLE_IP_MTYPE:
            StopAuthentication(LOGOFF_REASON_MULTIPLE_IP, APP_QUIT_TYPE_2, true);
            break;

        case MAC_CHANGED_MTYPE:
            dhcp_auth_step = 1;
            SendLogOffPacket(LOGOFF_REASON_MAC_CHANGED, true);
            StartStateMachine(false);
            break;

        case IP_CHANGED_MTYPE:
            dhcp_auth_step = 1;

            if (!IsRuijieNas())
                SendLogOffPacket(LOGOFF_REASON_IP_CHANGED, true);

            StartStateMachine(false);
            break;
    }

    close(fd);
}

DEFINE_DISPATH_MESSAGE_HANDLER(OnConnectNotify, CContextControlThread)
{
    UNUSED_VAR(arg2);

    switch (arg1) {
        case 0:
            StopAuthentication(LOGOFF_REASON_NORMAL_LOGOFF, APP_QUIT_TYPE_1, true);
            break;

        case 1:
            SendLogOffPacket(LOGOFF_REASON_NORMAL_LOGOFF, false);
            break;

        case 2:
            g_log_Wireless.AppendText("发生了重认证，开始StartStateMachine");
            StartStateMachine(false);
            break;

        case 3:
            if (CBackoffReAuthenticationManager::Instance().reauth_timer) {
                KillTimer(CBackoffReAuthenticationManager::Instance().reauth_timer);
                CBackoffReAuthenticationManager::Instance().reauth_timer = 0;
            }

            StopAuthentication(LOGOFF_REASON_NORMAL_LOGOFF, APP_QUIT_TYPE_2, true);
            break;

        case 4:
            if (reconnect_timer) {
                KillTimer(reconnect_timer);
                reconnect_timer = 0;
            }

            if (configure_info.is_autoreconnect)
                reconnect_timer =
                    SetTimer(
                        nullptr,
                        RECONNECT_TIMER_MTYPE,
                        60000 * CtrlThread->configure_info.autoreconnect,
                        nullptr
                    );

            break;
    }
}

DEFINE_DISPATH_MESSAGE_HANDLER(
    OnLogoffWithUnknownReason,
    CContextControlThread
)
{
    UNUSED_VAR(arg1);
    UNUSED_VAR(arg2);
    g_log_Wireless.AppendText("CContextControlThread::OnLogoffWithUnknownReason()");
    StopAuthentication(LOGOFF_REASON_UNKNOWN_REASON, APP_QUIT_TYPE_2, true);
}

DEFINE_DISPATH_MESSAGE_HANDLER(OnOpenSSOURL, CContextControlThread)
{
    UNUSED_VAR(arg1);
    UNUSED_VAR(arg2);

    if (CtrlThread->private_properties.utrust_url.empty())
        return;

    if (CtrlThread->private_properties.delay_second_show_utrust_url)
        open_sso_url_timer =
            SetTimer(
                OPEN_SSO_URL_TIMER_MTYPE,
                1000 * CtrlThread->private_properties.delay_second_show_utrust_url
            );

    else
        show_sso_url();
}

DEFINE_DISPATH_MESSAGE_HANDLER(OnOthersWantLogOff, CContextControlThread)
{
    StopAuthentication(
        static_cast<enum LOGOFF_REASON>(arg1),
        APP_QUIT_TYPE_2,
        arg2
    );
}

DEFINE_DISPATH_MESSAGE_HANDLER(OnPacketReturn, CContextControlThread) const
{
    struct EAPOLFrame *eapol_frame = nullptr;
    struct eapolpkg *eapol_pkg = reinterpret_cast<struct eapolpkg *>(arg2);

    if (!IS_WIRED(EAP_TYPE_MD5) || !eapol_pkg)
        return;

    if (arg1 <= 13) {
        delete eapol_pkg;
        return;
    }

    if (
        IsLoopBack(
            reinterpret_cast<struct ether_addr *>
            (eapol_pkg->etherheader.ether_shost)
        )
    ) {
        g_log_Wireless.AppendText("OnPacketReturn packet is skip i=%d", 2);
        delete eapol_pkg;
        return;
    }

    if (
        !IsHostDstMac(
            reinterpret_cast<struct ether_addr *>(eapol_pkg->etherheader.ether_dhost)
        ) &&
        !Is8021xGroupAddr(
            reinterpret_cast<struct ether_addr *>(eapol_pkg->etherheader.ether_dhost)
        ) &&
        !IsStarGroupDstMac(
            reinterpret_cast<struct ether_addr *>(eapol_pkg->etherheader.ether_dhost)
        )
    ) {
        g_log_Wireless.AppendText("OnPacketReturn packet is skip i=%d", 4);
        delete eapol_pkg;
        return;
    }

    if (eapol_pkg->eap_packet.code == EAP_REQUEST)
        CtrlThread->SetNasManufacturer(
            (
                eapol_pkg->etherheader.ether_shost[0] == 0x14 &&
                eapol_pkg->etherheader.ether_shost[1] == 0x14 &&
                eapol_pkg->etherheader.ether_shost[2] == 0x4B
            ) ||
            (
                eapol_pkg->etherheader.ether_shost[0] == 0x58 &&
                eapol_pkg->etherheader.ether_shost[1] == 0x69 &&
                eapol_pkg->etherheader.ether_shost[2] == 0x6C
            ) ||
            (
                eapol_pkg->etherheader.ether_shost[0] == 0x00 &&
                eapol_pkg->etherheader.ether_shost[1] == 0xD0 &&
                eapol_pkg->etherheader.ether_shost[2] == 0xF8
            ) ||
            (
                eapol_pkg->etherheader.ether_shost[0] == 0x00 &&
                eapol_pkg->etherheader.ether_shost[1] == 0x1A &&
                eapol_pkg->etherheader.ether_shost[2] == 0xA9
            )
        );

    eapol_frame = ChangeToEAPOLFrame(eapol_pkg, arg1);

    if (!eapol_frame)
        return;

    if (
        eapol_frame->ieee8021x_packet_type ==
        static_cast<enum IEEE8021X_PACKET_TYPE>(0xC0)
    ) {
        delete eapol_frame;

        if (
            !CheckSSAMessPacket(reinterpret_cast<char *>(eapol_pkg), arg1) ||
            !SSAMessPrivParse(reinterpret_cast<char *>(eapol_pkg), arg1)
        )
            delete eapol_pkg;

        return;
    }

    if (
        eapol_frame->ieee8021x_packet_type ==
        static_cast<enum IEEE8021X_PACKET_TYPE>(0xBF)
    ) {
        delete eapol_frame;
        std::string msgbuf =
            GetSAMMessBuff(reinterpret_cast<char *>(eapol_pkg), arg1);
        delete eapol_pkg;

        if (msgbuf.empty())
            return;

        AddMsgItem(5, msgbuf);
        g_uilog.AppendText(
            "AddMsgItem shownotify CContextControlThread::OnPacketReturn,strInfo=%s",
            msgbuf.c_str()
        );
        shownotify(msgbuf, CChangeLanguage::Instance().LoadString(95), 0);
        return;
    }

    rj_printf_debug(
        "Get a Eap packet Id = %d\n",
        eapol_frame->ieee8021x_packet_type
    );

    if (
        !machine_thread ||
        !machine_thread->PostThreadMessage(
            PACKET_NOTIFY_MTYPE,
            3,
            reinterpret_cast<unsigned long>(eapol_frame)
        )
    )
        delete eapol_frame;

    delete eapol_pkg;
}

DEFINE_DISPATH_MESSAGE_HANDLER(OnPatchLogoff, CContextControlThread)
{
    UNUSED_VAR(arg1);
    UNUSED_VAR(arg2);
    StopAuthentication(LOGOFF_REASON_NORMAL_LOGOFF, APP_QUIT_TYPE_0, true);
    g_uilog.AppendText("CContextControlThread::OnPatchLogoff(WM_QUIT_MAIN_LOOP,1)");
    theApp.GUI_QuitMainLoop("");
}

DEFINE_DISPATH_MESSAGE_HANDLER(OnRcvDHCPAuthResult, CContextControlThread)
{
    const char *msgl = reinterpret_cast<const char *>(arg2);

    if (!wait_dhcp_auth_result_timerid) {
        g_log_Wireless.AppendText(
            "%s 已经收到成功认证成功报文",
            "OnRcvDHCPAuthResult"
        );
        return;
    }

    CancelWaitDHCPAuthResultTimer();

    if (arg1) {
        if (arg2) {
            g_uilog.AppendText(
                "shownotify CContextControlThread::OnRcvDHCPAuthResult,strInfo=%s",
                msgl
            );
            shownotify(msgl, CChangeLanguage::Instance().LoadString(95), 0);
        }

        DoForSTATE_AUTHENTICATED();

        if (arg2)
            delete[] msgl;

        return;
    }

    if (arg2) {
        logoff_message = msgl;
        StopAuthentication(LOGOFF_REASON_FORCE_OFFLINE_3, APP_QUIT_TYPE_2, true);
        delete[] msgl;
        return;
    }

    logoff_message.clear();
    StopAuthentication(LOGOFF_REASON_COMM_FAIL_TIMEOUT, APP_QUIT_TYPE_2, true);
}

DEFINE_DISPATH_MESSAGE_HANDLER(
    OnRcvProxyDetectResult,
    CContextControlThread
)
{
    char tmpbuf[1024] = {};
    enum REQUEST_TYPE proxy_type = static_cast<enum REQUEST_TYPE>(arg1);
    enum LOGOFF_REASON logoff_reason = LOGOFF_REASON_UNKNOWN_REASON;
    in_addr_t fake_ipaddr = 0;
    struct ether_addr fake_macaddr = {};
    g_log_Wireless.AppendText("Rcv Proxy Detect Result:%u,%u", arg1, arg2);
    e_pHelloID[0] = 0x75;

    switch (proxy_type) {
        case REQUEST_HTTP:
            sprintf(
                tmpbuf,
                CChangeLanguage::Instance().LoadString(279).c_str(),
                "HTTP"
            );
            logoff_message = tmpbuf;
            logoff_reason =
                GetRadiusServer() == 9 ?
                LOGOFF_REASON_PROXY_DETECTED :
                LOGOFF_REASON_HTTP_PROXY;
            break;

        case REQUEST_SOCK4:
        case REQUEST_SOCK4A:
        case REQUEST_SOCK5:
            sprintf(
                tmpbuf,
                CChangeLanguage::Instance().LoadString(279).c_str(),
                "SOCKS"
            );
            logoff_message = tmpbuf;
            logoff_reason =
                GetRadiusServer() == 9 ?
                LOGOFF_REASON_PROXY_DETECTED :
                LOGOFF_REASON_SOCKS_PROXY;
            break;

        case REQUEST_FAKING_MAC:
            logoff_reason = LOGOFF_REASON_OTHERS_FAKING_MAC;

            if (
                proxy_detect_thread &&
                proxy_detect_thread->GetFakeInfo(&fake_ipaddr, &fake_macaddr)
            ) {
                sprintf(
                    tmpbuf,
                    CChangeLanguage::Instance().LoadString(278).c_str(),
                    inet_ntoa({ fake_ipaddr }),
                    fake_macaddr.ether_addr_octet[0],
                    fake_macaddr.ether_addr_octet[1],
                    fake_macaddr.ether_addr_octet[2],
                    fake_macaddr.ether_addr_octet[3],
                    fake_macaddr.ether_addr_octet[4],
                    fake_macaddr.ether_addr_octet[5]
                );
                logoff_message = tmpbuf;

            } else {
                logoff_message = "Others are faking your mac";
                g_log_Wireless.AppendText("GetFakeMacInfo failed");
            }

            break;

        default:
            logoff_reason = LOGOFF_REASON_PROXY_DETECTED;
            break;
    }

    if (GetRadiusServer() == 9)
        logoff_reason = LOGOFF_REASON_PROXY_DETECTED;

    StopAuthentication(logoff_reason, APP_QUIT_TYPE_2, true);
}

DEFINE_DISPATH_MESSAGE_HANDLER(OnRecvFailure, CContextControlThread)
{
    UNUSED_VAR(arg1);
    UNUSED_VAR(arg2);
    CtrlThread->has_auth_success = false;

    if (reconnect_timer) {
        KillTimer(reconnect_timer);
        reconnect_timer = 0;
    }
}

DEFINE_DISPATH_MESSAGE_HANDLER(OnSaWantReAuth, CContextControlThread)
{
    UNUSED_VAR(arg1);
    UNUSED_VAR(arg2);

    if (!IsRuijieNas())
        SendLogOffPacket(LOGOFF_REASON_NORMAL_LOGOFF, false);

    g_log_Wireless.AppendText("SAM要求重认证，开始StartStateMachine");
    StartStateMachine(false);
}

DEFINE_DISPATH_MESSAGE_HANDLER(OnSayHello, CContextControlThread) const
{
    if (machine_thread)
        machine_thread->PostThreadMessage(SAY_HELLO_MTYPE, arg1, arg2);
}

DEFINE_DISPATH_MESSAGE_HANDLER(OnSaySaEvent, CContextControlThread) const
{
    if (machine_thread)
        machine_thread->PostThreadMessage(SAY_HELLO_MTYPE, arg1, arg2);
}

DEFINE_DISPATH_MESSAGE_HANDLER(OnShowSaMessage, CContextControlThread) const
{
    UNUSED_VAR(arg2);
    const char *msgl = reinterpret_cast<const char *>(arg1);
    g_uilog.AppendText(
        "shownotify CContextControlThread::OnShowSaMessage,strInfo=%s",
        msgl
    );
    shownotify(msgl, CChangeLanguage::Instance().LoadString(95), 0);
    delete[] msgl;
}

DEFINE_DISPATH_MESSAGE_HANDLER(OnSimulateLogoff, CContextControlThread) const
{
    struct eapolpkg *eapol_pkg = nullptr;

    if (!arg2 || arg1 <= 4 || arg1 - 4 % 6)
        return;

    for (unsigned off = 4; off < arg1; off += 6) {
        eapol_pkg = new struct eapolpkg;
        *reinterpret_cast<struct ether_addr *>(eapol_pkg->etherheader.ether_dhost)
            = CtrlThread->dstaddr;
        memcpy(
            eapol_pkg->etherheader.ether_shost,
            reinterpret_cast<char *>(arg2 + off),
            sizeof(struct ether_addr)
        );
        eapol_pkg->etherheader.ether_type = htons(ETH_P_PAE);
        eapol_pkg->ieee8021x_version = 1;
        eapol_pkg->ieee8021x_packet_type = IEEE8021X_EAPOL_LOGOFF;
        eapol_pkg->ieee8021x_packet_length = 0;
        CtrlThread->PostThreadMessage(
            SAY_HELLO_MTYPE,
            18,
            reinterpret_cast<unsigned long>(eapol_pkg)
        );
    }

    delete[] reinterpret_cast<char *>(arg2);
}

DEFINE_DISPATH_MESSAGE_HANDLER(OnStartMachine, CContextControlThread)
{
    UNUSED_VAR(arg1);
    UNUSED_VAR(arg2);
    char tmpbuf[256] = {};
    SET_EAP_TYPE();
    SET_IF_TYPE();
    g_log_Wireless.AppendText("OnStartMachine");
    CBackoffReAuthenticationManager::Instance().Reset();

    if (!DoWithGetDHCPInfo()) {
        g_log_Wireless.AppendText("获取DHCP信息失败");
        GetCurDataAndTime(tmpbuf);
        theApp.GUI_update_connectdlg_by_states(STATE_LOGOFF);
        theApp.GUI_update_connect_text(
            std::string(tmpbuf).append(CChangeLanguage::Instance().LoadString(48))
        );
        return;
    }

    if (EnvironmentCheck() == -1) {
        g_log_Wireless.AppendText("EnvironmentCheck error");
        theApp.GUI_update_LOGOFF(LOGOFF_REASON_NORMAL_LOGOFF, STATE_LOGOFF);
        return;
    }

    if (!configure_info.dhcp_ipinfo.dhcp_enabled)
        repair_ip_gateway(configure_info.dhcp_ipinfo, configure_info.public_adapter);

    dhcp_auth_step = 1;
    has_auth_success = false;
    CtrlThread->private_properties.svr_switch_result.clear();
    CtrlThread->field_1139 = false;
    GetCurDataAndTime(tmpbuf);
    theApp.GUI_update_connectdlg_by_states(STATE_CONNECTING);
    theApp.GUI_update_connect_text(
        std::string(tmpbuf).append(CChangeLanguage::Instance().LoadString(26))
    );

    if (!BeginStart()) {
        g_log_Wireless.AppendText("BeginStart failed return");
        theApp.GUI_update_LOGOFF(LOGOFF_REASON_NORMAL_LOGOFF, STATE_LOGOFF);
        return;
    }

    switch (Authenticate_InitAll()) {
        case 0:
            if (StartStateMachine(true))
                DeAuthenticate_ExitAll();

            break;

        case 1:
            theApp.GUI_ShowMainWindow(CChangeLanguage::Instance().LoadString(1));
            break;

        case 2:
            theApp.GUI_ShowMainWindow("Thread create failed");
            break;

        case 3:
            theApp.GUI_ShowMainWindow("su_platform_init failed");
            break;

        case 4:
            theApp.GUI_ShowMainWindow("SetDirectMsgID failed");
            break;
    }
}

DEFINE_DISPATH_MESSAGE_HANDLER(
    OnStateMachineReturn,
    CContextControlThread
)
{
    UNUSED_VAR(arg2);
    static std::string strLogFile = g_strAppPath + "log/run.log";
    static enum STATES last_state = STATE_DISASSOC;
    CBackoffReAuthenticationManager &boram_instance =
        CBackoffReAuthenticationManager::Instance();
    char tmpbuf[64] = {};
    std::string timestr;
    timer_t reauth_timer = 0;
    g_log_Wireless.AppendText("Enter CContextControlThread::OnStateMachineReturn");
    GetCurDataAndTime(tmpbuf);
    timestr = tmpbuf;

    switch (arg1) {
        case STATE_CONNECTING:
            g_log_Wireless.AppendText("\t STATE_CONNECTING");
            theApp.GUI_update_connect_states_and_text(
                STATE_CONNECTING,
                timestr + CChangeLanguage::Instance().LoadString(32)
            );
            CLogFile::LogToFile(
                CChangeLanguage::Instance().LoadString(32).c_str(),
                strLogFile.c_str(),
                true,
                true
            );
            g_uilog.AppendText(
                "CContextControlThread::OnStateMachineReturn(CONNECTING "
                "WM_UPDATA_MINI_TEXT=%s) 0",
                (timestr + CChangeLanguage::Instance().LoadString(32)).c_str()
            );
            break;

        case STATE_ACQUIRED:
            g_log_Wireless.AppendText("\t STATE_ACQUIRED");
            CLogFile::LogToFile(
                CChangeLanguage::Instance().LoadString(33).c_str(),
                strLogFile.c_str(),
                true,
                true
            );
            theApp.GUI_update_connect_states_and_text(
                STATE_ACQUIRED,
                timestr + CChangeLanguage::Instance().LoadString(33)
            );
            g_uilog.AppendText(
                "CContextControlThread::OnStateMachineReturn(ACQUIRED "
                "WM_UPDATA_MINI_TEXT=%s) 1",
                (timestr + CChangeLanguage::Instance().LoadString(33)).c_str()
            );
            break;

        case STATE_AUTHENTICATING:
            g_log_Wireless.AppendText("\t STATE_AUTHENTICATING");
            CLogFile::LogToFile(
                CChangeLanguage::Instance().LoadString(34).c_str(),
                strLogFile.c_str(),
                true,
                true
            );
            theApp.GUI_update_connect_states_and_text(
                STATE_AUTHENTICATING,
                timestr + CChangeLanguage::Instance().LoadString(34)
            );
            g_uilog.AppendText(
                "CContextControlThread::OnStateMachineReturn(AUTHENTICATING "
                "WM_UPDATA_MINI_TEXT=%s) 1",
                (timestr + CChangeLanguage::Instance().LoadString(34)).c_str()
            );
            break;

        case STATE_AUTHENTICATED:
            g_log_Wireless.AppendText("\t 认证完成");
            CBackoffReAuthenticationManager::Instance().reauth_count = 0;

            if (
                IS_WIRED(EAP_TYPE_INVALID) &&
                (
                    CtrlThread->connect_fail ||
                    (
                        CtrlThread->reconnect_fail &&
                        CtrlThread->logoff_message.empty()
                    )
                )
            ) {
                g_log_Wireless.AppendText(
                    "CtrlThread->m_bConnectFail=%d || "
                    "(m_bReconnFail=%d && CtrlThread->m_strFailInfo=%s)",
                    CtrlThread->connect_fail,
                    reconnect_fail,
                    logoff_message.c_str()
                );
                logoff_message = CChangeLanguage::Instance().LoadString(145);
                PostThreadMessage(STATE_MACHINE_RETURN_MTYPE, 6, 0);
                return;
            }

            DeinitAll_Success();
            g_log_Wireless.AppendText(
                "Radius Server %u;  EAP TYPE %d; IF TYPE %d; auth step:%d",
                GetRadiusServer(),
                eap_type,
                if_type,
                GetDHCPAuthStep()
            );

            if (!IsDhcpAuth() || GetDHCPAuthStep() == 2)
                DoForSTATE_AUTHENTICATED();

            else
                DoBOOTP();

            break;

        case STATE_HOLD:
            g_log_Wireless.AppendText("\t STATE_HELD");
            AddMsgItem(5, CtrlThread->logoff_message);
            CLogFile::LogToFile(
                (
                    CChangeLanguage::Instance().LoadString(29) +
                    CtrlThread->logoff_message
                ).c_str(),
                strLogFile.c_str(),
                true,
                true
            );
            theApp.GUI_update_connect_states_and_text(
                STATE_HOLD,
                timestr +
                CChangeLanguage::Instance().LoadString(29) +
                (CtrlThread->logoff_message.empty() ? "" : ": ") +
                CtrlThread->logoff_message
            );
            g_uilog.AppendText(
                "AddMsgItem CContextControlThread::OnStateMachineReturn"
                "((WM_UPDATA_MINI_TEXT =%s & WM_UPDATA_MINI_BY_STATE,0)=STATE_HELD",
                (
                    timestr +
                    CChangeLanguage::Instance().LoadString(29) +
                    (CtrlThread->logoff_message.empty() ? "" : ": ") +
                    CtrlThread->logoff_message
                ).c_str()
            );
            break;

        case STATE_LOGOFF:
            g_log_Wireless.AppendText("\t STATE_LOGOFF");
            g_uilog.AppendText(
                "CContextControlThread::OnStateMachineReturn"
                "(LOGOFF WM_UPDATA_MINI_BY_STATE,STATE_LOGOFF,/*WM_SHOW_MAIN_WINDOW*/ 0) "
            );

            if (
                !IS_WIRED(EAP_TYPE_INVALID) ||
                !connect_fail ||
                !logoff_message.empty()
            ) {
                StopAuthentication(
                    LOGOFF_REASON_NORMAL_LOGOFF,
                    APP_QUIT_TYPE_2,
                    true
                );
                break;
            }

            StopAuthentication(
                LOGOFF_REASON_NORMAL_LOGOFF,
                APP_QUIT_TYPE_0,
                true
            );

            if (!boram_instance.IsNeedReAuthentication())
                break;

            boram_instance.reauth_count++;
            reauth_timer =
                SetTimer(
                    REAUTH_TIMER_MTYPE,
                    1000 * boram_instance.GetReAuthenticationTimerElapse()
                );

            if (reauth_timer)
                boram_instance.reauth_timer = reauth_timer;

            else
                g_log_Wireless.AppendText(
                    "CContextControlThread::OnStateMachineReturn>"
                    "failed to create backoff-reauthentication timer(elapse=%ds).",
                    boram_instance.GetReAuthenticationTimerElapse()
                );

            break;

        case REACQURE:
            g_log_Wireless.AppendText("\t REACQURE");
            break;

        case STATE_SUCCESSNOTIFY:
            g_log_Wireless.AppendText("\t STATE_SUCCESSNOTIFY");
            g_uilog.AppendText(
                "shownotify CContextControlThread::OnStateMachineReturn,strInfo=%s",
                CtrlThread->success_info.c_str()
            );
            shownotify(
                CtrlThread->success_info,
                CChangeLanguage::Instance().LoadString(95),
                0
            );
            break;

        case STATE_BEGIN:
            g_log_Wireless.AppendText("\t STATE_BEGIN");
            theApp.GUI_update_connect_states_and_text(
                STATE_CONNECTING,
                timestr + CChangeLanguage::Instance().LoadString(259)
            );
            CLogFile::LogToFile(
                CChangeLanguage::Instance().LoadString(259).c_str(),
                strLogFile.c_str(),
                true,
                true
            );
            break;

        case STATE_DISASSOC:
            g_log_Wireless.AppendText("\t STATE_DISASSOC");
            CLogFile::LogToFile(
                CChangeLanguage::Instance().LoadString(280).c_str(),
                strLogFile.c_str(),
                true,
                true
            );
            theApp.GUI_update_connect_states_and_text(
                STATE_HOLD,
                timestr + CChangeLanguage::Instance().LoadString(280)
            );
            g_uilog.AppendText(
                "CContextControlThread::OnStateMachineReturn"
                "(DISASSOC|STATE_HELD WM_UPDATA_MINI_TEXT=%s) 1",
                (timestr + CChangeLanguage::Instance().LoadString(280)).c_str()
            );
            break;

        case STATE_ROAM_SUCCESS:
            g_log_Wireless.AppendText("\t STATE_ROAM_SUCCESS");

            if (last_state != STATE_AUTHENTICATED && last_state != STATE_ROAM_SUCCESS)
                theApp.GUI_update_connectdlg_by_states(STATE_AUTHENTICATED);

            break;

        case STATE_END:
            g_log_Wireless.AppendText("\t STATE_END");
            AddMsgItem(5, CtrlThread->logoff_message);
            CLogFile::LogToFile(
                (
                    CChangeLanguage::Instance().LoadString(29) +
                    CtrlThread->logoff_message
                ).c_str(),
                strLogFile.c_str(),
                true,
                true
            );
            theApp.GUI_update_connect_states_and_text(
                STATE_HOLD,
                timestr +
                CChangeLanguage::Instance().LoadString(29) +
                (CtrlThread->logoff_message.empty() ? "" : ": ") +
                CtrlThread->logoff_message
            );
            g_uilog.AppendText(
                "AddMsgItem CContextControlThread::OnStateMachineReturn"
                "((WM_UPDATA_MINI_TEXT =%s & WM_UPDATA_MINI_BY_STATE,0)=STATE_HELD",
                (
                    timestr +
                    CChangeLanguage::Instance().LoadString(29) +
                    (CtrlThread->logoff_message.empty() ? "" : ": ") +
                    CtrlThread->logoff_message
                ).c_str()
            );
            break;
    }

    last_state = static_cast<enum STATES>(arg1);
}

DEFINE_DISPATH_MESSAGE_HANDLER(OnTimer, CContextControlThread)
{
    UNUSED_VAR(arg2);
    g_logFile_start.AppendText(
        "CContextControlThread::OnTimer nIDEvent = %d",
        arg1
    );

    switch (arg1) {
        case RECONNECT_TIMER_MTYPE:
            g_log_Wireless.AppendText("启动了重认证间隔，开始StartStateMachine");

            if (!IsRuijieNas())
                SendLogOffPacket(LOGOFF_REASON_NORMAL_LOGOFF, false);

            StartStateMachine(false);
            rj_printf_debug("TIME_AUTO_CONNECT-SendStartPacket\r\n");
            break;

        case DHCP_TIMEOUT_MTYPE:
            g_log_Wireless.AppendText("wait dhcp auth result timeout");
            CancelWaitDHCPAuthResultTimer();
            StopAuthentication(LOGOFF_REASON_COMM_FAIL_TIMEOUT, APP_QUIT_TYPE_2, true);
            break;

        case DHCP_IP_MTYPE:
            g_log_Wireless.AppendText("DHCP IP timer");

            if (!bootp_timerid) {
                g_log_Wireless.AppendText(
                    "Warning-DHCP IP timer 上次已删除成功,这是残留的timer消息 wyf"
                );
                break;
            }

            IsIPOffered();
            break;

        case REAUTH_TIMER_MTYPE:
            g_log_Wireless.AppendText("退避重认证.");
            KillTimer(CBackoffReAuthenticationManager::Instance().reauth_timer);
            CBackoffReAuthenticationManager::Instance().reauth_timer = 0;
            StartStateMachine(false);
            break;

        case OPEN_SSO_URL_TIMER_MTYPE:
            g_log_Wireless.AppendText("延迟打开SSOURL.");

            if (open_sso_url_timer) {
                KillTimer(open_sso_url_timer);
                show_sso_url();
            }

            break;

        default:
            rj_printf_debug(
                "CContextControlThread unknown timer flag = %d",
                arg1
            );
            g_logSystem.AppendText("unknown timer flag = %d", arg1);
            break;
    }

    OnTimerLeave(arg1);
}

DEFINE_DISPATH_MESSAGE_HANDLER(OnUpGradeReturn, CContextControlThread)
{
    char tmpbuf[1024] = {};
    g_uilog.AppendText(
        " CContextControlThread::OnUpGradeReturn(WM_QUIT_MAIN_LOOP)lParam=%d",
        arg2
    );

    if (!arg2) {
        if (arg1)
            delete[] reinterpret_cast<char *>(arg1);

        updteParam = true;
        update_file_path = reinterpret_cast<char *>(arg1);
        update_message.clear();
        StopAuthentication(LOGOFF_REASON_NORMAL_LOGOFF, APP_QUIT_TYPE_2, true);
        return;
    }

    switch (arg2) {
        case 3:
        case 4:
        case 5:
        case 7:
        case 9:
        default:
            sprintf(
                tmpbuf,
                CChangeLanguage::Instance().LoadString(276).c_str(),
                upgrade_url.c_str()
            );
            break;

        case 6:
            sprintf(
                tmpbuf,
                CChangeLanguage::Instance().LoadString(277).c_str(),
                (g_strAppPath + "upgrade/").c_str()
            );
            break;

        case 8:
            sprintf(
                tmpbuf,
                CChangeLanguage::Instance().LoadString(275).c_str(),
                (g_strAppPath + "upgrade/").c_str()
            );
            break;
    }

    shownotify(tmpbuf, CChangeLanguage::Instance().LoadString(96), 0);
}

DEFINE_DISPATH_MESSAGE_HANDLER(OnUpgradeClient, CContextControlThread)
{
    DoUpgrade(arg1, reinterpret_cast<char *>(arg2), 1);
    delete[] reinterpret_cast<char *>(arg2);
}

void CContextControlThread::OnShowLoginURL() const
{
    show_login_url();
}

bool CContextControlThread::RefreshSignal(const std::string &adapter_name)
{
    static std::string strLastAdapter;
    unsigned su_plat_init_ret = 0;
    unsigned su_cmd_ret = 0;
    char debug_file[256] = {};
    struct SuPlatformParam platform_param = {};
    struct SupfCmd cmd = {};

    if (strLastAdapter != adapter_name && auth_inited) {
        g_log_Wireless.AppendText("strLastAdapter != strAdaptor su_platform_deinit");
        su_platform_deinit();
        auth_inited = false;
    }

    if (adapter_name.empty()) {
        strLastAdapter = adapter_name;
        return false;
    }

    if (
        get_nic_type(adapter_name.c_str()) ==
        ADAPTER_WIRED /* ADAPTER_WIRELESS ???? */
    ) {
        rj_printf_debug("%s IS WIRED\n", adapter_name.c_str());
        strLastAdapter = adapter_name;
        return false;
    }

    if (strLastAdapter != adapter_name && auth_inited) {
        strcpy(platform_param.driver_name, "wext");
        strcpy(platform_param.ifname, adapter_name.c_str());
        sprintf(debug_file, "%slog/supf_debug.log", g_strAppPath.c_str());
//        platform_param.event_callback = supf_event_callback_fun;
        platform_param.debug_file =
#ifdef NDEBUG
            nullptr;
#else
            debug_file;
#endif // NDEBUG
        rj_printf_debug(" before su_platform_init \n");
        su_plat_init_ret = su_platform_init(&platform_param);
        rj_printf_debug(" after su_platform_init \n");

        if (su_plat_init_ret)
            return false;

        auth_inited = true;
    }

    strLastAdapter = adapter_name;
    cmd.cmd_type = SUPF_SCAN_CMD;
    cmd.cmd_ctx = nullptr;

    if ((su_cmd_ret = su_platform_cmd(&cmd))) {
        rj_printf_debug("SUPF_SCAN_CMD FALSE nRet=%d\n", su_cmd_ret);
        return false;
    }

    rj_printf_debug("CContextControlThread::RefreshSignal END\n");
    return true;
}

void CContextControlThread::SET_EAP_TYPE()
{
    if (configure_info.public_authmode == "EAPMD5")
        eap_type = EAP_TYPE_MD5;

    else if (configure_info.public_authmode == "EAPWIRELESS")
        eap_type = EAP_TYPE_PEAP;

    else if (configure_info.public_authmode == "EAPTLS")
        eap_type = EAP_TYPE_TLS;

    else
        eap_type = EAP_TYPE_INVALID;
}

void CContextControlThread::SET_IF_TYPE()
{
    if (
        configure_info.public_authmode == "EAPMD5" ||
        configure_info.public_authmode == "EAPTLS"
    )
        if_type = 1;

    else if (configure_info.public_authmode == "EAPWIRELESS")
        if_type = 2;

    else
        if_type = 0;
}

#define DESKEY reinterpret_cast<const unsigned char *>("womendou")
#define INIT_XORKEY { 'L', 'i', 'u', 'B', 'i', 'n', '&', 'U' }

bool CContextControlThread::SSAMessPrivParse(char *buf, unsigned buflen) const
{
    char xorkey[32] = INIT_XORKEY;
    unsigned char tmpibuf[8] = {}, tmpobuf[8] = {};
    buflen -= 25;
    buf += 25;

    if (buflen & 7)
        return false;

    deskey(DESKEY, DE1);

    for (unsigned i = 0; i < buflen >> 3; i++) {
        memcpy(tmpibuf, buf, 8);
        des(tmpibuf, tmpobuf);

        for (unsigned j = 0; j < 8; j++)
            tmpobuf[j] ^= xorkey[j];

        memcpy(buf, tmpobuf, 8);
        buf += 8;
        memcpy(xorkey, tmpibuf, 8);
    }

    return true;
}

void CContextControlThread::SaveRadiusPrivate(
    const struct EAPOLFrame *eapol_frame
)
{
    CRGPrivateProc priproc;
    g_log_Wireless.AppendText("SaveRadiusPrivate");

    if (IsRuijieNas())
        priproc.ParseRadiusInfo_RuijieNas(eapol_frame, private_properties);
}

void CContextControlThread::SendLogOffPacket(
    enum LOGOFF_REASON logoff_reason,
    bool
) const
{
    char tmpbuf[128] = {};
    char finalbuf[1000] = {};
    unsigned finalbuflen = 0;
    struct eapolpkg *eapol_pkg = nullptr;
    ModifyLogoffReason(logoff_reason);
    g_log_Wireless.AppendText("Enter CContextControlThread::SendLogOffPacket");

    if (!IsRuijieNas() && logoff_reason != 1)
        switch (GetRadiusServer()) {
            case 5:
#define PUT_TYPE(type) finalbuf[finalbuflen++] = (type)
#define PUT_LENGTH(length) \
    do { \
        *reinterpret_cast<uint16_t *>(&finalbuf[finalbuflen]) = \
                htons(length); \
        finalbuflen += 2; \
    } while (0)
#define PUT_DATA(buf, buflen) \
    do { \
        memcpy(&finalbuf[finalbuflen], (buf), (buflen)); \
        finalbuflen += (buflen); \
    } while (0)
#define PUT_DATA_IMMEDIATE_BYTE(byte) finalbuf[finalbuflen++] = (byte)
#define PUT_DATA_IMMEDIATE_UINT16(value) \
    do { \
        *reinterpret_cast<uint16_t *>(&finalbuf[finalbuflen]) = \
                htons(value); \
        finalbuflen += 2; \
    } while (0)
                g_log_Wireless.AppendText("SendLogOffPacket 第三方兼容，服务器类型是SMP");
                PUT_TYPE(0x01);
                PUT_LENGTH(0x01);
                PUT_DATA_IMMEDIATE_BYTE(0x25);
                ConvertUtf8ToGBK(
                    tmpbuf,
                    sizeof(tmpbuf),
                    configure_info.last_auth_username.c_str(),
                    configure_info.last_auth_username.length()
                );
                PUT_TYPE(0x02);
                PUT_LENGTH(strlen(tmpbuf));
                PUT_DATA(tmpbuf, strlen(tmpbuf));
                ConvertUtf8ToGBK(
                    tmpbuf,
                    sizeof(tmpbuf),
                    diskid.c_str(),
                    diskid.length()
                );
                PUT_TYPE(0x03);
                PUT_LENGTH(strlen(tmpbuf));
                PUT_DATA(tmpbuf, strlen(tmpbuf));
                PUT_TYPE(0x79);
                PUT_LENGTH(0x02);
                PUT_DATA_IMMEDIATE_UINT16(logoff_reason);

                if (dir_tran_srv) {
                    g_log_Wireless.AppendText(
                        "SendLogOffPacket m_pTreadDirSrv->SendToSmp befor"
                    );
                    g_log_Wireless.AppendText(
                        "SendLogOffPacket m_pTreadDirSrv(%p)->SendToSmp return %d",
                        dir_tran_srv,
                        dir_tran_srv->SendToSmp(finalbuf, finalbuflen, 10)
                    );
                }

#undef PUT_DATA
#undef PUT_LENGTH
#undef PUT_TYPE
#undef PUT_DATA_IMMEDIATE_BYTE
#undef PUT_DATA_IMMEDIATE_UINT16
                break;

            case 9:
#define PUT_TYPE(type) finalbuf[finalbuflen++] = (type)
#define PUT_LENGTH(length) finalbuf[finalbuflen++] = (length)
#define PUT_DATA(buf, buflen) \
    do { \
        memcpy(&finalbuf[finalbuflen], (buf), (buflen)); \
        finalbuflen += (buflen); \
    } while (0)
#define PUT_DATA_IMMEDIATE_BYTE(byte) finalbuf[finalbuflen++] = (byte)
#define PUT_DATA_IMMEDIATE_UINT32(value) \
    do { \
        *reinterpret_cast<uint32_t *>(&finalbuf[finalbuflen]) = \
                htonl(value); \
        finalbuflen += 4; \
    } while (0)
#define PUT_DATA_IMMEDIATE_UINT16(value) \
    do { \
        *reinterpret_cast<uint16_t *>(&finalbuf[finalbuflen]) = \
                htons(value); \
        finalbuflen += 2; \
    } while (0)
                PUT_TYPE(0x01);
                PUT_LENGTH(0x01);
                PUT_DATA_IMMEDIATE_BYTE(0x04);
                ConvertUtf8ToGBK(
                    tmpbuf,
                    sizeof(tmpbuf),
                    configure_info.last_auth_username.c_str(),
                    configure_info.last_auth_username.length()
                );
                PUT_TYPE(0x03);
                PUT_LENGTH(strlen(tmpbuf));
                PUT_DATA(tmpbuf, strlen(tmpbuf));
                PUT_TYPE(0x04);
                PUT_LENGTH(0x04);
                PUT_DATA_IMMEDIATE_UINT32(ntohl(GetNetOrderIPV4()));
                PUT_TYPE(0x05);
                PUT_LENGTH(0x06);
                GetAdapterMac(
                    reinterpret_cast<struct ether_addr *>(&finalbuf[finalbuflen])
                );
                finalbuflen += 6;
                PUT_TYPE(0x07);
                PUT_LENGTH(0x02);
                PUT_DATA_IMMEDIATE_UINT16(logoff_reason);
                logFile_debug.AppendText(
                    "m_pTreadDirSrv->SendToSam(csLogoffReson, nTmpIndex, 10) call");

                if (dir_tran_srv)
                    dir_tran_srv->SendToSam(finalbuf, finalbuflen, 10);

                logFile_debug.AppendText(
                    "m_pTreadDirSrv->SendToSam(csLogoffReson, nTmpIndex, 10) end call"
                );
#undef PUT_DATA
#undef PUT_LENGTH
#undef PUT_TYPE
#undef PUT_DATA_IMMEDIATE_BYTE
#undef PUT_DATA_IMMEDIATE_UINT32
#undef PUT_DATA_IMMEDIATE_UINT16
                break;
        }

    if (!IS_WIRED(EAP_TYPE_INVALID)) {
        WlanDisconnect();
        return;
    }

    if (IsRuijieNas()) {
        g_log_Wireless.AppendText("锐捷设备下线原因 %d", logoff_reason);

        if (machine_thread)
            machine_thread->PostThreadMessage(SEND_LOGOFF_MTYPE, 2, logoff_reason);

        return;
    }

    if (machine_thread)
        machine_thread->txSetLogOff_CompThird();

    eapol_pkg = new struct eapolpkg;
    eapol_pkg->ieee8021x_packet_length = 0;
    *reinterpret_cast<struct ether_addr *>(eapol_pkg->etherheader.ether_dhost) =
        CtrlThread->dstaddr;
    CtrlThread->GetAdapterMac(
        reinterpret_cast<struct ether_addr *>(eapol_pkg->etherheader.ether_shost)
    );
    eapol_pkg->etherheader.ether_type = htons(ETH_P_PAE);
    eapol_pkg->ieee8021x_version = 1;
    eapol_pkg->ieee8021x_packet_type = IEEE8021X_EAPOL_LOGOFF;
    g_log_Wireless.AppendText("SendPakcet logoff before");
    send_packet_thread->SendPacket(reinterpret_cast<char *>(eapol_pkg), 18);
    g_log_Wireless.AppendText("SendPakcet logoff after");
}

bool CContextControlThread::SendUserinfoToAuthSvr() const
{
    switch (GetRadiusServer()) {
        case 5:
            return SendUserinfoToAuthSvr_ForSMP();

        case 9:
            return SendUserinfoToAuthSvr_ForSAM();

        default:
            assert(false);
    }
}

bool CContextControlThread::SendUserinfoToAuthSvr_ForSAM() const
{
    char tmpbuf[128] = {};
    char finalbuf[200] = {};
    unsigned finalbuflen = 0;
#define PUT_TYPE(type) finalbuf[finalbuflen++] = (type)
#define PUT_LENGTH(length) finalbuf[finalbuflen++] = (length)
#define PUT_DATA(buf, buflen) \
    do { \
        memcpy(&finalbuf[finalbuflen], (buf), (buflen)); \
        finalbuflen += (buflen); \
    } while (0)
#define PUT_DATA_IMMEDIATE_BYTE(byte) finalbuf[finalbuflen++] = (byte)
    PUT_TYPE(0x01);
    PUT_LENGTH(0x01);
    PUT_DATA_IMMEDIATE_BYTE(0x08);
    ConvertUtf8ToGBK(
        tmpbuf,
        sizeof(tmpbuf),
        configure_info.last_auth_username.c_str(),
        configure_info.last_auth_username.length()
    );
    PUT_TYPE(0x03);
    PUT_LENGTH(strlen(tmpbuf));
    PUT_DATA(tmpbuf, strlen(tmpbuf));
    PUT_TYPE(0x04);
    PUT_LENGTH(sizeof(configure_info.dhcp_ipinfo.ip4_ipaddr));
    PUT_DATA(&configure_info.dhcp_ipinfo.ip4_ipaddr, 4);
    PUT_TYPE(0x05);
    PUT_LENGTH(sizeof(struct ether_addr));
    GetAdapterMac(
        reinterpret_cast<struct ether_addr *>(&finalbuf[finalbuflen])
    );
    finalbuflen += 6;
    PUT_TYPE(0x0B);
    PUT_LENGTH(sizeof(configure_info.dhcp_ipinfo.ip4_netmask));
    PUT_DATA(
        &configure_info.dhcp_ipinfo.ip4_netmask,
        sizeof(configure_info.dhcp_ipinfo.ip4_netmask)
    );
    PUT_TYPE(0x0C);
    PUT_LENGTH(sizeof(configure_info.dhcp_ipinfo.gateway));
    PUT_DATA(
        &configure_info.dhcp_ipinfo.gateway,
        sizeof(configure_info.dhcp_ipinfo.gateway)
    );
    PUT_TYPE(0x0D);
    PUT_LENGTH(sizeof(configure_info.dhcp_ipinfo.dns));
    PUT_DATA(
        &configure_info.dhcp_ipinfo.dns,
        sizeof(configure_info.dhcp_ipinfo.dns)
    );

    if (configure_info.dhcp_ipinfo.ipaddr6_count) {
        PUT_TYPE(0xE);
        PUT_LENGTH(sizeof(configure_info.dhcp_ipinfo.ip6_link_local_ipaddr));
        PUT_DATA(
            &configure_info.dhcp_ipinfo.ip6_link_local_ipaddr,
            sizeof(configure_info.dhcp_ipinfo.ip6_link_local_ipaddr)
        );
        PUT_TYPE(0xF);
        PUT_LENGTH(sizeof(configure_info.dhcp_ipinfo.ip6_ipaddr));
        PUT_DATA(
            &configure_info.dhcp_ipinfo.ip6_ipaddr,
            sizeof(configure_info.dhcp_ipinfo.ip6_ipaddr)
        );
        PUT_TYPE(0x11);
        PUT_LENGTH(sizeof(configure_info.dhcp_ipinfo.ip6_my_ipaddr));
        PUT_DATA(
            &configure_info.dhcp_ipinfo.ip6_my_ipaddr,
            sizeof(configure_info.dhcp_ipinfo.ip6_my_ipaddr)
        );
        PUT_TYPE(0x12);
        PUT_LENGTH(0x01);
        PUT_DATA_IMMEDIATE_BYTE(configure_info.dhcp_ipinfo.ipaddr6_count);
    }

#undef PUT_DATA
#undef PUT_LENGTH
#undef PUT_TYPE
#undef PUT_DATA_IMMEDIATE_BYTE
    assert(finalbuflen <= 200);

    if (!dir_tran_srv)
        return false;

    return dir_tran_srv->SendToSam(finalbuf, finalbuflen, 10);
}

bool CContextControlThread::SendUserinfoToAuthSvr_ForSMP() const
{
    char tmpbuf[128] = {};
    char finalbuf[200] = {};
    unsigned finalbuflen = 0;
    unsigned dnsbuflen = 0;
#define PUT_TYPE(type) finalbuf[finalbuflen++] = (type)
#define PUT_LENGTH(length) \
    do { \
        *reinterpret_cast<uint16_t *>(&finalbuf[finalbuflen]) = \
                htons(length); \
        finalbuflen += 2; \
    } while (0)
#define PUT_DATA(buf, buflen) \
    do { \
        memcpy(&finalbuf[finalbuflen], (buf), (buflen)); \
        finalbuflen += (buflen); \
    } while (0)
#define PUT_DATA_IMMEDIATE_BYTE(byte) finalbuf[finalbuflen++] = (byte)
    PUT_TYPE(0x01);
    PUT_LENGTH(0x01);
    PUT_DATA_IMMEDIATE_BYTE(0x2D);
    ConvertUtf8ToGBK(
        tmpbuf,
        sizeof(tmpbuf),
        configure_info.last_auth_username.c_str(),
        configure_info.last_auth_username.length()
    );
    PUT_TYPE(0x02);
    PUT_LENGTH(strlen(tmpbuf));
    PUT_DATA(tmpbuf, strlen(tmpbuf));
    ConvertUtf8ToGBK(tmpbuf, sizeof(tmpbuf), diskid.c_str(), diskid.length());
    PUT_TYPE(0x03);
    PUT_LENGTH(strlen(tmpbuf));
    PUT_DATA(tmpbuf, strlen(tmpbuf));
    PUT_TYPE(0xCD);
    PUT_LENGTH(sizeof(configure_info.dhcp_ipinfo.adapter_mac));
    PUT_DATA(
        &configure_info.dhcp_ipinfo.adapter_mac,
        sizeof(configure_info.dhcp_ipinfo.adapter_mac)
    );
    PUT_TYPE(0xC9);
    PUT_LENGTH(sizeof(configure_info.dhcp_ipinfo.ip4_ipaddr));
    PUT_DATA(
        &configure_info.dhcp_ipinfo.ip4_ipaddr,
        sizeof(configure_info.dhcp_ipinfo.ip4_ipaddr)
    );
    PUT_TYPE(0xCA);
    PUT_LENGTH(sizeof(configure_info.dhcp_ipinfo.ip4_netmask));
    PUT_DATA(
        &configure_info.dhcp_ipinfo.ip4_netmask,
        sizeof(configure_info.dhcp_ipinfo.ip4_netmask)
    );
    PUT_TYPE(0xCB);
    PUT_LENGTH(sizeof(configure_info.dhcp_ipinfo.gateway));
    PUT_DATA(
        &configure_info.dhcp_ipinfo.gateway,
        sizeof(configure_info.dhcp_ipinfo.gateway)
    );
    PUT_TYPE(0xCC);
    PUT_LENGTH(sizeof(configure_info.dhcp_ipinfo.dns));
    PUT_DATA(
        &configure_info.dhcp_ipinfo.dns,
        sizeof(configure_info.dhcp_ipinfo.dns)
    );
    PUT_TYPE(0xD3);
    get_alternate_dns(&finalbuf[finalbuflen + 2], dnsbuflen);
    PUT_LENGTH(dnsbuflen);
    finalbuflen += dnsbuflen;
    assert(finalbuflen <= 800);
#undef PUT_DATA
#undef PUT_LENGTH
#undef PUT_TYPE
#undef PUT_DATA_IMMEDIATE_BYTE

    if (!dir_tran_srv || dir_tran_srv->SendToSmp(finalbuf, finalbuflen, 10)) {
        g_log_Wireless.AppendText("send ip information to smp fail");
        return false;
    }

    g_log_Wireless.AppendText("send ip information to smp success");
    return true;
}

void CContextControlThread::SetConnectWindowText(enum OP_STATE op_state) const
{
    char timebuf[64] = {};
    std::string timestr;
    GetCurDataAndTime(timebuf);
    timestr = timebuf;
    timestr.append(
        CChangeLanguage::Instance().LoadString(op_state == OP_STATE_1 ? 252 : 251)
    );
    theApp.GUI_update_connect_text(timestr);
    g_uilog.AppendText(
        "CContextControlThread::SetConnectWindowText(WM_UPDATA_MINI_TEXT,1)=%s",
        timestr.c_str()
    );
}

void CContextControlThread::SetInitFailed() const
{
    char timebuf[64] = {};
    std::string timestr;
    GetCurDataAndTime(timebuf);
    timestr = timebuf;
    timestr.append(CChangeLanguage::Instance().LoadString(248));
    theApp.GUI_update_connect_text(timestr);
}

void CContextControlThread::SetNasManufacturer(bool is_ruijie_nas_l)
{
    is_ruijie_nas = is_ruijie_nas_l;
}

void CContextControlThread::SetRadiusServer(unsigned type)
{
    if (type == 9 || type == 5)
        radius_server = type;
}

void CContextControlThread::SetWaitDHCPAuthResultTimer()
{
    CancelWaitDHCPAuthResultTimer();
    wait_dhcp_auth_result_timerid = SetTimer(DHCP_TIMEOUT_MTYPE, 9000);
}

bool CContextControlThread::StartAdapterStateCheck() const
{
    char errbuf[512] = {};
    struct ether_addr macaddr = {};
    GetAdapterMac(&macaddr);

    if (
        !adapter_detect_thread->StartDetect(
            configure_info.public_adapter.c_str(),
            &macaddr,
            configure_info.dhcp_ipinfo.ip4_ipaddr,
            thread_id,
            ADAPTER_STATE_MTYPE,
            false,
            errbuf
        )
    ) {
        g_log_Wireless.AppendText("Fail to start adapter state checking.");
        return false;
    }

    return true;
}

void CContextControlThread::StartDirectTrans(
    const struct SuRadiusPrivate &private_prop,
    bool wait,
    bool request_init_data_now
)
{
    char tmpbuf[128] = {};
    struct DHCPIPInfo dhcp_ipinfo = {};
    struct tagSmpParaDir smp_para_dir = {};
    struct tagDirectTranSrvPara sam_para_dir = {};

    if (direct_trans) {
        g_log_Wireless.AppendText("CContextControlThread::StartDirectTrans 直通已开启");
        return;
    }

    direct_trans = true;
    g_log_Wireless.AppendText("enter CContextControlThread::StartDirectTrans 开启直通模块");
    GetDHCPIPInfo(dhcp_ipinfo, false);

    switch (GetRadiusServer()) {
        case 5:
            ConvertUtf8ToGBK(
                tmpbuf,
                sizeof(tmpbuf),
                configure_info.last_auth_username.c_str(),
                configure_info.last_auth_username.length()
            );
            strcpy(smp_para_dir.username, tmpbuf);
            smp_para_dir.server_and_us_in_the_same_subnet = false;
            smp_para_dir.success = false;
            smp_para_dir.hello_interval = 1000 * private_prop.hello_interv;
            memcpy(
                smp_para_dir.keybuf,
                private_prop.encrypt_key,
                sizeof(smp_para_dir.keybuf)
            );
            memcpy(
                smp_para_dir.ivbuf,
                private_prop.encrypt_iv,
                sizeof(smp_para_dir.ivbuf)
            );
            smp_para_dir.smp_ipaddr = htonl(private_prop.indicate_serv_ip);
            smp_para_dir.smp_port = private_prop.indicate_port;
            smp_para_dir.su_port = private_prop.msg_client_port;
            smp_para_dir.utc_time = private_prop.server_utc_time;
            smp_para_dir.su_ipaddr = dhcp_ipinfo.ip4_ipaddr;
            smp_para_dir.adapter_mac = dhcp_ipinfo.adapter_mac;
            smp_para_dir.gateway_ipaddr = dhcp_ipinfo.gateway;
            strcpy(smp_para_dir.diskid, diskid.c_str());
            smp_para_dir.timeout = 3000;
            smp_para_dir.diskid_len = diskid.length();
            smp_para_dir.retry_count = 3;
            smp_para_dir.request_init_data_now = request_init_data_now;
            smp_para_dir.version =
                private_prop.direct_communication_highest_version_supported <= 2 ?
                private_prop.direct_communication_highest_version_supported :
                2;

            if (dir_tran_srv)
                dir_tran_srv->Init_Smp(&smp_para_dir, wait);

            break;

        case 9:
            if (private_prop.hello_interv) {
                sam_para_dir.use_handshake_to_sam = true;
                sam_para_dir.timer_to_sam = private_prop.hello_interv;

            } else
                sam_para_dir.use_handshake_to_sam = false;

            sam_para_dir.field_0 = true;
            ConvertUtf8ToGBK(
                tmpbuf,
                sizeof(tmpbuf),
                configure_info.last_auth_username.c_str(),
                configure_info.last_auth_username.length()
            );
            strcpy(smp_para_dir.username, tmpbuf);
            memcpy(
                sam_para_dir.keybuf,
                private_prop.encrypt_key,
                sizeof(sam_para_dir.keybuf)
            );
            g_eapPeapLog.AppendText("");
            memcpy(
                sam_para_dir.ivbuf,
                private_prop.encrypt_iv,
                sizeof(sam_para_dir.ivbuf)
            );
            sam_para_dir.sam_ipaddr = htonl(private_prop.indicate_serv_ip);
            sam_para_dir.sam_port = private_prop.indicate_port;
            sam_para_dir.su_port = private_prop.msg_client_port;
            sam_para_dir.su_ipaddr = dhcp_ipinfo.ip4_ipaddr;
            sam_para_dir.timeout = 3000;
            sam_para_dir.retry_count = 3;
            sam_para_dir.utc_time = private_prop.server_utc_time;
            sam_para_dir.adapter_mac = dhcp_ipinfo.adapter_mac;
            sam_para_dir.gateway_ipaddr = dhcp_ipinfo.gateway;
            sam_para_dir.version =
                private_prop.direct_communication_highest_version_supported <= 2 ?
                private_prop.direct_communication_highest_version_supported :
                2;
            sam_para_dir.server_and_us_in_the_same_subnet =
                (sam_para_dir.sam_ipaddr & dhcp_ipinfo.ip4_netmask) ==
                (dhcp_ipinfo.ip4_ipaddr & dhcp_ipinfo.ip4_netmask);

            if (dir_tran_srv)
                dir_tran_srv->Init_Sam(&sam_para_dir, wait);

            if (!private_prop.smp_ipaddr) {
                logFile_debug.AppendText("SMP服务器ip为0,不启动SMP直通");
                break;
            }

            strcpy(
                smp_para_dir.username,
                configure_info.last_auth_username.c_str()
            );
            memcpy(
                smp_para_dir.keybuf,
                private_prop.encrypt_key,
                sizeof(smp_para_dir.keybuf)
            );
            memcpy(
                smp_para_dir.ivbuf,
                private_prop.encrypt_iv,
                sizeof(smp_para_dir.ivbuf)
            );
            smp_para_dir.smp_ipaddr = htonl(private_prop.smp_ipaddr);
            smp_para_dir.smp_port = private_prop.indicate_port;
            smp_para_dir.utc_time = 0;
            smp_para_dir.su_port = private_prop.msg_client_port;
            smp_para_dir.su_ipaddr = dhcp_ipinfo.ip4_ipaddr;
            smp_para_dir.adapter_mac = dhcp_ipinfo.adapter_mac;
            logFile_debug.AppendText(
                "设置直通的网关为:%d.%d.%d.%d",
                dhcp_ipinfo.gateway & 0xff,
                dhcp_ipinfo.gateway >> 8 & 0xff,
                dhcp_ipinfo.gateway >> 16 & 0xff,
                dhcp_ipinfo.gateway >> 24
            );
            smp_para_dir.gateway_ipaddr = dhcp_ipinfo.gateway;
            strcpy(smp_para_dir.diskid, diskid.c_str());
            smp_para_dir.diskid_len = diskid.length();
            smp_para_dir.timeout = 3000;
            smp_para_dir.retry_count = 3;
            smp_para_dir.server_and_us_in_the_same_subnet =
                (smp_para_dir.smp_ipaddr & dhcp_ipinfo.ip4_netmask) ==
                (smp_para_dir.su_ipaddr & dhcp_ipinfo.ip4_netmask);
            smp_para_dir.success = true;
            smp_para_dir.request_init_data_now = true;
            smp_para_dir.version =
                private_prop.direct_communication_highest_version_supported <= 2 ?
                private_prop.direct_communication_highest_version_supported :
                2;

            if (dir_tran_srv)
                dir_tran_srv->Init_Smp(&smp_para_dir, false);
    }
}

void CContextControlThread::StartProcessBusiness(
    const struct SuRadiusPrivate &private_prop
)
{
    process_business_started = true;
    g_log_Wireless.AppendText("Enter StartProcessBusiness启动总业务");

    if (GetRadiusServer() == 9)
        DoUpgrade(private_prop.su_newest_ver, private_prop.su_upgrade_url, 2);

    ConnectClientCenter();
    SuccessNotification(private_prop.account_info);
    SuccessNotification(private_prop.persional_info);

    if (
        !CtrlThread->private_properties.utrust_url.empty() &&
        CtrlThread->private_properties.is_show_utrust_url == 1
    )
        OnOpenSSOURL(0, 0);

    OnShowLoginURL();
    SuccessNotification(
        private_prop.broadcast_str.empty() &&
        CtrlThread->configure_info.other_authhintenable ?
        CChangeLanguage::Instance().LoadString(172) :
        private_prop.broadcast_str
    );
    InitAll_Success(private_prop);

    if (!IS_WIRED(EAP_TYPE_INVALID)) {
        configure_info.is_autoreconnect = false;
        configure_info.autoreconnect = 0;
        PostThreadMessage(ASK_SMP_INIT_DATA_MTYPE, 4, 0);
        return;
    }

    if (IsRuijieNas()) {
        PostThreadMessage(ASK_SMP_INIT_DATA_MTYPE, 4, 0);
        return;
    }

    if (private_prop.su_reauth_interv > 0) {
        configure_info.is_autoreconnect = true;
        configure_info.autoreconnect = private_prop.su_reauth_interv;
        PostThreadMessage(ASK_SMP_INIT_DATA_MTYPE, 4, 0);
    }
}

int CContextControlThread::StartStateMachine(bool no_get_dhcpinfo)
{
    static bool bfirst = true;
    char timebuf[256] = {};
    struct eapolpkg *eapol_pkg = nullptr;
    g_log_Wireless.AppendText("Enter CContextControlThread::StartStateMachine");
    private_properties.radius_type = 5;
    private_properties.su_newest_ver = 0;
    private_properties.account_info.clear();
    private_properties.persional_info.clear();
    private_properties.broadcast_str.clear();
    private_properties.fail_reason.clear();
    private_properties.su_upgrade_url.clear();
    private_properties.su_reauth_interv = 0;
    private_properties.radius_type = 0;
    private_properties.proxy_avoid = 0;
    private_properties.dialup_avoid = 0;
    private_properties.indicate_serv_ip = 0;
    private_properties.indicate_port = 0;
    private_properties.smp_ipaddr = 0;
    private_properties.proxy_dectect_kinds = 0;
    private_properties.hello_interv = 0;
    memset(
        private_properties.encrypt_key,
        0,
        sizeof(private_properties.encrypt_key)
    );
    memset(
        private_properties.encrypt_iv,
        0,
        sizeof(private_properties.encrypt_iv)
    );
    private_properties.server_utc_time = 0;
    private_properties.svr_switch_result.clear();
    private_properties.services.clear();
    private_properties.user_login_url.clear();
    private_properties.msg_client_port = 80;
    private_properties.utrust_url.clear();
    private_properties.direct_communication_highest_version_supported = 1;
    private_properties.is_show_utrust_url = 1;
    private_properties.delay_second_show_utrust_url = 0;
    private_properties.parse_hello = 0;
    private_properties.direct_comm_heartbeat_flags = 0;
    process_business_started = 0;
    logoff_message.clear();
    field_1150 = 0;
    ip_offer_count = 0;
    g_strNotify.clear();

    if (!no_get_dhcpinfo && !DoWithGetDHCPInfo()) {
        g_log_Wireless.AppendText("获取DHCP信息失败");
        GetCurDataAndTime(timebuf);
        theApp.GUI_update_connectdlg_by_states(STATE_LOGOFF);
        theApp.GUI_update_connect_text(
            std::string(timebuf).append(CChangeLanguage::Instance().LoadString(48))
        );
        return -1;
    }

    if (IS_WLAN(EAP_TYPE_PEAP)) {
        g_log_Wireless.AppendText(
            "CContextControlThread::StartStateMachine 无线认证，m_CurrentState=%d",
            state
        );
        state = STATE_AUTHENTICATING;
        WlanConnect();
        return 0;
    }

    if (bfirst && IS_WIRED(EAP_TYPE_INVALID) && !check_safe_exit(1)) {
        eapol_pkg = new struct eapolpkg;
        eapol_pkg->etherheader.ether_dhost[0] = 0x01;
        eapol_pkg->etherheader.ether_dhost[1] = 0x80;
        eapol_pkg->etherheader.ether_dhost[2] = 0xC2;
        eapol_pkg->etherheader.ether_dhost[3] = 0x00;
        eapol_pkg->etherheader.ether_dhost[4] = 0x00;
        eapol_pkg->etherheader.ether_dhost[5] = 0x03;
        CtrlThread->GetAdapterMac(
            reinterpret_cast<struct ether_addr *>(eapol_pkg->etherheader.ether_shost)
        );
        eapol_pkg->etherheader.ether_type = htons(ETH_P_PAE);
        eapol_pkg->ieee8021x_version = 1;
        eapol_pkg->ieee8021x_packet_type = IEEE8021X_EAPOL_LOGOFF;
        send_packet_thread->SendPacket(reinterpret_cast<char *>(eapol_pkg), 18);
        delete eapol_pkg;
    }

    bfirst = false;
    state = STATE_AUTHENTICATING;
    machine_thread->PostThreadMessage(START_STATE_MACHINE_MTYPE, 0, 0);
    g_log_Wireless.AppendText(
        "CContextControlThread::StartStateMachine PostThreadMessage "
        "WM_MACHINE_START failed "
    );
    return 0;
}

bool CContextControlThread::StopAdapterStateCheck() const
{
    return adapter_detect_thread ? adapter_detect_thread->StopDetect(2) : true;
}

void CContextControlThread::StopAuthentication(
    enum LOGOFF_REASON logoff_reason,
    enum APP_QUIT_TYPE quit_type,
    bool logoff_if_required
)
{
    g_log_Wireless.AppendText("Enter CContextControlThread::StopAuthentication");

    if (state == STATE_LOGOFF || state == STATE_INVALID) {
        g_logSystem.AppendText(
            "CContextControlThread::StopAuthentication already logoff - return[%d]",
            state
        );
        return;
    }

    state = STATE_LOGOFF;
    theApp.GUI_update_LOGOFF(logoff_reason, STATE_LOGOFF);
    CancelWaitDHCPAuthResultTimer();
    CancelBOOTP();
    g_log_Wireless.AppendText("nLogOffReson %d", logoff_reason);

    if (logoff_reason != LOGOFF_REASON_UNKNOWN_REASON && logoff_if_required)
        SendLogOffPacket(logoff_reason, true);

    DeinitAll_Success();

    if (reconnect_timer) {
        KillTimer(reconnect_timer);
        reconnect_timer = 0;
    }

    if (quit_type != APP_QUIT_TYPE_0) {
        g_log_Wireless.AppendText("DeAuthenticate_ExitAll execute");
        DeAuthenticate_ExitAll();

        if (post_command('q'))
            g_log_Wireless.AppendText("StopAuthentication to quit program");

        else
            g_log_Wireless.AppendText("StopAuthentication to quit program failed");
    }

    g_log_Wireless.AppendText("LEAVE CContextControlThread::StopAuthentication");
}

void CContextControlThread::SuccessNotification(
    const std::string &notif_str
) const
{
    if (notif_str.empty())
        return;

    AddMsgItem(5, notif_str);
    g_uilog.AppendText(
        "AddMsgItem shownotify CContextControlThread::SuccessNotification"
        "(CString strMsg)=%s",
        notif_str.c_str()
    );
    shownotify(notif_str, CChangeLanguage::Instance().LoadString(95), 0);
}

bool CContextControlThread::WlanConnect() const
{
    char tmpbuf[2000] = {};
    unsigned tmpbuflen = 0;
    unsigned cmdret = 0;
    struct StartCmdCtx start_cmd_ctx = {};
    struct SupfCmd cmd = {};
    CRGPrivateProc priproc;
    g_log_Wireless.AppendText("@%s", "WlanConnect");
    priproc.EncapRGVerdorSegForPeap(tmpbuf, tmpbuflen, nullptr);
    start_cmd_ctx.eap_type = RFC_EAP_PEAP;
    start_cmd_ctx.eap_type_internal = RFC_EAP_MSCHAPV2;
    strcpy(start_cmd_ctx.identity, configure_info.last_auth_username.c_str());
    strcpy(start_cmd_ctx.password, configure_info.last_auth_password.c_str());
    strcpy(
        start_cmd_ctx.ssid,
        CtrlThread->configure_info.public_wirelessconf.c_str()
    );
    start_cmd_ctx.ssid_len = strlen(start_cmd_ctx.ssid);
    start_cmd_ctx.private_len = tmpbuflen;
    start_cmd_ctx.private_data = tmpbuf;
    cmd.cmd_type = SUPF_START_CMD;
    cmd.cmd_ctx = &start_cmd_ctx;

    if ((cmdret = su_platform_cmd(&cmd))) {
        g_log_Wireless.AppendText(
            "%s start cmd error=%d",
            "WlanConnect",
            cmdret
        );
        return false;
    }

    return true;
}

bool CContextControlThread::WlanDisconnect() const
{
    struct SupfCmd cmd = {};
    cmd.cmd_type = SUPF_STOP_CMD;
    cmd.cmd_ctx = nullptr;
    return !su_platform_cmd(&cmd);
}

void CContextControlThread::WlanScanComplete(
    const struct ScanCmdCtx *scan_result
)
{
    bool found = false;
    wireless_signal.clear();
    rj_printf_debug("扫描完成-num(%d)\n", scan_result->num);

    for (int i = 0; i < scan_result->num; i++) {
        found = false;

        if (
            scan_result->res[i].wpa_ie.wpa_ie_set ||
            !scan_result->res[i].wpa_ie.key_mgmt & WPA_KEY_MGMT_IEEE8021X
        )
            continue;

        rj_printf_debug(
            "ssid(len=%d):%s;bssid:%02x:%02x:%02x:%02x:%02x:%02x;signal:%d\n",
            scan_result->res[i].ssid_len,
            scan_result->res[i].ssid,
            scan_result->res[i].bssid[0],
            scan_result->res[i].bssid[1],
            scan_result->res[i].bssid[2],
            scan_result->res[i].bssid[3],
            scan_result->res[i].bssid[4],
            scan_result->res[i].bssid[5],
            scan_result->res[i].qual
        );

        for (struct tagWirelessSignal &signal : wireless_signal) {
            if (
                signal.ssid_len == scan_result->res[i].ssid_len &&
                !memcmp(signal.ssid, scan_result->res[i].ssid, signal.ssid_len)
            ) {
                signal.qual = std::max(signal.qual, scan_result->res[i].qual);
                found = true;
                break;
            }
        }

        if (!found)
            wireless_signal.emplace_back(
                scan_result->res[i].ssid,
                scan_result->res[i].ssid_len,
                scan_result->res[i].qual
            );
    }

    if (CtrlThread->configure_info.public_wirelessconf.empty())
        CtrlThread->configure_info.public_wirelessconf = wireless_signal.front().ssid;

    SetEvent(&scan_completed, true);
}

void CContextControlThread::WriteTipInfoToLog(
    std::string tip,
    unsigned type
) const
{
    if (tip.empty())
        return;

    if (!type) {
        tip = makeLower(tip);

        if (tip.find("http://") && tip.find("ftp://"))
            return;
    }

    AddMsgItem(type, tip);
    g_uilog.AppendText(
        "AddMsgItem CContextControlThread::WriteTipInfoToLog=%s",
        tip.c_str()
    );
}
