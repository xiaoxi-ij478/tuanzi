#ifndef UDPLISTENTHREAD_H_INCLUDED
#define UDPLISTENTHREAD_H_INCLUDED

#include "lnxthread.h"
#include "criticalsection.h"
#include "directtransfer.h"
#include "dirtranstags.h"

#define RECV_PACKET_RETURN_MTYPE 0x7D1
#define TIMER_CLEAR_DIR_SENDER 0x6F

struct UdpListenParam {
    key_t mainthread;
    in_addr_t su_ipaddr;
    char ndisname[512];
    WAIT_HANDLE event_udp_ready;
};

class CUDPListenThread : public CLnxThread
{
    public:
        CUDPListenThread(struct UdpListenParam *listen_param = nullptr);
        ~CUDPListenThread() override;

        int GSNReceiver(
            in_addr_t srcaddr,
            unsigned srcport,
            in_addr_t dstaddr,
            unsigned dstport,
            key_t pthread,
            int on_receive_packet_post_mtype
        );

    protected:
        bool DispathMessage(struct LNXMSG *msg) override;
        bool ExitInstance() override;
        bool InitInstance() override;
        void OnTimer(int tflag) const override;

    private:
        bool CloseGSNReceiver(int id);
        bool DecryptPrivateData(
            const struct tagDirectCom_ProtocalParam &proto_param,
            unsigned char *buf,
            unsigned buflen
        ) const;
        bool EncryptPrivateData(
            const struct tagDirectCom_ProtocalParam &proto_param,
            unsigned char *buf,
            unsigned buflen
        ) const;
        unsigned long GetLastTimeStampForReceive(
            in_addr_t addr,
            unsigned short port
        );
        unsigned long GetNextTimeStampForSend(
            in_addr_t addr,
            unsigned short port
        );
        unsigned GetOutOfOrderNum(in_addr_t addr, unsigned short port);
        bool GetProtocalParam(
            struct tagDirectCom_ProtocalParam &proto_param,
            in_addr_t addr,
            unsigned short port
        );
        bool HandlePrivateData(
            const unsigned char * /* struct DirTransFullPkg * */ buf,
            unsigned buflen
        );
        void InitTimeStampV2(
            in_addr_t addr,
            unsigned short port,
            unsigned long next_timestamp_for_send /* nextTimeStampForSend */
        );
        bool IsGoodAsyUTC(
            const struct tagDirectCom_ProtocalParam &proto_param,
            unsigned long timestamp
        );
        bool IsIPHeadChecksumRight(const unsigned char *buf, unsigned buflen) const;
        bool IsSuExpected(const unsigned char *buf, unsigned buflen) const;
        bool IsUDPChecksumRight(const unsigned char *buf, unsigned buflen) const;
        bool IsUDPPacket(const unsigned char *buf, unsigned buflen) const;
        void OnRecvPacketReturn(unsigned long buflen, const void *buf);
        void OnTimer(unsigned long buflen, void *buf);
        bool ResponseSender(const unsigned char *buf, unsigned buflen);
        bool RevcDirectPack(const unsigned char *buf, unsigned buflen);
        void SendResponse(
            const struct tagDirectCom_ProtocalParam &proto_param,
            struct tagDirPacketHead &packet_head,
            in_addr_t dstaddr,
            unsigned dstport,
            in_addr_t srcaddr,
            unsigned srcport,
            const unsigned char *packet
        );
        void SetDirParaXieYi(const tagDirectCom_ProtocalParam &proto_param);
        void SetIfListenRes(bool val);
        void SetLastTimeStampForReceive(
            in_addr_t addr,
            unsigned short port,
            unsigned long timestamp
        );
        void SetListenPort(unsigned short port);
        void SetMainThread(key_t mainthread_l);
        void SetNDISName(const char *ndisname_l);
        void SetOutOfOrderNum(
            in_addr_t addr,
            unsigned short port,
            unsigned outoforder_num
        );
        bool SetProtocalParam_TimeStamp(
            in_addr_t addr,
            unsigned short port,
            unsigned long utc_time,
            unsigned long timestamp
        );
        void SetResSender(const struct tagDirResPara &res_sender);
        void SetSamIPAddress(in_addr_t sam_ipaddr_l);
        void SetSuIPAddress(in_addr_t su_ipaddr_l);
        void SetWorkingFalg(bool working);
        void freeMemory();

        static unsigned short CheckSumForRecv(
            const unsigned char *buf,
            unsigned buflen
        );

        key_t mainthread;
        unsigned short listen_port;
        in_addr_t sam_ipaddr;
        in_addr_t su_ipaddr;
        char ndisname[512];
        WAIT_HANDLE *event_udp_ready;
        std::vector<struct tagTimeStampV2> timestamps;
        CRITICAL_SECTION timestamp_mutex;
        struct tagDirResPara dir_para;
        bool working_falg;
        bool listen_res;
        std::vector<struct tagDirectCom_ProtocalParam> proto_params;
        CRITICAL_SECTION get_set_proto_param_mutex;
        std::vector<struct tagRecvBind> gsn_pkgs;
        CRITICAL_SECTION recv_mutex;
        unsigned gsn_pkgid;
        CDirectTransfer dir_trans;
        timer_t clear_timer_id;
};

#endif // UDPLISTENTHREAD_H_INCLUDED
