#include "all.h"
#include "util.h"
#include "netutil.h"
#include "dirtransutil.h"
#include "directtransfer.h"

CDirectTransfer::CDirectTransfer() :
    packet_sent(),
    last_sent_packet(),
    last_sent_packet_len()
{}

CDirectTransfer::~CDirectTransfer()
{}

bool CDirectTransfer::InitPara(const struct tagDirTranPara *para)
{
    if (para)
        CopyDirTranPara(&dir_tran_para, para);

    return para;
}

bool CDirectTransfer::Send(const void *buf, unsigned buflen)
{
    return sendudp(
               dir_tran_para.srcaddr,
               dir_tran_para.dstaddr,
               dir_tran_para.srcport,
               dir_tran_para.dstport,
               buf,
               buflen
           );
}

bool CDirectTransfer::SendLast() const
{
    if (!packet_sent)
        return false;

    if (!CtrlThread->send_packet_thread)
        return true;

    CtrlThread->send_packet_thread->SendPacket(
        last_sent_packet,
        last_sent_packet_len
    );
    return true;
}

bool CDirectTransfer::sendudp(
    const char *srcaddr,
    const char *dstaddr,
    unsigned short srcport,
    unsigned short dstport,
    const void *buf,
    unsigned buflen
)
{
    struct [[gnu::packed]] {
        struct etherudppkg header;
        char data[2048];
    } tmpbuf;
    unsigned ipv4_len = 0, udp_len = 0;
    *reinterpret_cast<struct ether_addr *>
    (tmpbuf.header.etherheader.ether_shost) = dir_tran_para.srcmacaddr;
    *reinterpret_cast<struct ether_addr *>
    (tmpbuf.header.etherheader.ether_dhost) = dir_tran_para.dstmacaddr;
    tmpbuf.header.etherheader.ether_type = htons(ETHERTYPE_IP);
    ipv4_len = InitIpv4Header(&tmpbuf.header.ipheader, srcaddr, dstaddr, buflen);
    udp_len = InitUdpHeader(&tmpbuf.header.udpheader, srcport, dstport, buflen);
    ComputeUdpPseudoHeaderChecksumV4(
        &tmpbuf.header.ipheader,
        &tmpbuf.header.udpheader,
        static_cast<const char *>(buf),
        buflen
    );
    memcpy(&tmpbuf.data, buf, buflen);

    if (CtrlThread->send_packet_thread)
        CtrlThread->send_packet_thread->SendPacket(
            &tmpbuf,
            sizeof(struct etherudppkg) + buflen
        );

    else
        g_logSystem.AppendText("CtrlThread->m_sendPacketThread=NULL");

    packet_sent = true;
    memcpy(
        last_sent_packet,
        &tmpbuf,
        last_sent_packet_len = sizeof(struct etherudppkg) + buflen
    );
    return true;
}

#define DESKEY reinterpret_cast<const unsigned char *>("|sS1&@8q")
#define INIT_XORKEY { 'A', 'c', '3', '#', '1', '!', 'Q', 'd' }

bool CDirectTransfer::DescryptForSAM(char *buf, unsigned buflen)
{
    unsigned char tmpibuf[8] = {}, tmpobuf[8] = {};
    char xorkey[8] = INIT_XORKEY;

    if (buflen & 7)
        return false;

    deskey(DESKEY, DE1);

    for (unsigned i = 0; i < buflen >> 3; i++) {
        memcpy(tmpibuf, buf, 8);
        des(tmpibuf, tmpobuf);

        for (unsigned j = 0; j < 8; j++)
            tmpobuf[j] ^= xorkey[j];

        memcpy(buf, tmpobuf, 8);
        buf += 8;
        memcpy(xorkey, tmpibuf, 8);
    }

    return true;
}

bool CDirectTransfer::EncryptForSAM(char *buf, unsigned buflen)
{
    unsigned char tmpibuf[8] = {}, tmpobuf[8] = {};
    char xorkey[8] = INIT_XORKEY;

    if (buflen & 7)
        return false;

    deskey(DESKEY, EN0);

    for (unsigned i = 0; i < buflen >> 3; i++) {
        memcpy(tmpibuf, buf, 8);

        for (unsigned j = 0; j < 8; j++)
            tmpibuf[j] ^= xorkey[j];

        des(tmpibuf, tmpobuf);
        memcpy(buf, tmpobuf, 8);
        buf += 8;
        memcpy(xorkey, tmpobuf, 8);
    }

    return true;
}
