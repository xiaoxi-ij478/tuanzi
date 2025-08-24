#ifndef DIRECTTRANSRV_H_INCLUDED
#define DIRECTTRANSRV_H_INCLUDED

#include "dirtranthread.h"

class CDirectTranSrv : public CLnxThread
{
    public:
        CDirectTranSrv();
        ~CDirectTranSrv() override;

        bool DeInit_Sam();
        bool DeInit_Smp();
        bool Init_Sam(struct tagDirectTranSrvPara *dir_srv_para, bool wait);
        bool Init_Smp(struct tagSmpParaDir *smp_para, bool wait);
        bool PostToSam(const char *buf, unsigned buflen) const;
        bool PostToSamWithNoResponse(const char *buf, unsigned buflen) const;
        bool PostToSmp(const char *buf, unsigned buflen) const;
        bool PostToSmpWithNoResponse(const char *buf, unsigned buflen) const;
        bool SendToSam(const char *buf, unsigned buflen, unsigned timeout) const;
        bool SendToSamWithNoResponse(
            const char *buf,
            unsigned buflen,
            unsigned timeout
        ) const;
        bool SendToSmp(const char *buf, unsigned buflen, unsigned timeout) const;
        bool SendToSmpWithNoResponse(
            const char *buf,
            unsigned buflen,
            unsigned timeout
        ) const;

    protected:
        void DispathMessage(struct LNXMSG *msg) override;
        bool ExitInstance() override;
        bool InitInstance() override;
        void OnTimer(int tflag) override;

    private:
        bool AnalyzePrivate_SAM(const char *buf, unsigned buflen) const;
        bool AnalyzePrivate_SMP(const char *buf, unsigned buflen);
        bool AskForSmpInitData();
        bool DeInitDirectEnvironment();
        void Destroy();
        void DoWithAuthResult(bool should_do);
        unsigned long GetSMPTimestamp() const;
        int GetXmlChild_Node_INT(
            const TiXmlNode *node,
            const std::string &value,
            const std::string &
        ) const;
        std::string GetXmlChild_Node_STR(
            const TiXmlNode *node,
            const std::string &value,
            const std::string &
        ) const;
        bool HandshakeToSAM();
        bool HandshakeToSMP();
        bool InitDirThread();
        bool InitDirectEnvironment();
        DECLARE_DISPATH_MESSAGE_HANDLER(OnRecvPacket_SAM) const;
        DECLARE_DISPATH_MESSAGE_HANDLER(OnInit_SAM);
        DECLARE_DISPATH_MESSAGE_HANDLER(OnDeInit_SAM);
        DECLARE_DISPATH_MESSAGE_HANDLER(OnRecvPacket_SMP);
        DECLARE_DISPATH_MESSAGE_HANDLER(OnInit_SMP);
        DECLARE_DISPATH_MESSAGE_HANDLER(OnDeInit_SMP);
        DECLARE_DISPATH_MESSAGE_HANDLER(OnPost_SMP) const;
        DECLARE_DISPATH_MESSAGE_HANDLER(OnPost_SAM) const;
        DECLARE_DISPATH_MESSAGE_HANDLER(OnPostNoResponse_SMP) const;
        DECLARE_DISPATH_MESSAGE_HANDLER(OnPostNoResponse_SAM) const;
        DECLARE_DISPATH_MESSAGE_HANDLER(OnTimer);
        void ParseACLParam(const char *buf, unsigned buflen) const;
        void ParseDHCPAuthResult_ForSAM(
            const char *buf,
            unsigned buflen,
            unsigned &pos
        ) const;
        void ParseDHCPAuthResult_ForSMP(const char *buf, unsigned buflen);
        void ParseGetHIStatusNow(const char *buf, unsigned buflen) const;
        void ParseGetHostInfoNow(const char *buf, unsigned buflen) const;
        void ParseLogoff(const char *buf, unsigned buflen) const;
        void ParseLogoffOhers(const char *buf, unsigned buflen) const;
        void ParseModifyPWResult(const char *buf, unsigned buflen) const;
        void ParseMsgAndPro(const char *buf, unsigned buflen) const;
        void ParsePasswordSecurity(const TiXmlDocument &xml) const;
        void ParseRM_Assist(const char *buf, unsigned buflen) const;
        void ParseReAuth(const char *buf, unsigned buflen) const;
        void ParseSMPData(const char *buf, unsigned buflen);

        bool sam_or_smp_inited;
        CRITICAL_SECTION destroy_mutex;

    public:
        CDirTranThread *dir_thread;

    private:
        struct tagSmpParaDir dir_smp_para;
        struct tagDirectTranSrvPara dir_trans_srvpara;
        timer_t sam_init_timerid;
        int sam_gsn_receiver_id;
        unsigned long sam_utc_time;
        unsigned long sam_timestamp;
        timer_t hello_timer;
        int smp_gsn_receiver_id;
        unsigned long smp_utc_time;
        unsigned long smp_timestamp;
        int handshake_gsn_sender_id;
        timer_t smp_init_timerid;
        unsigned timer_ask;
        int smp_init_data_gsn_sender_id;
        struct tagSmpInitPacket smp_init_packet;
};

#endif // DIRECTTRANSRV_H_INCLUDED
