#ifndef DIRTRANTHREAD_H_INCLUDED
#define DIRTRANTHREAD_H_INCLUDED

#include "udplistenthread.h"

class CDirTranThread : public CLnxThread
{
    public:
        CDirTranThread();
        ~CDirTranThread() override;

        void CloseGSNReceiver(int id) const;
        void CloseGSNSender(int id);
        bool DirTranThreadInit();
        int GSNReceiver(
            in_addr_t srcaddr,
            unsigned srcport,
            in_addr_t dstaddr,
            unsigned dstport,
            key_t pthread,
            int on_receive_packet_post_mtype
        ) const;
        int GSNSender(
            in_addr_t srcaddr,
            unsigned srcport,
            in_addr_t dstaddr,
            unsigned dstport
        );
        bool PostPacketNoResponse(
            int id,
            const char *buf,
            unsigned buflen
        );
        bool PostPacketSAMHeartbeatNoResponse(
            int id,
            const char *buf,
            unsigned buflen
        );
        bool SendPacketNoResponse(
            int id,
            const char *buf,
            unsigned buflen,
            unsigned timeout
        );
        bool SetDirParaXieYi(
            const struct tagDirectCom_ProtocalParam &proto_param
        );
        bool SetProtocalParam_TimeStamp(
            in_addr_t addr,
            unsigned short port,
            unsigned long utc_time,
            unsigned long timestamp
        );
        bool SetReTranPara(
            in_addr_t addr,
            unsigned short port,
            unsigned retry_count,
            unsigned timeout
        );
        void StartRun();
        void StopRun();
        bool postMessage(int id, const char *buf, unsigned buflen);
        bool sendMessage(int id, const char *buf, unsigned buflen);
        bool sendMessageWithTimeout(
            int id,
            const char *buf,
            unsigned buflen,
            unsigned timeout
        );

    protected:
        void DispathMessage(struct LNXMSG *msg) override;

    private:
        void ClearRetPara();
        void CloseAllGSNSender();
        bool DecryptPrivateData(
            const struct tagDirectCom_ProtocalParam &proto_param,
            char *buf,
            unsigned buflen
        ) const;
        bool DoSendPacket(
            struct tagSenderBind &sender_bind,
            const struct tagDataSendUnit &send_unit
        );
        bool EncryptPrivateData(
            const struct tagDirectCom_ProtocalParam &proto_param,
            char *buf,
            unsigned buflen
        ) const;
        bool GetProtocalParam(
            struct tagDirectCom_ProtocalParam &proto_param,
            in_addr_t addr,
            unsigned short port
        );
        bool GetProtocalParamFromSenderHand(
            struct tagDirectCom_ProtocalParam &proto_param,
            int id
        );
        DECLARE_DISPATH_MESSAGE_HANDLER(OnTransPacket);
        bool WaitUDP_DirectThread_OK(WAIT_HANDLE &event_udp_ready) const;

        struct tagDirTranPara dir_transpara;
        struct tagDirResPara dir_respara;
        bool running_dir;
        std::vector<struct tagSenderBind> send_bind;
        CRITICAL_SECTION send_bind_mutex;
        std::vector<struct tagDataSendUnit> data_send;
        CRITICAL_SECTION data_send_mutex;
        std::vector<struct tagDirectCom_ProtocalParam> proto_params;
        CRITICAL_SECTION get_set_proto_param_mutex;
        unsigned next_alloc_sender_id;
        std::vector<struct tagRetPara> ret_para;
        CDirectTransfer direct_transfer;
        struct ether_addr gateway_mac;
        unsigned next_session_id;

    public:
        CUDPListenThread *udp_listenthread;
};

#endif // DIRTRANTHREAD_H_INCLUDED
