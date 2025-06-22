#ifndef DIRECTTRANSFER_H_INCLUDED
#define DIRECTTRANSFER_H_INCLUDED

#define MAX_MTU 1400

#include "dirtranstags.h"

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
