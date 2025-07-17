#include "all.h"
#include "global.h"
#include "rgprivateproc.h"
#include "encodeutil.h"
#include "sysutil.h"
#include "vz_apiapp.h"
#include "netutil.h"
#include "eapolutil.h"

struct eapolpkg *ChangeToUChar(
    const struct EAPOLFrame *eapol_frame,
    unsigned *length
)
{
    CRGPrivateProc priproc;
    unsigned priproc_len = 0;
    unsigned len = 0;
    struct eapolpkg *ret =
            reinterpret_cast<struct eapolpkg *>(new char[1400]);

    if (CtrlThread->IsRuijieNas())
        *reinterpret_cast<struct ether_addr *>(ret->etherheader.ether_dhost) =
            eapol_frame->dstaddr;

    else {
        ret->etherheader.ether_dhost[0] = 0x01;
        ret->etherheader.ether_dhost[1] = 0x80;
        ret->etherheader.ether_dhost[2] = 0xC2;
        ret->etherheader.ether_dhost[3] = 0x00;
        ret->etherheader.ether_dhost[4] = 0x00;
        ret->etherheader.ether_dhost[5] = 0x03;
    }

    *reinterpret_cast<struct ether_addr *>(ret->etherheader.ether_shost) =
        eapol_frame->srcaddr;
    ret->etherheader.ether_type = htons(ETH_P_PAE);
    ret->ieee8021x_version = 1;
    ret->ieee8021x_packet_type = eapol_frame->ieee8021x_packet_type;
    ret->ieee8021x_packet_length = htons(eapol_frame->ieee8021x_packet_length);

    if (eapol_frame->ieee8021x_packet_length) {
        if (eapol_frame->ieee8021x_packet_length < 4) {
            delete[] reinterpret_cast<char *>(ret);
            return nullptr;
        }

        ret->eap_packet.code = eapol_frame->eap_code;
        ret->eap_packet.id = eapol_frame->eap_id;
        ret->eap_packet.length = htons(eapol_frame->eap_length);
        ret->eap_packet.type = eapol_frame->eap_type;
        len = offsetof(struct eapolpkg, eap_packet.type);

        if (eapol_frame->eap_type != EAP_TYPE_MD5) {
            if (eapol_frame->ieee8021x_packet_length > 5) {
                memcpy(
                    ret->eap_packet.data.identity.message,
                    eapol_frame->eap_type_notif_data,
                    eapol_frame->ieee8021x_packet_length - 5
                );
                len += eapol_frame->ieee8021x_packet_length - 5;
            }

        } else {
            if (eapol_frame->ieee8021x_packet_length <= 5) {
                delete[] reinterpret_cast<char *>(ret);
                return nullptr;
            }

            ret->eap_packet.data.md5.value_size = eapol_frame->eap_type_md5_length;
            memcpy(
                ret->eap_packet.data.md5.value_name,
                eapol_frame->eap_type_md5_data,
                eapol_frame->ieee8021x_packet_length - 6
            );

            if (!CtrlThread->IsRuijieNas()) {
                len += 16;
                priproc.EncapRGVerdorSeg(
                    ret->eap_packet.data.md5.value_name + 16,
                    priproc_len
                );

                if (priproc_len) {
                    ret->ieee8021x_packet_length =
                        ret->eap_packet.length = htons(priproc_len + 22);
                    len += priproc_len;
                }

            } else
                len += eapol_frame->ieee8021x_packet_length - 6;
        }
    }

    if (
        (
            eapol_frame->ieee8021x_packet_type != IEEE8021X_EAPOL_START ||
            (
                eapol_frame->dstaddr.ether_addr_octet[0] != 0x01 &&
                eapol_frame->dstaddr.ether_addr_octet[1] != 0x80 &&
                eapol_frame->dstaddr.ether_addr_octet[2] != 0xC2
            )
        ) &&
        CtrlThread->IsRuijieNas()
    )
        AppendPrivateProperty(reinterpret_cast<char *>(ret), len, eapol_frame);

    *length = len;
    return ret;
}

struct EAPOLFrame *ChangeToEAPOLFrame(
    const struct eapolpkg *eapol_pkg,
    unsigned length
)
{
    struct EAPOLFrame *ret = new struct EAPOLFrame;
    CRGPrivateProc priproc;
    const char *private_buf = nullptr;
    unsigned service_strlen = 0;
    std::string service_str;

    if (length < 18 || length > 1514)
        return nullptr;

#define EXIT_IF_UNMATCH(cond) do if (!(cond)) { delete ret; return nullptr; } while (0)
    InitEAPOLFrame(ret);
    ret->dstaddr =
        *reinterpret_cast<const struct ether_addr *>
        (eapol_pkg->etherheader.ether_dhost);
    ret->srcaddr =
        *reinterpret_cast<const struct ether_addr *>
        (eapol_pkg->etherheader.ether_shost);
    EXIT_IF_UNMATCH(!IsLoopBack(&ret->srcaddr));
    EXIT_IF_UNMATCH(
        IsHostDstMac(&ret->dstaddr) ||
        Is8021xGroupAddr(&ret->dstaddr) ||
        IsMulDstMac(&ret->dstaddr) ||
        IsStarGroupDstMac(&ret->dstaddr)
    );
    ret->ether_type = ntohs(eapol_pkg->etherheader.ether_type);
    EXIT_IF_UNMATCH(ntohs(eapol_pkg->etherheader.ether_type) == ETH_P_PAE);
    ret->ieee8021x_version = eapol_pkg->ieee8021x_version;
    ret->ieee8021x_packet_type = eapol_pkg->ieee8021x_packet_type;

    if (
        ret->ieee8021x_version == 0x01 &&
        (
            ret->ieee8021x_packet_type == static_cast<enum IEEE8021X_PACKET_TYPE>(0xC0) ||
            ret->ieee8021x_packet_type == static_cast<enum IEEE8021X_PACKET_TYPE>(0xBF)
        )
    )
        return ret;

    EXIT_IF_UNMATCH(ret->ieee8021x_packet_type == IEEE8021X_EAP_PACKET);
    ret->ieee8021x_packet_length = ntohs(eapol_pkg->ieee8021x_packet_length);
    EXIT_IF_UNMATCH(ret->ieee8021x_packet_length > 3);
    ret->eap_code = eapol_pkg->eap_packet.code;
    ret->eap_id = eapol_pkg->eap_packet.id;
    ret->eap_length = ntohs(eapol_pkg->eap_packet.length);

    if (ret->eap_code == EAP_SUCCESS || ret->eap_code == EAP_FAILURE) {
        ParsePrivateProperty(
            reinterpret_cast<const char *>(&eapol_pkg->eap_packet.type),
            ret->ieee8021x_packet_length - 4,
            ret
        );
        return ret;
    }

    if (ret->ieee8021x_packet_length <= 4)
        return ret;

    ret->eap_type = eapol_pkg->eap_packet.type;

    if (ret->eap_type == EAP_TYPE_MD5) {
        EXIT_IF_UNMATCH(ret->ieee8021x_packet_length > 5);
        ret->eap_type_md5_length = eapol_pkg->eap_packet.data.md5.value_size;
        ret->eap_type_md5_data = new char[ret->eap_type_md5_length + 1];
        memcpy(
            ret->eap_type_md5_data,
            eapol_pkg->eap_packet.data.md5.value_name,
            ret->eap_type_md5_length
        );
        memcpy(e_pMd5Chanllenge, eapol_pkg->eap_packet.data.md5.value_name, 16);
        ret->eap_type_md5_data[ret->eap_type_md5_length] = 0;

        if (CtrlThread->IsRuijieNas()) {
            if (length >= 24 + ret->eap_type_md5_length + 6 + 4 + 1 + 1 + 4 + 1) {
                private_buf =
                    reinterpret_cast<const char *>
                    (&eapol_pkg->eap_packet.data.md5.value_name[ret->eap_type_md5_length]);
#define VALUE(off_pos, type) (*reinterpret_cast<const type *>(&private_buf[off_pos]))
                service_strlen = VALUE(4 + 1 + 1 + 4 + 1, unsigned char) - 2;

                if (
                    service_strlen &&
                    length >=
                    24 + ret->eap_type_md5_length + 6 + 4 + 1 + 1 + 4 + 1 + service_strlen
                ) {
                    if (
                        ntohl(VALUE(0, unsigned)) == FIELD_MAGIC &&
                        VALUE(4, unsigned char) == 0x2E &&
                        VALUE(4 + 1, unsigned char) == 0x03
                    )
                        CtrlThread->field_53B = VALUE(4 + 1 + 1, unsigned char);

                    if (
                        ntohl(VALUE(4 + 1 + 1, unsigned)) == FIELD_MAGIC &&
                        VALUE(4 + 1 + 1 + 4, unsigned char) == 0x66
                    ) {
                        logFile.AppendText("ChangeToEAPOLFrame---------更新服务列表");
                        ConvertGBKToUtf8(
                            service_str,
                            &private_buf[
                                ret->eap_type_md5_length +
                                6 + 4 + 1 + 1 + 4 + 1 + service_strlen
                            ],
                            service_strlen
                        );
                        CtrlThread->private_properties.services.clear();
                        ParseString(
                            service_str,
                            '@',
                            CtrlThread->private_properties.services
                        );
                        RcvSvrList(CtrlThread->private_properties.services);
                    }
                }

#undef VALUE
            }

        } else if (ret->eap_length > 22)
            priproc.ReadRGVendorSeg(
                eapol_pkg->eap_packet.data.md5.value_name,
                ret->eap_type_md5_length
            );

    } else if (ret->ieee8021x_packet_length > 5) {
        memcpy(
            ret->eap_type_notif_data = new char[ret->ieee8021x_packet_length - 5 + 1],
            eapol_pkg->eap_packet.data.notification.message,
            ret->ieee8021x_packet_length - 5
        );
        ret->eap_type_notif_data[ret->ieee8021x_packet_length - 5] = 0;
    }

#undef EXIT_IF_UNMATCH
    return ret;
}

struct eapolpkg *CreateEapolPacket(
    const struct EAPOLFrame *eapol_frame,
    unsigned *length
)
{
    struct eapolpkg *ret = ChangeToUChar(eapol_frame, length);
    return ret && *length < 4049 ? ret : nullptr;
}

void DeleteFrameMemory(struct EAPOLFrame *eapol_frame)
{
    if (
        eapol_frame->eap_code != EAP_REQUEST &&
        eapol_frame->eap_code != EAP_RESPONSE
    ) {
        delete eapol_frame;
        return;
    }

    if (
        (
            eapol_frame->eap_type == EAP_TYPE_IDENTITY ||
            eapol_frame->eap_type == EAP_TYPE_NOTIFICATION ||
            eapol_frame->eap_type == EAP_TYPE_NAK ||
            eapol_frame->eap_type == EAP_TYPE_OTP ||
            eapol_frame->eap_type == EAP_TYPE_GTC
        ) &&
        eapol_frame->ieee8021x_packet_length > 5
    )
        delete[] eapol_frame->eap_type_notif_data;

    else if (
        eapol_frame->eap_type == EAP_TYPE_MD5 &&
        eapol_frame->ieee8021x_packet_length > 6
    )
        delete[] eapol_frame->eap_type_md5_data;

    delete eapol_frame;
}

void DhcpIpInfoToUChar(char *buf, const struct EAPOLFrame *eapol_frame)
{
    unsigned short crcsum = 0;
    *buf++ = 0x00;
    *buf++ = 0x00;
    *buf++ = 0x13;
    *buf++ = 0x11;
    *buf++ = eapol_frame->dhcp_ipinfo.dhcp_enabled;
    *buf++ = eapol_frame->dhcp_ipinfo.ip4_ipaddr >> 24;
    *buf++ = eapol_frame->dhcp_ipinfo.ip4_ipaddr >> 16 & 0xff;
    *buf++ = eapol_frame->dhcp_ipinfo.ip4_ipaddr >> 8 & 0xff;
    *buf++ = eapol_frame->dhcp_ipinfo.ip4_ipaddr & 0xff;
    *buf++ = eapol_frame->dhcp_ipinfo.ip4_netmask >> 24;
    *buf++ = eapol_frame->dhcp_ipinfo.ip4_netmask >> 16 & 0xff;
    *buf++ = eapol_frame->dhcp_ipinfo.ip4_netmask >> 8 & 0xff;
    *buf++ = eapol_frame->dhcp_ipinfo.ip4_netmask & 0xff;
    *buf++ = eapol_frame->dhcp_ipinfo.gateway >> 24;
    *buf++ = eapol_frame->dhcp_ipinfo.gateway >> 16 & 0xff;
    *buf++ = eapol_frame->dhcp_ipinfo.gateway >> 8 & 0xff;
    *buf++ = eapol_frame->dhcp_ipinfo.gateway & 0xff;
    *buf++ = eapol_frame->dhcp_ipinfo.dns >> 24;
    *buf++ = eapol_frame->dhcp_ipinfo.dns >> 16 & 0xff;
    *buf++ = eapol_frame->dhcp_ipinfo.dns >> 8 & 0xff;
    *buf++ = eapol_frame->dhcp_ipinfo.dns & 0xff;
    crcsum = crc16(reinterpret_cast<unsigned char *>(buf), 21);
    *buf++ = crcsum >> 8;
    *buf++ = crcsum & 0xff;
}

void EncapUCharDhcpIpInfo(char *buf, struct EAPOLFrame *eapol_frame)
{
    char tmpbuf[23] = {};
    DhcpIpInfoToUChar(buf, eapol_frame);
    encode(tmpbuf, sizeof(tmpbuf));
    memcpy(buf, tmpbuf, sizeof(tmpbuf));
}

void InitEAPOLFrame(struct EAPOLFrame *eapol_frame)
{
    eapol_frame->dstaddr = {};
    eapol_frame->srcaddr = {};
    eapol_frame->ether_type = 0;
    eapol_frame->ieee8021x_version = 0;
    eapol_frame->ieee8021x_packet_type = IEEE8021X_EAP_PACKET;
    eapol_frame->ieee8021x_packet_length = 0;
    eapol_frame->eap_code = EAP_CODE_INVALID;
    eapol_frame->eap_id = 0;
    eapol_frame->eap_length = 0;
    eapol_frame->eap_type = EAP_TYPE_INVALID;
    eapol_frame->eap_type_md5_data = nullptr;
    eapol_frame->eap_type_md5_length = 0;
    eapol_frame->fail_reason_magic = 0;
    eapol_frame->fail_reason_length = 0;
    memset(eapol_frame->fail_reason, 0, sizeof(eapol_frame->fail_reason));
    eapol_frame->dhcp_ipinfo.dhcp_enabled = 0;
    eapol_frame->dhcp_ipinfo.ip4_ipaddr = 0;
    eapol_frame->dhcp_ipinfo.ip4_netmask = 0;
    eapol_frame->dhcp_ipinfo.gateway = 0;
    eapol_frame->dhcp_ipinfo.dns = 0;
}

void AppendPrivateProperty(
    char *buf,
    unsigned &len,
    const struct EAPOLFrame *eapol_frame
)
{
    char password_buf[512] = {};
    char dns_buf[253] = {};
    char service_buf[128] = {};
    char diskid_buf[128] = {};
    char dontknow_buf[128] = {};
    char version_buf[128] = {};
    char empty_md5_challenge[16] = {};
    unsigned password_buflen = 0;
    unsigned dns_buflen = 0;
    unsigned service_buflen = 0;
    unsigned diskid_buflen = 0;
    unsigned dontknow_buflen = 0;
    unsigned version_buflen = 0;
    CVz_APIApp vz_apiapp;
    EncapUCharDhcpIpInfo(&buf[len], eapol_frame);
    len += 23;
    buf[len++] = 0x00;
    buf[len++] = 0x00;
    buf[len++] = 0x13;
    buf[len++] = 0x11;
    EncapProgrammName("8021x.exe", buf);
    len += 32;
    EncapUCharVersionNumber(&buf[len]);
    len += 4;
    buf[len++] = eapol_frame->field_6C0;
    buf[len++] = 0x00;
    buf[len++] = 0x00;
    buf[len++] = 0x13;
    buf[len++] = 0x11;
    buf[len++] = 0x00;
    buf[len++] = 0x00;
    buf[len++] = 0x1A;
    buf[len++] = 0x0C;
    buf[len++] = 0x00;
    buf[len++] = 0x00;
    buf[len++] = 0x13;
    buf[len++] = 0x11;
    buf[len++] = 0x18;
    buf[len++] = 0x06;
    *reinterpret_cast<uint32_t *>(&buf[len]) = CtrlThread->IsDhcpAuth();
    len += sizeof(uint32_t);
    buf[len++] = 0x1A;
    buf[len++] = 0x0E;
    buf[len++] = 0x00;
    buf[len++] = 0x00;
    buf[len++] = 0x13;
    buf[len++] = 0x11;
    buf[len++] = 0x2D;
    buf[len++] = 0x08;
    *reinterpret_cast<struct ether_addr *>(&buf[len]) = eapol_frame->field_C;
    len += sizeof(struct ether_addr);
    buf[len++] = 0x1A;
    buf[len++] = 0x08;
    buf[len++] = 0x00;
    buf[len++] = 0x00;
    buf[len++] = 0x13;
    buf[len++] = 0x11;
    buf[len++] = 0x2F;
    buf[len++] = 0x02;

    if (CtrlThread->field_53B) {
        ConvertUtf8ToGBK(
            password_buf,
            sizeof(password_buf),
            CtrlThread->configure_info.last_auth_password.c_str(),
            CtrlThread->configure_info.last_auth_password.length()
        );
        password_buflen = strlen(password_buf);
        password_buflen =
            password_buflen ?
            ((password_buflen << 4) + !!(password_buflen & 15)) >> 4 :
            16;
        RadiusEncrpytPwd(
            e_pMd5Chanllenge,
            sizeof(e_pMd5Chanllenge),
            password_buf,
            password_buflen,
            &buf[len]
        );
        buf[len - 1] += password_buflen;
        buf[len - 7] += password_buflen;
        len += password_buflen;
        CtrlThread->field_53B = false;
    }

    get_alternate_dns(dns_buf, dns_buflen = sizeof(dns_buflen));
    buf[len++] = 0x1A;
    buf[len++] = dns_buflen + 8;
    buf[len++] = 0x00;
    buf[len++] = 0x00;
    buf[len++] = 0x13;
    buf[len++] = 0x11;
    buf[len++] = 0x76;
    buf[len++] = dns_buflen + 2;
    memcpy(&buf[len], dns_buf, dns_buflen);
    len += dns_buflen;
    buf[len++] = 0x1A;
    buf[len++] = 0x09;
    buf[len++] = 0x00;
    buf[len++] = 0x00;
    buf[len++] = 0x13;
    buf[len++] = 0x11;
    buf[len++] = 0x35;
    buf[len++] = 0x03;
    buf[len++] = eapol_frame->dhcp_ipinfo.ipaddr6_count;
    buf[len++] = 0x1A;
    buf[len++] = 0x18;
    buf[len++] = 0x00;
    buf[len++] = 0x00;
    buf[len++] = 0x13;
    buf[len++] = 0x11;
    buf[len++] = 0x36;
    buf[len++] = 0x12;
    *reinterpret_cast<struct in6_addr *>(&buf[len]) =
        eapol_frame->dhcp_ipinfo.ip6_my_ipaddr;
    len += sizeof(struct in6_addr);
    buf[len++] = 0x1A;
    buf[len++] = 0x18;
    buf[len++] = 0x00;
    buf[len++] = 0x00;
    buf[len++] = 0x13;
    buf[len++] = 0x11;
    buf[len++] = 0x38;
    buf[len++] = 0x12;
    *reinterpret_cast<struct in6_addr *>(&buf[len]) =
        eapol_frame->dhcp_ipinfo.ip6_link_local_ipaddr;
    len += sizeof(struct in6_addr);
    buf[len++] = 0x1A;
    buf[len++] = 0x18;
    buf[len++] = 0x00;
    buf[len++] = 0x00;
    buf[len++] = 0x13;
    buf[len++] = 0x11;
    g_dhcpDug.AppendText("ipv6 可集聚全球单播地址:");
    g_dhcpDug.HexPrinter(
        reinterpret_cast<const char *>(eapol_frame->dhcp_ipinfo.ip6_ipaddr.s6_addr),
        sizeof(struct in6_addr)
    );
    buf[len++] = 0x4E;
    buf[len++] = 0x12;
    *reinterpret_cast<struct in6_addr *>(&buf[len]) =
        eapol_frame->dhcp_ipinfo.ip6_ipaddr;
    len += sizeof(struct in6_addr);
    buf[len++] = 0x1A;
    buf[len++] = 0x88;
    buf[len++] = 0x00;
    buf[len++] = 0x00;
    buf[len++] = 0x13;
    buf[len++] = 0x11;
    buf[len++] = 0x4D;
    buf[len++] = 0x82;

    if (eapol_frame->eap_type == EAP_TYPE_MD5)
        vz_apiapp.Vz_API(
            &buf[len],
            e_pMd5Chanllenge,
            CtrlThread->configure_info.public_title.c_str()
        );

    else
        vz_apiapp.Vz_API(&buf[len], empty_md5_challenge, "");

    len += 128;
    buf[len++] = 0x1A;
    buf[len++] = 0x28;
    buf[len++] = 0x00;
    buf[len++] = 0x00;
    buf[len++] = 0x13;
    buf[len++] = 0x11;
    buf[len++] = 0x39;
    buf[len++] = 0x22;
    ConvertUtf8ToGBK(
        service_buf,
        sizeof(service_buf),
        CtrlThread->field_1139 ?
        CtrlThread->service_name.c_str() :
        CtrlThread->configure_info.public_service.c_str(),
        CtrlThread->field_1139 ?
        CtrlThread->service_name.length() :
        CtrlThread->configure_info.public_service.length()
    );
    service_buflen = strlen(service_buf);
    memcpy(&buf[len], service_buf, service_buflen);
    len += 32;
    buf[len++] = 0x1A;
    buf[len++] = 0x48;
    buf[len++] = 0x00;
    buf[len++] = 0x00;
    buf[len++] = 0x13;
    buf[len++] = 0x11;
    buf[len++] = 0x54;
    buf[len++] = 0x42;
    ConvertUtf8ToGBK(
        diskid_buf,
        sizeof(diskid_buf),
        CtrlThread->diskid.c_str(),
        CtrlThread->diskid.length()
    );
    diskid_buflen = strlen(diskid_buf);
    memcpy(&buf[len], diskid_buf, diskid_buflen);
    len += 64;
    ConvertUtf8ToGBK(
        dontknow_buf,
        sizeof(dontknow_buf),
        CtrlThread->sec_domain_name.c_str(),
        CtrlThread->sec_domain_name.length()
    );
    dontknow_buflen = strlen(dontknow_buf);
    buf[len++] = dontknow_buflen + 8;
    buf[len++] = 0x00;
    buf[len++] = 0x00;
    buf[len++] = 0x13;
    buf[len++] = 0x11;
    buf[len++] = 0x55;
    memcpy(&buf[len], CtrlThread->sec_domain_name.c_str(), dontknow_buflen);
    len += dontknow_buflen;
    buf[len++] = 0x1A;
    buf[len++] = 0x09;
    buf[len++] = 0x00;
    buf[len++] = 0x00;
    buf[len++] = 0x13;
    buf[len++] = 0x11;
    buf[len++] = 0x62;
    buf[len++] = 0x03;
    buf[len++] = eapol_frame->eap_type == EAP_TYPE_MD5 ? GetHIRusultByLocal() : 0;

    if (CtrlThread->field_1139) {
        buf[len++] = 0x1A;
        buf[len++] = 0x0A;
        buf[len++] = 0x00;
        buf[len++] = 0x00;
        buf[len++] = 0x13;
        buf[len++] = 0x11;
        buf[len++] = 0x64;
        buf[len++] = 0x04;
        buf[len++] = 0x00;
        buf[len++] = 0x01;
    }

    buf[len++] = 0x1A;
    buf[len++] = 0x09;
    buf[len++] = 0x00;
    buf[len++] = 0x00;
    buf[len++] = 0x13;
    buf[len++] = 0x11;
    buf[len++] = 0x70;
    buf[len++] = 0x03;
    buf[len++] = Is64BIT() ? 64 : 32; // I intentionally don't use hexadecimal
    ConvertUtf8ToGBK(
        version_buf,
        sizeof(version_buf),
        theApp.version.c_str(),
        theApp.version.length()
    );
    version_buflen = strlen(version_buf);
    buf[len++] = 0x1A;
    buf[len++] = version_buflen + 9;
    buf[len++] = 0x00;
    buf[len++] = 0x00;
    buf[len++] = 0x13;
    buf[len++] = 0x11;
    buf[len++] = 0x6F;
    buf[len++] = version_buflen + 3;
    memcpy(&buf[len], version_buf, version_buflen);
    len += version_buflen;
    buf[len++] = 0x00;
    buf[len++] = 0x1A;
    buf[len++] = 0x09;
    buf[len++] = 0x00;
    buf[len++] = 0x00;
    buf[len++] = 0x13;
    buf[len++] = 0x11;
    buf[len++] = 0x79;
    buf[len++] = 0x03;
    buf[len++] = 0x02;
}

void ParsePrivateProperty(
    const char *buf,
    unsigned len,
    struct EAPOLFrame *eapol_frame
)
{
    unsigned fail_reason_len = 0;
    unsigned cur_pos = 0;
    char hello_buf[15] = {};
    std::string fail_reason;
    std::string service_list_str;

    if (len < 5)
        return;

    if (ntohl(*reinterpret_cast<const uint32_t *>(buf)) != FIELD_MAGIC)
        return;

    cur_pos += sizeof(uint32_t);
    eapol_frame->fail_reason_magic = FIELD_MAGIC;
    fail_reason_len = ntohs(*reinterpret_cast<const uint16_t *>(&buf[cur_pos]));
    cur_pos += sizeof(uint16_t);

    if (len < fail_reason_len + 6)
        return;

    memset(eapol_frame->fail_reason, 0, sizeof(eapol_frame->fail_reason));
    ConvertGBKToUtf8(fail_reason, &buf[cur_pos], fail_reason_len);
    strcpy(eapol_frame->fail_reason, fail_reason.c_str());
    g_uilog.AppendText(
        "ParsePrivateProperty pFrame->EAPinfo.notification=%s",
        fail_reason.c_str()
    );
    cur_pos += fail_reason_len;

    if (len < fail_reason_len + 6 + 6 + 105)
        return;

    if (ntohl(*reinterpret_cast<const uint32_t *>(&buf[cur_pos])) != FIELD_MAGIC)
        return;

    cur_pos += sizeof(uint32_t);
    eapol_frame->upgrade_info.magic = FIELD_MAGIC;
    cur_pos += sizeof(uint16_t);
    eapol_frame->upgrade_info.su_newest_ver =
        ntohl(*reinterpret_cast<const uint32_t *>(&buf[cur_pos]));
    cur_pos += 105 + 32 * buf[cur_pos + 104];

    if (len < cur_pos + 6)
        return;

    if (ntohl(*reinterpret_cast<const uint32_t *>(&buf[cur_pos])) != FIELD_MAGIC)
        return;

    eapol_frame->proxy_avoid_magic = FIELD_MAGIC;
    cur_pos += sizeof(uint32_t);
    eapol_frame->proxy_avoid_val = buf[cur_pos++] == 1;
    eapol_frame->proxy_avoid_val2 = buf[cur_pos++];
    cur_pos += sizeof(uint16_t);

    if (
        !eapol_frame->proxy_avoid_val ||
        eapol_frame->proxy_avoid_val2 > 1 ||
        len < cur_pos + 6
    )
        return;

    if (ntohl(*reinterpret_cast<const uint32_t *>(&buf[cur_pos])) != FIELD_MAGIC)
        return;

    eapol_frame->dont_know_field_magic = FIELD_MAGIC;
    cur_pos += sizeof(uint32_t);
    eapol_frame->dont_know_field_val = buf[cur_pos++] == 1;
    eapol_frame->dont_know_field_val2 = buf[cur_pos++];
    cur_pos += sizeof(uint16_t);

    if (
        !eapol_frame->dont_know_field_val ||
        eapol_frame->dont_know_field_val2 > 1 ||
        len < cur_pos + 15
    )
        return;

    memcpy(hello_buf, &buf[cur_pos], 15);
    decode(hello_buf, 15);
    cur_pos += 15;

    if (ntohl(*reinterpret_cast<uint32_t *>(&hello_buf[0])) != FIELD_MAGIC)
        return;

    eapol_frame->parse_hello_magic = FIELD_MAGIC;
    eapol_frame->parse_hello_val = hello_buf[1];
    eapol_frame->parse_hello_val2 = hello_buf[2];
    eapol_frame->parse_hello_id =
        ntohl(*reinterpret_cast<uint32_t *>(&hello_buf[3]));
    eapol_frame->parse_hello_inv =
        1000 * ntohl(*reinterpret_cast<uint32_t *>(&hello_buf[7]));
    eapol_frame->parse_hello_val3 = hello_buf[11];

    if (eapol_frame->parse_hello_val != 10)
        return;

    CtrlThread->private_properties.radius_type = 5;
    CtrlThread->private_properties.su_newest_ver = 0;
    CtrlThread->private_properties.account_info.clear();
    CtrlThread->private_properties.persional_info.clear();
    CtrlThread->private_properties.broadcast_str.clear();
    CtrlThread->private_properties.fail_reason.clear();
    CtrlThread->private_properties.su_upgrade_url.clear();
    CtrlThread->private_properties.su_reauth_interv = 0;
    CtrlThread->private_properties.radius_type = 0;
    CtrlThread->private_properties.proxy_avoid = 0;
    CtrlThread->private_properties.dialup_avoid = 0;
    CtrlThread->private_properties.indicate_serv_ip = 0;
    CtrlThread->private_properties.indicate_port = 0;
    CtrlThread->private_properties.field_A4 = 0;
    CtrlThread->private_properties.proxy_dectect_kinds = 0;
    CtrlThread->private_properties.hello_interv = 0;
    memset(
        CtrlThread->private_properties.encrypt_key,
        0,
        sizeof(CtrlThread->private_properties.encrypt_key)
    );
    memset(
        CtrlThread->private_properties.encrypt_iv,
        0,
        sizeof(CtrlThread->private_properties.encrypt_iv)
    );
    CtrlThread->private_properties.server_utc_time = 0;
    CtrlThread->private_properties.svr_switch_result.clear();
    CtrlThread->private_properties.user_login_url.clear();
    CtrlThread->private_properties.msg_client_port = 80;
    CtrlThread->private_properties.utrust_url.clear();
    CtrlThread->private_properties.is_show_utrust_url = 1;
    CtrlThread->private_properties.delay_second_show_utrust_url = 0;
    CtrlThread->private_properties.parse_hello = 0;
    CtrlThread->private_properties.direct_communication_highest_version_supported =
        true;
    CtrlThread->private_properties.direct_comm_heartbeat_flags = 0;
    g_rjPrivateParselog.AppendText("BEGIN PARSE PRIVATE SEG");
    g_rjPrivateParselog.HexPrinter(&buf[cur_pos], len - cur_pos);

    if (len == cur_pos || len < cur_pos + 7)
        return;

    unsigned cur_magic = ntohl(*reinterpret_cast<const uint32_t *>(&buf[cur_pos]));
    char cur_type = buf[cur_pos + 4];
    char cur_length = buf[cur_pos + 5];
    cur_pos += 6;

    for (
        ;
        cur_magic == FIELD_MAGIC &&
        cur_length > 1 &&
        len != cur_pos &&
        len >= cur_pos + 6;
        cur_pos += cur_length - 2,
        cur_magic = ntohl(*reinterpret_cast<const uint32_t *>(&buf[cur_pos])),
        cur_type = buf[cur_pos + 4],
        cur_length = buf[cur_pos + 5],
        cur_pos += 6
    )
        switch (cur_type) {
            case 0x37:
                if (len < cur_length - 2 + cur_pos)
                    return;

                eapol_frame->upgrade_info.type = cur_type;
                eapol_frame->upgrade_info.length = cur_length - 2;
                ConvertGBKToUtf8(
                    eapol_frame->upgrade_info.su_upgrade_url,
                    &buf[cur_pos],
                    cur_length - 2
                );
                break;

            case 0x3C:
            case 0x3D:
                eapol_frame->fail_reason[fail_reason_len] = '\r';
                eapol_frame->fail_reason[fail_reason_len + 1] = '\n';
                ConvertGBKToUtf8(
                    &eapol_frame->fail_reason[fail_reason_len + 2],
                    sizeof(eapol_frame->fail_reason) - fail_reason_len - 2,
                    &buf[cur_pos],
                    cur_length - 2
                );
                break;

            case 0x56:
                if (cur_length != 6 || len < cur_pos + 4)
                    return;

                CtrlThread->configure_info.auto_reconnect =
                    ntohl(*reinterpret_cast<const uint32_t *>(&buf[cur_pos]));
                CtrlThread->configure_info.is_autoreconnect =
                    CtrlThread->configure_info.auto_reconnect;
                logFile.AppendText(
                    "auto reconnect is: %u",
                    CtrlThread->configure_info.auto_reconnect
                );
                break;

            case 0x59:
                if (cur_length != 6)
                    return;

                CtrlThread->private_properties.hello_interv =
                    ntohl(*reinterpret_cast<const uint32_t *>(&buf[cur_pos]));
                g_rjPrivateParselog.AppendText(
                    "%s heart interval:%u",
                    "ParsePrivateProperty",
                    CtrlThread->private_properties.hello_interv
                );
                break;

            case 0x5A:
                if (cur_length != 6 || len < cur_pos + 4)
                    return;

                CtrlThread->private_properties.indicate_port =
                    ntohl(*reinterpret_cast<const uint32_t *>(&buf[cur_pos]));
                g_rjPrivateParselog.AppendText(
                    "ChangeToEAPOLFrame()---->INDICATE_PORT = %d",
                    CtrlThread->private_properties.indicate_port
                );
                break;

            case 0x5B:
                if (cur_length != 6 || len < cur_pos + 4)
                    return;

                CtrlThread->private_properties.indicate_serv_ip =
                    ntohl(*reinterpret_cast<const uint32_t *>(&buf[cur_pos]));
                g_rjPrivateParselog.AppendText(
                    "ChangeToEAPOLFrame()---->INDICATE_SERV_IP = %0.8x",
                    CtrlThread->private_properties.indicate_serv_ip
                );
                break;

            case 0x5C:
                if (cur_length != 10 || len < cur_pos + 8)
                    return;

                memcpy(CtrlThread->private_properties.encrypt_key, &buf[cur_pos], 8);
                RC4(CtrlThread->private_properties.encrypt_key, "com.ruijie.www", 8);
                g_rjPrivateParselog.AppendText("ChangeToEAPOLFrame()---->ENCRYPT_KEY");
                g_rjPrivateParselog.HexPrinter(
                    CtrlThread->private_properties.encrypt_key,
                    8
                );
                break;

            case 0x5D:
                if (cur_length != 10 || len < cur_pos + 8)
                    return;

                memcpy(CtrlThread->private_properties.encrypt_iv, &buf[cur_pos], 8);
                RC4(CtrlThread->private_properties.encrypt_iv, "com.ruijie.www", 8);
                g_rjPrivateParselog.AppendText("ChangeToEAPOLFrame()---->ENCRYPT_IV");
                g_rjPrivateParselog.HexPrinter(
                    CtrlThread->private_properties.encrypt_iv,
                    8
                );
                break;

            case 0x5E:
                if (cur_length != 10 || len < cur_pos + 8)
                    return;

                memcpy(
                    reinterpret_cast<char *>
                    (CtrlThread->private_properties.server_utc_time),
                    &buf[cur_pos],
                    8
                );
                g_rjPrivateParselog.AppendText("ChangeToEAPOLFrame()---->SERVER_UTC_TIME");
                g_rjPrivateParselog.HexPrinter(
                    reinterpret_cast<char *>
                    (CtrlThread->private_properties.server_utc_time),
                    8
                );
                break;

            case 0x61:
                if (cur_length != 3 || len < cur_pos + 1)
                    return;

                CtrlThread->private_properties.radius_type = buf[cur_pos];
                g_log_Wireless.AppendText(
                    "Radius type:%d",
                    CtrlThread->private_properties.radius_type
                );
                CtrlThread->SetRadiusServer(CtrlThread->private_properties.radius_type);
                g_rjPrivateParselog.AppendText(
                    "ChangeToEAPOLFrame()---->RADIUS_SERV_TYPE = %d",
                    CtrlThread->GetRadiusServer()
                );
                CtrlThread->field_1149 = 0;
                logFile.AppendText("radius server is new version");
                break;

            case 0x65:
                if (len < cur_length - 2 + cur_pos)
                    return;

                CtrlThread->private_properties.svr_switch_result =
                    AsciiToStr(&buf[cur_pos], cur_length - 2);
                g_rjPrivateParselog.AppendText(
                    "ParsePrivateProperty()---->SvrSwitchResult=%s",
                    CtrlThread->private_properties.svr_switch_result.c_str()
                );
                break;

            case 0x66:
                if (len < cur_length - 2 + cur_pos || cur_length == 2)
                    return;

                logFile.AppendText("ParsePrivateProperty---------更新服务列表");
                ConvertGBKToUtf8(service_list_str, &buf[cur_pos], cur_length - 2);
                CtrlThread->private_properties.services.clear();
                ParseString(
                    service_list_str,
                    '@',
                    CtrlThread->private_properties.services
                );
                RcvSvrList(CtrlThread->private_properties.services);
                break;

            case 0x68:
                if (len < cur_length - 2 + cur_pos)
                    return;

                if (cur_length == 2)
                    break;

                CtrlThread->private_properties.user_login_url =
                    AsciiToStr(&buf[cur_pos], cur_length - 2);
                g_rjPrivateParselog.AppendText(
                    "ParsePrivateProperty()---->UserLoginUrl=%s",
                    CtrlThread->private_properties.user_login_url.c_str()
                );
                break;

            case 0x6E:
                if (cur_length != 6 || len < cur_pos + 4)
                    return;

                CtrlThread->private_properties.msg_client_port =
                    ntohl(*reinterpret_cast<const uint32_t *>(&buf[cur_pos]));
                g_rjPrivateParselog.AppendText(
                    "ChangeToEAPOLFrame()---->MSG_CLIENT_PORT %d",
                    CtrlThread->private_properties.msg_client_port
                );
                break;

            case 0x72:
                if (cur_length == 2)
                    break;

                CtrlThread->private_properties.utrust_url =
                    AsciiToStr(&buf[cur_pos], cur_length - 2);
                g_rjPrivateParselog.AppendText(
                    "ParsePrivateProperty()---->utrust_url=%s",
                    CtrlThread->private_properties.utrust_url.c_str()
                );
                break;

            case 0x74:
                if (cur_length != 6) {
                    g_rjPrivateParselog.AppendText(
                        "%s PROXY_DECTECT_KINDS len=%d error",
                        "ParsePrivateProperty",
                        cur_length
                    );
                    return;
                }

                CtrlThread->private_properties.proxy_dectect_kinds =
                    ntohl(*reinterpret_cast<const uint32_t *>(&buf[cur_pos]));
                g_rjPrivateParselog.AppendText(
                    "%s proxy type:%08x",
                    "ParsePrivateProperty",
                    CtrlThread->private_properties.proxy_dectect_kinds
                );
                break;

            case 0x77:
                if (cur_length != 5)
                    break;

                CtrlThread->private_properties.is_show_utrust_url = buf[cur_pos];
                CtrlThread->private_properties.delay_second_show_utrust_url =
                    ntohs(*reinterpret_cast<const uint16_t *>(&buf[cur_pos + 1]));
                g_rjPrivateParselog.AppendText(
                    "ParsePrivateProperty()----->IS_SHOW_UTRUST_URL = %d,"
                    "delaySecondShowUtrustUrl=%d",
                    CtrlThread->private_properties.is_show_utrust_url,
                    CtrlThread->private_properties.delay_second_show_utrust_url
                );
                break;

            case 0x79:
                if (cur_length != 3) {
                    g_dhcpDug.AppendText(
                        "Invalid length in the property"
                        "(direct-communication-highest-version-supported)."
                    );
                    return;
                }

                if (len < cur_pos + 1) {
                    g_dhcpDug.AppendText(
                        "When parsing the property"
                        "(direct-communication-highest-version-supported), "
                        "the private property packet length is too short."
                    );
                    return;
                }

                CtrlThread->private_properties.direct_communication_highest_version_supported =
                    buf[cur_pos];
                g_dhcpDug.AppendText(
                    "direct-communication-highest-version-supported is %d.",
                    CtrlThread->private_properties.direct_communication_highest_version_supported
                );
                break;

            case 0x80:
                if (cur_length != 6) {
                    g_dhcpDug.AppendText(
                        "Invalid length in the property"
                        "(direct-communication-heartbeat-flags)."
                    );
                    return;
                }

                if (len < cur_pos + 4) {
                    g_dhcpDug.AppendText(
                        "When parsing the property"
                        "(direct-communication-heartbeat-flags), "
                        "the private property packet length is too short."
                    );
                    return;
                }

                CtrlThread->private_properties.direct_comm_heartbeat_flags =
                    buf[cur_pos];
                g_dhcpDug.AppendText(
                    "direct-communication-heartbeat-flags is %u.",
                    CtrlThread->private_properties.direct_comm_heartbeat_flags
                );
        }
}

void EncapProgrammName(const std::string &prog_name, char *buf)
{
    char tmpbuf[128] = {};
    unsigned tmpbuflen = 0;
    ConvertUtf8ToGBK(buf, sizeof(buf), prog_name.c_str(), prog_name.length());
    memset(buf, 0, 32);

    if ((tmpbuflen = strlen(tmpbuf)) < 32)
        memcpy(buf, tmpbuf, tmpbuflen);
}

void EncapUCharVersionNumber(char *buf)
{
    unsigned major = 0, minor = 0;
    memset(buf, 0, 4);
    GetSuInternalVersion(major, minor);
    *buf++ = major;
    *buf++ = minor;
    *buf++ = 1;
    *buf++ = 2;
}
