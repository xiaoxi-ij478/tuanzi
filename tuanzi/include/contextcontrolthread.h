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

struct tagWirelessSignal {
    tagWirelessSignal() = default;
    tagWirelessSignal(char *ssid_l, unsigned ssid_len, unsigned qual) :
        ssid_len(ssid_len), qual(qual)
    // *INDENT-OFF*
    {
    // *INDENT-ON*
        memcpy(ssid, ssid_l, ssid_len);
    }
    char ssid[33];
    unsigned ssid_len;
    unsigned qual;
};

class CContextControlThread : public CLnxThread
{
    public:
        CContextControlThread();
        ~CContextControlThread() override;

    protected:
        bool InitInstance() override;
        bool ExitInstance() override;
        void DispathMessage(struct LNXMSG * msg) override;
        void OnTimer(int tflag) const override;

    private:
        unsigned Authenticate_InitAll() const;
        bool BeginStart() const;
        void CancelBOOTP() const;
        void CancelWaitDHCPAuthResultTimer() const;
        bool CheckSSAMessPacket(char *buf, unsigned buflen) const;
        void CheckSelf() const;
        void ConnectClientCenter() const;
        void DeAuthenticate_ExitAll() const;
        void DeinitAll_Success() const;
        void DoBOOTP() const;
        void DoForSTATE_AUTHENTICATED() const;
        void DoUpgrade(
            unsigned version,
            std::string url,
            unsigned type
        ) const;
        void DoWithDHCPUpload() const;
        bool DoWithGetDHCPInfo() const;
        void DoWithSendUserinfoSuccess() const;
        int EnvironmentCheck() const;
        void ExitExtance_ExitAll() const;
        bool GetAdapterDHCPEnable() const;
        void GetAdapterMac(struct ether_addr * dst) const;
        unsigned GetDHCPAuthStep() const;
        void GetDHCPInfoParam(struct DHCPIPInfo & dst) const;
        in_addr_t GetNetOrderIPV4() const;
        unsigned GetRadiusServer() const;
        std::string GetSAMMessBuff(char *buf, unsigned buflen) const;
        bool IS_EAP_TYPE(enum EAP_TYPE_RFC type) const;
        bool IS_WIRED(enum EAP_TYPE_RFC type) const;
        bool IS_WLAN(enum EAP_TYPE_RFC type) const;
        void InitAll_Success(const struct SuRadiusPrivate & private_prop) const;
        void InitCheckSelf() const;
        bool InitInstance_InitAll() const;
        bool InitNICDevice() const;
        void InitStartDstMac() const;
        bool IsDhcpAuth() const;
        void IsIPOffered() const;
        bool IsRuijieNas() const;
        bool IsServerlistUpdate(const std::vector<std::string> & new_list) const;
        bool IsWirelessAuth() const;
        void KillDirectSrv() const;
        unsigned ModifyLogoffReason(enum LOGOFF_REASON & reason) const;
        DECLARE_DISPATH_MESSAGE_HANDLER(ONSAMWantLogOff) const;
        DECLARE_DISPATH_MESSAGE_HANDLER(ONSAWantLogOff) const;
        DECLARE_DISPATH_MESSAGE_HANDLER(OnAdaptersState) const;
        DECLARE_DISPATH_MESSAGE_HANDLER(OnConnectNotify) const;
        DECLARE_DISPATH_MESSAGE_HANDLER(OnLogoffWithUnknownReason) const;
        DECLARE_DISPATH_MESSAGE_HANDLER(OnOpenSSOURL) const;
        DECLARE_DISPATH_MESSAGE_HANDLER(OnOthersWantLogOff) const;
        DECLARE_DISPATH_MESSAGE_HANDLER(OnPacketReturn) const;
        DECLARE_DISPATH_MESSAGE_HANDLER(OnPatchLogoff) const;
        DECLARE_DISPATH_MESSAGE_HANDLER(OnRcvDHCPAuthResult) const;
        DECLARE_DISPATH_MESSAGE_HANDLER(OnRcvProxyDetectResult) const;
        DECLARE_DISPATH_MESSAGE_HANDLER(OnRecvFailure) const;
        DECLARE_DISPATH_MESSAGE_HANDLER(OnSaWantReAuth) const;
        DECLARE_DISPATH_MESSAGE_HANDLER(OnSayHello) const;
        DECLARE_DISPATH_MESSAGE_HANDLER(OnSaySaEvent) const;
        DECLARE_DISPATH_MESSAGE_HANDLER(OnShowSaMessage) const;
        DECLARE_DISPATH_MESSAGE_HANDLER(OnSimulateLogoff) const;
        DECLARE_DISPATH_MESSAGE_HANDLER(OnStartMachine) const;
        DECLARE_DISPATH_MESSAGE_HANDLER(OnStateMachineReturn) const;
        DECLARE_DISPATH_MESSAGE_HANDLER(OnTimer) const;
        DECLARE_DISPATH_MESSAGE_HANDLER(OnUpGradeReturn) const;
        DECLARE_DISPATH_MESSAGE_HANDLER(OnUpgradeClient) const;
        void OnShowLoginURL() const;
        bool RefreshSignal(const std::string & adapter_name) const;
        void SET_EAP_TYPE() const;
        void SET_IF_TYPE() const;
        bool SSAMessPrivParse(char *buf, unsigned buflen) const;
        void SaveRadiusPrivate(struct EAPOLFrame * eapol_frame) const;
        void SendLogOffPacket(enum LOGOFF_REASON logoff_reason, bool) const;
        bool SendUserinfoToAuthSvr() const;
        bool SendUserinfoToAuthSvr_ForSAM() const;
        bool SendUserinfoToAuthSvr_ForSMP() const;
        void SetConnectWindowText(enum OP_STATE op_state) const;
        void SetInitFailed() const;
        void SetNasManufacturer(bool is_ruijie_nas_l) const;
        void SetRadiusServer(unsigned type) const;
        void SetWaitDHCPAuthResultTimer() const;
        bool StartAdapterStateCheck() const;
        void StartDirectTrans(
            const struct SuRadiusPrivate & private_prop,
            bool wait,
            bool request_init_data_now
        ) const;
        void StartProcessBusiness(
            const struct SuRadiusPrivate & private_prop
        ) const;
        int StartStateMachine(bool no_get_dhcpinfo) const;
        bool StopAdapterStateCheck() const;
        void StopAuthentication(
            enum LOGOFF_REASON logoff_reason,
            enum APP_QUIT_TYPE quit_type,
            bool logoff_if_required
        ) const;
        void SuccessNotification(const std::string & notif_str) const;
        bool WlanConnect() const;
        bool WlanDisconnect() const;
        void WlanScanComplete(struct ScanCmdCtx * scan_result) const;
        void WriteTipInfoToLog(std::string tip, unsigned type) const;

        bool auth_inited;
        char field_1D1;
        std::vector<struct tagWirelessSignal> wireless_signal;
        char *field_1F0;
        char *field_1F8;
        CRxPacketThread * read_packet_thread;
        CSendPacketThread * send_packet_thread;
        CStateMachineThread * machine_thread;
        CDirectTranSrv * dir_tran_srv;
        CDownLoadThread * download_thread;
        CHelloProcessor * hello_processor;
        CAdapterDetectThread * adapter_detect_thread;
        CProxyDetectThread * proxy_detect_thread;
        struct SaveConfigureInfo configure_info;
        char *field_400;
        struct SuRadiusPrivate private_properties;
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
        std::string sec_domain_name;
        std::vector<std::string> nic_in_use;
        bool field_538;
        bool reconnect_fail;
        bool connect_fail;
        bool field_53B;
        std::string logoff_message;
        std::string success_info;
        struct ether_addr dstaddr;
        struct ether_addr start_dst_addr;
        struct ether_addr old_src_macaddr;
        timer_t reconnect_timer;
        char field_570[3000];
        bool process_business_started;
        bool field_1129;
        bool field_112A;
        std::string diskid;
        bool field_1138;
        bool field_1139;
        bool service_list_updated;
        std::string service_name;
        bool has_auth_success;
        bool field_1149;
        enum STATES state;
        unsigned field_1150;
        timer_t wait_dhcp_auth_result_timerid;
        timer_t open_sso_url_timer;
        std::string upgrade_url;
        WAIT_HANDLE scan_completed;
        char field_11D0[32];
};

#endif // CONTEXTCONTROLTHREAD_H_INCLUDED
