#include "all.h"
#include "util.h"
#include "netutil.h"
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
    unsigned char tmpbuf[2048] = {};
    unsigned ipv4_len = 0, udp_len = 0;
    memcpy(tmpbuf, &dir_tran_para.srcmacaddr, sizeof(struct ether_addr));
    memcpy(
        tmpbuf + sizeof(struct ether_addr),
        &dir_tran_para.dstmacaddr,
        sizeof(struct ether_addr)
    );
    ipv4_len = InitIpv4Header(
                   tmpbuf + sizeof(struct ether_addr) * 2,
                   srcaddr,
                   dstaddr,
                   buflen
               );
    udp_len = InitUdpHeader(
                  tmpbuf + sizeof(struct ether_addr) * 2 + ipv4_len,
                  srcport,
                  dstport,
                  buflen
              );
    ComputeUdpPseudoHeaderChecksumV4(
        reinterpret_cast<struct iphdr *>(tmpbuf + sizeof(struct ether_addr) * 2),
        reinterpret_cast<struct udphdr *>
        (tmpbuf + sizeof(struct ether_addr) * 2 + ipv4_len),
        static_cast<const unsigned char *>(buf),
        buflen
    );
    memcpy(
        tmpbuf + sizeof(struct ether_addr) * 2 + ipv4_len + udp_len,
        buf,
        buflen
    );

    if (CtrlThread->send_packet_thread)
        CtrlThread->send_packet_thread->SendPacket(
            tmpbuf,
            sizeof(struct ether_addr) * 2 + ipv4_len + udp_len + buflen
        );

    else
        g_logSystem.AppendText("CtrlThread->m_sendPacketThread=NULL");

    packet_sent = true;
    memcpy(
        last_sent_packet,
        tmpbuf,
        last_sent_packet_len =
            sizeof(struct ether_addr) * 2 + ipv4_len + udp_len + buflen
    );
    return true;
}

#define DESKEY reinterpret_cast<const unsigned char *>("|sS1&@8q")
#define INIT_XORKEY { 'A', 'c', '3', '#', '1', '!', 'Q', 'd' }

bool CDirectTransfer::DescryptForSAM(unsigned char *buf, unsigned buflen)
{
    unsigned char tmpibuf[8] = {}, tmpobuf[8] = {};
    unsigned char xorkey[8] = INIT_XORKEY;

    if (buflen & 0x7)
        return false;

    deskey(DESKEY, DE1);

    for (unsigned i = 0; i < buflen >> 3; i++) {
        for (unsigned j = 0; j < 8; j++)
            tmpibuf[j] = buf[j];

        des(tmpibuf, tmpobuf);

        for (unsigned j = 0; j < 8; j++)
            tmpobuf[j] ^= xorkey[j];

        for (unsigned j = 0; j < 8; j++)
            *buf++ = tmpobuf[j];

        for (unsigned j = 0; j < 8; j++)
            xorkey[j] = tmpibuf[j];
    }

    return true;
}

bool CDirectTransfer::EncryptForSAM(unsigned char *buf, unsigned buflen)
{
    unsigned char tmpibuf[8] = {}, tmpobuf[8] = {};
    unsigned char xorkey[8] = INIT_XORKEY;

    if (buflen & 0x7)
        return false;

    deskey(DESKEY, EN0);

    for (unsigned i = 0; i < buflen >> 3; i++) {
        for (unsigned j = 0; j < 8; j++)
            tmpibuf[j] = buf[j];

        for (unsigned j = 0; j < 8; j++)
            tmpibuf[j] ^= xorkey[j];

        des(tmpibuf, tmpobuf);

        for (unsigned j = 0; j < 8; j++)
            *buf++ = tmpobuf[j];

        for (unsigned j = 0; j < 8; j++)
            xorkey[j] = tmpobuf[j];
    }

    return true;
}
