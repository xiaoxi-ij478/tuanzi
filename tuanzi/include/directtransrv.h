#ifndef DIRECTTRANSRV_H_INCLUDED
#define DIRECTTRANSRV_H_INCLUDED

#include "lnxthread.h"
#include "criticalsection.h"
#include "dirtranstags.h"
#include "dirtranthread.h"

class CDirectTranSrv : public CLnxThread
{
    public:
        CDirectTranSrv();
        ~CDirectTranSrv() override;

    protected:
        void DispathMessage(struct LNXMSG *msg) override;
        bool ExitInstance() override;
        bool InitInstance() override;
        void OnTimer(int tflag) const override;

    private:
        bool AnalyzePrivate_SAM(unsigned char *buf, unsigned buflen) const;
        bool AnalyzePrivate_SMP(unsigned char *buf, unsigned buflen) const;
        bool AskForSmpInitData() const;
        bool DeInitDirectEnvironment() const;
        bool DeInit_Sam() const;
        bool DeInit_Smp() const;
        void Destroy() const;
        void DoWithAuthResult(bool should_do) const;
        unsigned long GetSMPTimestamp() const;
        int GetXmlChild_Node_INT(
            TiXmlNode *node,
            const std::string &value,
            const std::string &
        ) const;
        std::string GetXmlChild_Node_STR(
            TiXmlNode *node,
            const std::string &value,
            const std::string &
        ) const;
        bool HandshakeToSAM() const;
        bool HandshakeToSMP() const;
        bool InitDirThread() const;
        bool InitDirectEnvironment() const;
        bool Init_Sam(struct tagDirectTranSrvPara *srv_para, bool wait) const;
        bool Init_Smp(struct tagSmpParaDir *srv_para, bool wait) const;
        void OnDeInit_SAM(unsigned long buflen, void *buf) const;
        void OnDeInit_SMP(unsigned long buflen, void *buf) const;
        void OnInit_SAM(unsigned long buflen, void *buf) const;
        void OnInit_SMP(unsigned long buflen, void *buf) const;
        void OnPostNoResponse_SAM(unsigned long buflen, void *buf) const;
        void OnPostNoResponse_SMP(unsigned long buflen, void *buf) const;
        void OnPost_SAM(unsigned long buflen, void *buf) const;
        void OnPost_SMP(unsigned long buflen, void *buf) const;
        void OnRecvPacket_SAM(unsigned long buflen, void *buf) const;
        void OnRecvPacket_SMP(unsigned long buflen, void *buf) const;
        void OnTimer(unsigned long buflen, void *buf) const;
        void ParseACLParam(unsigned char *buf, unsigned buflen) const;
        void ParseDHCPAuthResult_ForSAM(
            unsigned char *buf,
            unsigned buflen,
            int &next_buflen
        ) const;
        void ParseDHCPAuthResult_ForSMP(unsigned char *buf, unsigned buflen) const;
        void ParseGetHIStatusNow(unsigned char *buf, unsigned buflen) const;
        void ParseGetHostInfoNow(unsigned char *buf, unsigned buflen) const;
        void ParseLogoff(unsigned char *buf, unsigned buflen) const;
        void ParseLogoffOhers(unsigned char *buf, unsigned buflen) const;
        void ParseModifyPWResult(unsigned char *buf, unsigned buflen) const;
        void ParseMsgAndPro(unsigned char *buf, unsigned buflen) const;
        void ParsePasswordSecurity(TiXmlDocument &xml) const;
        void ParseRM_Assist(unsigned char *buf, unsigned buflen) const;
        void ParseReAuth(unsigned char *buf, unsigned buflen) const;
        void ParseSMPData(unsigned char *buf, unsigned buflen) const;
        bool PostToSam(unsigned char *buf, unsigned buflen) const;
        bool PostToSamWithNoResponse(unsigned char *buf, unsigned buflen) const;
        bool PostToSmp(unsigned char *buf, unsigned buflen) const;
        bool PostToSmpWithNoResponse(unsigned char *buf, unsigned buflen) const;
        bool SendToSam(unsigned char *buf, unsigned buflen, unsigned timeout) const;
        bool SendToSamWithNoResponse(
            unsigned char *buf,
            unsigned buflen,
            unsigned timeout
        ) const;
        bool SendToSmp(unsigned char *buf, unsigned buflen, unsigned timeout) const;
        bool SendToSmpWithNoResponse(
            unsigned char *buf,
            unsigned buflen,
            unsigned timeout
        ) const;

        bool sam_or_smp_inited;
        CRITICAL_SECTION destory_mutex;
        CDirTranThread *dir_thread;
        struct tagSmpParaDir dir_smp_para;
        bool field_360;
        bool field_361;
        bool request_init_data_now;
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
