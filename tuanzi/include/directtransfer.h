#ifndef DIRECTTRANSFER_H_INCLUDED
#define DIRECTTRANSFER_H_INCLUDED

#define MAX_MTU 1400

struct tagDirTranPara {
    char dstaddr[16];
    unsigned short dstport;
    char srcaddr[16];
    unsigned short srcport;
    struct ether_addr dstmacaddr;
    struct ether_addr srcmacaddr;
    unsigned char data[MAX_MTU];
    unsigned mtu;
    unsigned field_5D4;
};

class CDirectTransfer
{
    public:
        CDirectTransfer();
        virtual ~CDirectTransfer();

        bool InitPara(const struct tagDirTranPara *para);
        bool Send(const void *buf, unsigned buflen);
        bool SendLast() const;

    private:
        bool sendudp(
            const char *srcaddr,
            const char *dstaddr,
            unsigned short srcport,
            unsigned short dstport,
            const void *buf,
            unsigned buflen
        );

        static bool DescryptForSAM(unsigned char *buf, unsigned buflen);
        static bool EncryptForSAM(unsigned char *buf, unsigned buflen);

        struct tagDirTranPara dir_tran_para;
        bool packet_sent;
        unsigned char last_sent_packet[2048];
        unsigned last_sent_packet_len;
};

#endif // DIRECTTRANSFER_H_INCLUDED
