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
    connecting(),
    field_53A(),
    field_53B(),
    logoff_message(),
    field_548(),
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
    field_1170(),
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
    private_properties.field_A4 = 0;
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
    SetEvent(&field_1170, true);
    CloseHandle(&field_1170);
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

void CContextControlThread::OnTimer(int tflag) const
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

unsigned CContextControlThread::Authenticate_InitAll() const
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
        platform_param.event_callback = supf_event_callback_fun;
        platform_param.debug_file = debug_file; /* nullptr */
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

    read_packet_thread->SetRxPacketAdapter(configure_info.public_adapter);

    if (read_packet_thread->StartRecvPacketThread() == -1)
        return 1;

    if (
        !send_packet_thread->SetSenderAdapter(
            CtrlThread->configure_info.public_adapter
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

bool CContextControlThread::BeginStart() const
{
    CtrlThread->GetAdapterMac(&old_src_macaddr);
    g_Logoff.AppendText(
        "CContextControlThread::BeginStart()---m_oldSrcMacAddr=%X-%X-%X-%X-%X-%X",
        old_src_macaddr.ether_addr_octet[0],
        old_src_macaddr.ether_addr_octet[1],
        old_src_macaddr.ether_addr_octet[2],
        old_src_macaddr.ether_addr_octet[3],
        old_src_macaddr.ether_addr_octet[5]
        old_src_macaddr.ether_addr_octet[4],
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

void CContextControlThread::CancelBOOTP() const
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

void CContextControlThread::CancelWaitDHCPAuthResultTimer() const
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

void CContextControlThread::DeAuthenticate_ExitAll() const
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
                    "CContextControlThread::DeAuthenticate_ExitAll TerminateThread m_readPa"
                    "cketThread->m_nThreadID=%x",
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

void CContextControlThread::DeinitAll_Success() const
{
    g_log_Wireless.AppendText("DeinitAll_Success ENTER");

    if (adapter_detect_thread)
        adapter_detect_thread->StopDetect(1);

    if (proxy_detect_thread)
        proxy_detect_thread->StopDetect();

    KillDirectSrv();

    if (hello_processor) {
        hello_processor->Exit();
        delete hello_processor
        hello_processor = nullptr;
    }
}

void CContextControlThread::DoBOOTP() const
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

    if (bootp_timerid = SetTimer(nullptr, PACKET_NOTIFY_MTYPE, 2000, nullptr)) {
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

void CContextControlThread::DoForSTATE_AUTHENTICATED() const
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
    theApp.GUI_update_connectdlg_by_states(APPSTATUS_SUCCESS);
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
) const
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
        download_para.save_path,
        download_para.save_filename
    );
}

void CContextControlThread::DoWithDHCPUpload() const
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

bool CContextControlThread::DoWithGetDHCPInfo() const
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
            dhcp_ipinfo.gateway >> 24,
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

void CContextControlThread::DoWithSendUserinfoSuccess() const
{
    SetWaitDHCPAuthResultTimer();
}

int CContextControlThread::EnvironmentCheck() const
{
    char tmpbuf[512] = {};
    char tmpbuf2[256] = {};
    bool network_manager_stopped = false;
    struct ifreq ifr = {};
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    std::vector<struct NICsStatus> nic_statuses;
    get_all_nics_statu(nics_status);

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
            std::string &&tmpstr = std::string(tmpbuf2).append(tmpbuf);
            g_log_Wireless.AppendText("%s", tmpstr.c_str());
            theApp.GUI_update_connect_text(tmpstr);

        } else
            network_manager_stopped = true;
    }

    if (IS_WLAN(RFC_EAP_NONE) && check_service_status("wpa_supplicant")) {
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
            std::string &&tmpstr = std::string(tmpbuf2).append(tmpbuf);
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
}

void CContextControlThread::ExitExtance_ExitAll() const
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

unsigned CContextControlThread::GetDHCPAuthStep() const
{
    if (IsRuijieNas() && IS_WIRED(RFC_EAP_MD5) && IsDhcpAuth()) {
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

bool CContextControlThread::IS_EAP_TYPE(enum EAP_TYPE_RFC type) const
{
    return eap_type == type;
}

bool CContextControlThread::IS_WIRED(enum EAP_TYPE_RFC type) const
{
    if (type == RFC_EAP_NONE)
        return if_type == 1;

    if (if_type == 1)
        return eap_type == type;

    return false;
}

bool CContextControlThread::IS_WLAN(enum EAP_TYPE_RFC type) const
{
    g_log_Wireless.AppendText("m_iftype=%d m_unEapType=%d", if_type, eap_type);

    if (type == RFC_EAP_NONE)
        return if_type == 2;

    if (if_type == 2)
        return eap_type == type;

    return false;
}

void CContextControlThread::InitAll_Success(
    const struct SuRadiusPrivate &private_prop
) const
{
    char errbuf[512] = {};
    struct ether_addr macaddr = {};
    bool disallow_multi_nic_ip = false;
    g_log_Wireless.AppendText("InitAll_Success ENTER");

    if (private_prop.parse_hello) {
        if (private_prop.parse_hello == -1) {
            if (hello_processor) {
                g_log_Wireless.AppendText("Hello disable");
                hello_processor->ProcessorStop();
            }

        } else if (private_prop.parse_hello == 1) {
            g_log_Wireless.AppendText("Hello Enable");

            if (!hello_processor)
                hello_processor = new CHelloProcessor;

            if (hello_processor)
                hello_processor->ProcessorRun(
                    GetMessageID(),
                    private_prop.parse_hello_inv,
                    private_prop.parse_hello_id
                );
        }
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
            "Start Proxy Detect,type=%08x"
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

bool CContextControlThread::InitInstance_InitAll() const
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

bool CContextControlThread::InitNICDevice() const
{
    g_log_Wireless.AppendText("Enter CContextControlThread::InitNICDevice");
    return GetNICInUse(nic_in_use, false);
}

void CContextControlThread::InitStartDstMac() const
{
    struct ether_addr macaddr_special1 = { 0x01, 0x80, 0xC2, 0x00, 0x00, 0x03 };
    struct ether_addr macaddr_special2 = { 0x01, 0xD0, 0xF8, 0x00, 0x00, 0x03 };
    start_addr =
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

void CContextControlThread::IsIPOffered() const
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
        !IsGetDhcpIpp(configure_info.dhcp_ipinfo.ip4_ipaddr) ||
        !IsGetDhcpIpp(configure_info.dhcp_ipinfo.gateway)
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
    return IS_WIRED(RFC_EAP_MD5) ? is_ruijie_nas : false;
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

void CContextControlThread::KillDirectSrv() const
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
    }

    if (IsRuijieNas() && reason != LOGOFF_REASON_NORMAL_LOGOFF)
        reason -= 100;

    return reason;
}

DEFINE_DISPATH_MESSAGE_HANDLER(ONSAMWantLogOff, CContextControlThread) const
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

DEFINE_DISPATH_MESSAGE_HANDLER(ONSAWantLogOff, CContextControlThread) const
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

DEFINE_DISPATH_MESSAGE_HANDLER(OnAdaptersState, CContextControlThread) const
{
    static in_addr_t gateway = 0; // szGateway
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    char speed_buf[100] = {};
    struct rtentry route = {};
    g_log_Wireless.AppendText("OnAdaptersState %d", arg1);

    switch (arg1) {
        case ADAPTER_UP_REPORT_MTYPE:
            if (IS_WLAN(RFC_EAP_NONE))
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

            if (get_nic_speed(speed_buf, configure_info.public_adapter)) {
                std::string &&speed_str =
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
            if (IS_WLAN(RFC_EAP_NONE))
                break;

            theApp.GUI_update_LOGOFF(LOGOFF_REASON_NIC_NOT_CONNECTED, APPSTATUS_6);
            StopAuthentication(LOGOFF_REASON_NORMAL_LOGOFF, APP_QUIT_TYPE_0, false);
            break;

        case ADAPTER_DISABLE_REPORT_MTYPE:
        case ADAPTER_ERROR_REPORT_MTYPE:
            gateway = configure_info.dhcp_ipinfo.gateway;
            theApp.GUI_update_LOGOFF(LOGOFF_REASON_NIC_DISABLED, APPSTATUS_6);

            if (IS_WLAN(RFC_EAP_NONE))
                WlanDisconnect();

            StopAuthentication(LOGOFF_REASON_NORMAL_LOGOFF, APP_QUIT_TYPE_0, false);
            break;

        case ADAPTER_ENABLE_REPORT_MTYPE:
            if (IS_WIRED(RFC_EAP_NONE))
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

DEFINE_DISPATH_MESSAGE_HANDLER(OnConnectNotify, CContextControlThread) const
{
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
            CBackoffReAuthenticationManager &instance =
                CBackoffReAuthenticationManager::Instance();

            if (instance.reauth_timer) {
                KillTimer(instance.reauth_timer);
                instance.reauth_timer = 0;
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
                        60000 * CtrlThread->configure_info.auto_reconnect,
                        nullptr
                    );

            break;
    }
}

DEFINE_DISPATH_MESSAGE_HANDLER(
    OnLogoffWithUnknownReason,
    CContextControlThread
) const
{
    g_log_Wireless.AppendText("CContextControlThread::OnLogoffWithUnknownReason()");
    StopAuthentication(LOGOFF_REASON_UNKNOWN_REASON, APP_QUIT_TYPE_2, true);
}

DEFINE_DISPATH_MESSAGE_HANDLER(OnOpenSSOURL, CContextControlThread) const
{
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

DEFINE_DISPATH_MESSAGE_HANDLER(OnOthersWantLogOff, CContextControlThread) const
{
    StopAuthentication(arg1, APP_QUIT_TYPE_2, arg2);
}

DEFINE_DISPATH_MESSAGE_HANDLER(OnPacketReturn, CContextControlThread) const
{
}

DEFINE_DISPATH_MESSAGE_HANDLER(OnPatchLogoff, CContextControlThread) const
{
}

DEFINE_DISPATH_MESSAGE_HANDLER(OnRcvDHCPAuthResult, CContextControlThread) const
{
}

DEFINE_DISPATH_MESSAGE_HANDLER(
    OnRcvProxyDetectResult,
    CContextControlThread
) const
{
}

DEFINE_DISPATH_MESSAGE_HANDLER(OnRecvFailure, CContextControlThread) const
{
}

DEFINE_DISPATH_MESSAGE_HANDLER(OnSaWantReAuth, CContextControlThread) const
{
}

DEFINE_DISPATH_MESSAGE_HANDLER(OnSayHello, CContextControlThread) const
{
}

DEFINE_DISPATH_MESSAGE_HANDLER(OnSaySaEvent, CContextControlThread) const
{
}

DEFINE_DISPATH_MESSAGE_HANDLER(OnShowSaMessage, CContextControlThread) const
{
}

DEFINE_DISPATH_MESSAGE_HANDLER(OnSimulateLogoff, CContextControlThread) const
{
}

DEFINE_DISPATH_MESSAGE_HANDLER(OnStartMachine, CContextControlThread) const
{
}

DEFINE_DISPATH_MESSAGE_HANDLER(
    OnStateMachineReturn,
    CContextControlThread
) const
{
}

DEFINE_DISPATH_MESSAGE_HANDLER(OnTimer, CContextControlThread) const
{
}

DEFINE_DISPATH_MESSAGE_HANDLER(OnUpGradeReturn, CContextControlThread) const
{
}

DEFINE_DISPATH_MESSAGE_HANDLER(OnUpgradeClient, CContextControlThread) const
{
}

void CContextControlThread::OnShowLoginURL() const
{
}

bool CContextControlThread::RefreshSignal(const std::string &adapter_name) const
{
}

void CContextControlThread::SET_EAP_TYPE() const
{
}

void CContextControlThread::SET_IF_TYPE() const
{
}

bool CContextControlThread::SSAMessPrivParse(char *buf, unsigned buflen) const
{
}

void CContextControlThread::SaveRadiusPrivate(struct EAPOLFrame *eapol_frame)
const
{
}

void CContextControlThread::SendLogOffPacket(
    enum LOGOFF_REASON logoff_reason,
    bool
) const
{
}

bool CContextControlThread::SendUserinfoToAuthSvr() const
{
}

bool CContextControlThread::SendUserinfoToAuthSvr_ForSAM() const
{
}

bool CContextControlThread::SendUserinfoToAuthSvr_ForSMP() const
{
}

void CContextControlThread::SetConnectWindowText(enum OP_STATE op_state) const
{
}

void CContextControlThread::SetInitFailed() const
{
}

void CContextControlThread::SetNasManufacturer(bool is_ruijie_nas_l) const
{
}

void CContextControlThread::SetRadiusServer(uint) const
{
}

void CContextControlThread::SetWaitDHCPAuthResultTimer() const
{
}

bool CContextControlThread::StartAdapterStateCheck() const
{
}

void CContextControlThread::StartDirectTrans(
    const struct SuRadiusPrivate &private_prop,
    bool wait,
    bool request_init_data_now
) const
{
}

void CContextControlThread::StartProcessBusiness(
    const struct SuRadiusPrivate &private_prop
) const
{
}

bool CContextControlThread::StartStateMachine(bool no_get_dhcpinfo) const
{
}

bool CContextControlThread::StopAdapterStateCheck() const
{
}

void CContextControlThread::StopAuthentication(
    enum LOGOFF_REASON logoff_reason,
    enum APP_QUIT_TYPE quit_type,
    bool logoff_if_required
) const
{
}

void CContextControlThread::SuccessNotification(
    const std::string &notif_str
) const
{
}

bool CContextControlThread::WlanConnect() const
{
}

bool CContextControlThread::WlanDisconnect() const
{
}

void CContextControlThread::WlanScanComplete(
    struct ScanCmdCtx *scan_result
) const
{
}

void CContextControlThread::WriteTipInfoToLog(
    std::string tip,
    unsigned type
) const
{
}
