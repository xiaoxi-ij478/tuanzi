#ifndef DIRTRANTHREAD_H_INCLUDED
#define DIRTRANTHREAD_H_INCLUDED

#include "lnxthread.h"
#include "dirtranstags.h"
#include "directtransfer.h"
#include "udplistenthread.h"
#include "criticalsection.h"

#define ON_TRANSPACKET_MTYPE 0xD9

class CDirTranThread : public CLnxThread
{
    public:
        CDirTranThread();
        ~CDirTranThread() override;

    protected:
        bool DispathMessage(struct LNXMSG *msg) override;

    private:
        void ClearRetPara() const;
        void CloseAllGSNSender() const;
        void CloseGSNReceiver(int id) const;
        void CloseGSNSender(int id) const;
        bool DecryptPrivateData(
            const struct tagDirectCom_ProtocalParam &proto_param,
            unsigned char *buf,
            unsigned buflen
        ) const;
        bool DirTranThreadInit() const;
        bool DoSendPacket(
            struct tagSenderBind &sender_bind,
            const struct tagDataSendUnit &send_unit
        ) const;
        bool EncryptPrivateData(
            const struct tagDirectCom_ProtocalParam &proto_param,
            unsigned char *buf,
            unsigned buflen
        ) const;
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
        ) const;
        bool GetProtocalParam(
            struct tagDirectCom_ProtocalParam &proto_param,
            in_addr_t addr,
            unsigned short port
        ) const;
        bool GetProtocalParamFromSenderHand(
            struct tagDirectCom_ProtocalParam &proto_param,
            int id
        ) const;
        void OnTransPacket(unsigned long buflen, void *buf) const;
        bool PostPacketNoResponse(
            int id,
            unsigned char *buf,
            unsigned buflen
        ) const;
        bool PostPacketSAMHeartbeatNoResponse(
            int id,
            unsigned char *buf,
            unsigned buflen
        ) const;

        bool SendPacketNoResponse(
            int id,
            unsigned char *buf,
            unsigned buflen,
            unsigned timeout
        ) const;
        void SetDirParaXieYi(
            const struct tagDirectCom_ProtocalParam &proto_param
        ) const;
        bool SetProtocalParam_TimeStamp(
            in_addr_t addr,
            unsigned short port,
            unsigned long utc_time,
            unsigned long timestamp
        ) const;
        bool SetReTranPara(
            in_addr_t addr,
            unsigned short port,
            unsigned retry_count,
            unsigned timeout
        ) const;
        void StartRun() const;
        void StopRun() const;
        bool WaitUDP_DirectThread_OK(WAIT_HANDLE &event_udp_ready) const;
        bool postMessage(
            int id,
            unsigned char *buf,
            unsigned buflen
        ) const;
        bool sendMessage(
            int id,
            unsigned char *buf,
            unsigned buflen
        ) const;
        bool sendMessageWithTimeout(
            int id,
            unsigned char *buf,
            unsigned buflen,
            unsigned timeout
        ) const;

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
        CUDPListenThread *udp_listenthread;
};

#endif // DIRTRANTHREAD_H_INCLUDED
