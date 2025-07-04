#include "all.h"
#include "md5checksum.h"
#include "util.h"
#include "timeutil.h"
#include "dirtransutil.h"

void CreateDirPktHead(
    struct mtagFinalDirPacket &final_packet_head,
    struct tagDirPacketHead &packet_head,
    [[maybe_unused]] struct tagSenderBind &sender_bind,
    char *buf,
    unsigned buflen,
    char *keybuf,
    char *ivbuf
)
{
    char *checksum_buf = nullptr;
    char md5_checksum[16] = {};
    char *md5_checksum_ascii = nullptr;
#define COPY_FIELD(name) final_packet_head.name = packet_head.name
    COPY_FIELD(version);
    COPY_FIELD(response_code);
    COPY_FIELD(id);
    COPY_FIELD(packet_len);
    memcpy(
        final_packet_head.md5sum,
        packet_head.md5sum,
        sizeof(packet_head.md5sum)
    );
    COPY_FIELD(session_id);
    COPY_FIELD(timestamp);
    final_packet_head.field_24 = packet_head.field_28;
    COPY_FIELD(slicetype);
    COPY_FIELD(data_len);
#undef COPY_FIELD
    checksum_buf = new char[ntohs(packet_head.packet_len) + 16];
    *reinterpret_cast<struct mtagFinalDirPacket *>(checksum_buf) =
        final_packet_head;
    memset(
        reinterpret_cast<struct mtagFinalDirPacket *>(checksum_buf)->md5sum,
        0,
        sizeof(md5_checksum)
    );
    // we must hard-code keybuf and ivbuf's size
    memcpy(checksum_buf + sizeof(struct mtagFinalDirPacket), keybuf, 8);
    memcpy(checksum_buf + sizeof(struct mtagFinalDirPacket) + 8, ivbuf, 8);
    md5_checksum_ascii =
        CMD5Checksum::GetMD5(
            checksum_buf,
            sizeof(struct mtagFinalDirPacket) + sizeof(md5_checksum)
        );
    MD5StrtoUChar(md5_checksum_ascii, md5_checksum);
    memcpy(
        reinterpret_cast<struct mtagFinalDirPacket *>(checksum_buf)->md5sum,
        md5_checksum,
        sizeof(md5_checksum)
    );
    delete[] md5_checksum_ascii;
    delete[] checksum_buf;
}

void CreateSessionIfNecessary(
    struct tagRecvBind &gsn_pkg,
    in_addr_t srcaddr,
    unsigned session_id,
    struct tagRecvSessionBind &recv_session
)
{
    for (struct tagRecvSessionBind &session : gsn_pkg.recv_session_bounds) {
        if (session.srcaddr != srcaddr || session.session_id != session_id)
            continue;

        recv_session = session;
        return;
    }

    gsn_pkg.recv_session_bounds.emplace_back(
        session_id,
        srcaddr,
        0,
        0,
        0,
        gsn_pkg.on_receive_packet_post_mtype,
        nullptr,
        0,
        0,
        GetTickCount(),
        false
    );
    recv_session = gsn_pkg.recv_session_bounds.back();
}

void InitSmpInitPacket(struct tagSmpInitPacket &packet)
{
    packet.smp_current_time.clear();
    packet.basic_config.login_url.clear();
    packet.basic_config.disable_arpbam.clear();
    packet.basic_config.disable_dhcpbam.clear();
    packet.basic_config.hi_detect_interval = 0;
    packet.basic_config.hello_interval = 0;
    packet.basic_config.hostinfo_report_interval = 0;
    packet.basic_config.timeout = 3;
    packet.basic_config.retry_times = 3;
    packet.arp.enabled = 0;
    packet.arp.gateway_ip.clear();
    packet.arp.gateway_ip.clear();
    packet.illegal_network_detect.enabled = 0;
    packet.illegal_network_detect.syslog_ip.clear();
    packet.illegal_network_detect.syslog_port = 0;
    packet.illegal_network_detect.detect_interval = 0;
    packet.illegal_network_detect.is_block = 0;
    packet.illegal_network_detect.block_tip.clear();
    packet.hi_xml.clear();
    packet.security_domain_xml.clear();
}

void CopyDirTranPara(
    struct tagDirTranPara *dst,
    const struct tagDirTranPara *src
)
{
    memcpy(dst, src, sizeof(struct tagDirTranPara));
    memset(dst->data, 0, sizeof(dst->data));

    if (dst->mtu > MAX_MTU)
        dst->mtu = MAX_MTU;

    memcpy(dst->data, src->data, dst->mtu);
}
