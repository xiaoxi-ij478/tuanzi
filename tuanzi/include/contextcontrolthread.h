#ifndef CONTEXTCONTROLTHREAD_H_INCLUDED
#define CONTEXTCONTROLTHREAD_H_INCLUDED

#include "lnxthread.h"
#include "saveconfigureinfo.h"
#include "suradiusprivate.h"
#include "supf.h"

class CRxPacketThread;
class CSendPacketThread;
class CStateMachineThread;
class CDirectTranSrv;
class CDownLoadThread;
class CHelloProcessor;
class CAdapterDetectThread;
class CProxyDetectThread;
struct tagWirelessSignal;

class CContextControlThread : public CLnxThread
{
    public:
        CContextControlThread();
        ~CContextControlThread() override;

        void GetAdapterMac(struct ether_addr *dst) const;
        unsigned GetDHCPAuthStep();
        void GetDHCPInfoParam(struct DHCPIPInfo &dst) const;
        in_addr_t GetNetOrderIPV4() const;
        unsigned GetRadiusServer() const;
        void InitStartDstMac();
        bool IS_EAP_TYPE(enum EAP_TYPE_RFC type) const;
        bool IS_WIRED(enum EAP_TYPE_RFC type) const;
        bool IS_WLAN(enum EAP_TYPE_RFC type) const;
        bool IsDhcpAuth() const;
        bool IsRuijieNas() const;
        bool IsServerlistUpdate(const std::vector<std::string> &new_list) const;
        bool RefreshSignal(const std::string &adapter_name);
        void SaveRadiusPrivate(const struct EAPOLFrame *eapol_frame);
        void SetRadiusServer(unsigned type);
        void WlanScanComplete(const struct ScanCmdCtx *scan_result);

    protected:
        bool InitInstance() override;
        bool ExitInstance() override;
        void DispathMessage(struct LNXMSG *msg) override;
        void OnTimer(int tflag) override;

    private:
        unsigned Authenticate_InitAll();
        bool BeginStart();
        void CancelBOOTP();
        void CancelWaitDHCPAuthResultTimer();
        bool CheckSSAMessPacket(char *buf, unsigned buflen) const;
        void CheckSelf() const;
        void ConnectClientCenter() const;
        void DeAuthenticate_ExitAll();
        void DeinitAll_Success();
        void DoBOOTP();
        void DoForSTATE_AUTHENTICATED();
        void DoUpgrade(
            unsigned version,
            std::string url,
            unsigned type
        );
        void DoWithDHCPUpload();
        bool DoWithGetDHCPInfo();
        void DoWithSendUserinfoSuccess();
        int EnvironmentCheck();
        void ExitExtance_ExitAll();
        bool GetAdapterDHCPEnable() const;
        std::string GetSAMMessBuff(char *buf, unsigned buflen) const;
        void InitAll_Success(const struct SuRadiusPrivate &private_prop);
        void InitCheckSelf() const;
        bool InitInstance_InitAll();
        bool InitNICDevice();
        void IsIPOffered();
        bool IsWirelessAuth() const;
        void KillDirectSrv();
        unsigned ModifyLogoffReason(enum LOGOFF_REASON &reason) const;
        DECLARE_DISPATH_MESSAGE_HANDLER(ONSAMWantLogOff);
        DECLARE_DISPATH_MESSAGE_HANDLER(ONSAWantLogOff);
        DECLARE_DISPATH_MESSAGE_HANDLER(OnAdaptersState);
        DECLARE_DISPATH_MESSAGE_HANDLER(OnConnectNotify);
        DECLARE_DISPATH_MESSAGE_HANDLER(OnLogoffWithUnknownReason);
        DECLARE_DISPATH_MESSAGE_HANDLER(OnOpenSSOURL);
        DECLARE_DISPATH_MESSAGE_HANDLER(OnOthersWantLogOff);
        DECLARE_DISPATH_MESSAGE_HANDLER(OnPacketReturn) const;
        DECLARE_DISPATH_MESSAGE_HANDLER(OnPatchLogoff);
        DECLARE_DISPATH_MESSAGE_HANDLER(OnRcvDHCPAuthResult);
        DECLARE_DISPATH_MESSAGE_HANDLER(OnRcvProxyDetectResult);
        DECLARE_DISPATH_MESSAGE_HANDLER(OnRecvFailure);
        DECLARE_DISPATH_MESSAGE_HANDLER(OnSaWantReAuth);
        DECLARE_DISPATH_MESSAGE_HANDLER(OnSayHello) const;
        DECLARE_DISPATH_MESSAGE_HANDLER(OnSaySaEvent) const;
        DECLARE_DISPATH_MESSAGE_HANDLER(OnShowSaMessage) const;
        DECLARE_DISPATH_MESSAGE_HANDLER(OnSimulateLogoff) const;
        DECLARE_DISPATH_MESSAGE_HANDLER(OnStartMachine);
        DECLARE_DISPATH_MESSAGE_HANDLER(OnStateMachineReturn);
        DECLARE_DISPATH_MESSAGE_HANDLER(OnTimer);
        DECLARE_DISPATH_MESSAGE_HANDLER(OnUpGradeReturn);
        DECLARE_DISPATH_MESSAGE_HANDLER(OnUpgradeClient);
        void OnShowLoginURL() const;
        void SET_EAP_TYPE();
        void SET_IF_TYPE();
        bool SSAMessPrivParse(char *buf, unsigned buflen) const;
        void SendLogOffPacket(enum LOGOFF_REASON logoff_reason, bool) const;
        bool SendUserinfoToAuthSvr() const;
        bool SendUserinfoToAuthSvr_ForSAM() const;
        bool SendUserinfoToAuthSvr_ForSMP() const;
        void SetConnectWindowText(enum OP_STATE op_state) const;
        void SetInitFailed() const;
        void SetNasManufacturer(bool is_ruijie_nas_l);
        void SetWaitDHCPAuthResultTimer();
        bool StartAdapterStateCheck() const;
        void StartDirectTrans(
            const struct SuRadiusPrivate &private_prop,
            bool wait,
            bool request_init_data_now
        );
        void StartProcessBusiness(
            const struct SuRadiusPrivate &private_prop
        );
        int StartStateMachine(bool no_get_dhcpinfo);
        bool StopAdapterStateCheck() const;
        void StopAuthentication(
            enum LOGOFF_REASON logoff_reason,
            enum APP_QUIT_TYPE quit_type,
            bool logoff_if_required
        );
        void SuccessNotification(const std::string &notif_str) const;
        bool WlanConnect() const;
        bool WlanDisconnect() const;
        void WriteTipInfoToLog(std::string tip, unsigned type) const;

        bool auth_inited;
        char field_1D1;

    public:
        std::vector<struct tagWirelessSignal> wireless_signal;
        unsigned long field_1F0;
        char *field_1F8;

    public:
        CRxPacketThread *read_packet_thread;
        CSendPacketThread *send_packet_thread;
        CStateMachineThread *machine_thread;
        CDirectTranSrv *dir_tran_srv;
        CDownLoadThread *download_thread;
        CHelloProcessor *hello_processor;
        CAdapterDetectThread *adapter_detect_thread;
        CProxyDetectThread *proxy_detect_thread;
        struct SaveConfigureInfo configure_info;

    private:
        char *field_400;

    public:
        struct SuRadiusPrivate private_properties;

    private:
        bool is_ruijie_nas;
        unsigned radius_server;
        unsigned dhcp_auth_step;
        enum EAP_TYPES eap_type;
        unsigned if_type;
        timer_t bootp_timerid;
        unsigned ip_offer_count;
        unsigned field_4EC;
        sem_t bootp_semaphore;
        std::vector<char *> field_4F8;
        bool direct_trans;
        bool field_511;

    public:
        std::string sec_domain_name;
        std::vector<std::string> nic_in_use;
        bool field_538;
        bool reconnect_fail;
        bool connect_fail;

    public:
        bool field_53B;
        std::string logoff_message;
        std::string success_info;

    public:
        struct ether_addr dstaddr;
        struct ether_addr start_dst_addr;

    private:
        struct ether_addr old_src_macaddr;
        timer_t reconnect_timer;
        char field_570[3000];
        bool process_business_started;
        bool field_1129;
        bool field_112A;

    public:
        std::string diskid;
        bool field_1138;
        bool field_1139;
        bool service_list_updated;

    public:
        std::string service_name;

    public:
        bool has_auth_success;
        bool field_1149;

    private:
        enum STATES state;
        unsigned field_1150;
        timer_t wait_dhcp_auth_result_timerid;
        timer_t open_sso_url_timer;
        std::string upgrade_url;

    public:
        WAIT_HANDLE scan_completed;

    private:
        char field_11D0[32];
};

#endif // CONTEXTCONTROLTHREAD_H_INCLUDED
