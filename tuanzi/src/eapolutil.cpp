#include "all.h"
#include "global.h"
#include "rgprivateproc.h"
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

    if (length < 18 || length > 1514)
        return nullptr;

#define EXIT_IF_UNMATCH(cond) do if (!(cond)) { delete ret; return; } while (0)
    InitEAPOLFrame(ret);
    ret->dstaddr =
        *reinterpret_cast<struct ether_addr *>(eapol_pkg->etherheader.ether_dhost);
    ret->srcaddr =
        *reinterpret_cast<struct ether_addr *>(eapol_pkg->etherheader.ether_shost);
    EXIT_IF_UNMATCH(!IsLoopBack(ret->srcaddr));
    EXIT_IF_UNMATCH(
        IsHostDstMac(ret->dstaddr) ||
        Is8021xGroupAddr(ret->dstaddr) ||
        IsMulDstMac(ret->dstaddr) ||
        IsStarGroupDstMac(ret->dstaddr)
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
            ret->ieee8021x_packet_length - 4
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
        ret->eap_type_md5_data[ret->eap_type_md5_length]=0;
        if(CtrlThread->IsRuijieNas())
        {

        }else{
if(ret->eap_length>22){

}
        }
    }

#undef EXIT_IF_UNMATCH
}

struct eapolpkg *CreateEapolPacket(
    const struct EAPOLFrame *eapol_frame,
    unsigned *length
)
{
}

void DeleteFrameMemory(struct EAPOLFrame *eapol_frame)
{
}

void DhcpIpInfoToUChar(char *buf, struct EAPOLFrame *eapol_frame)
{
}

void EncapUCharDhcpIpInfo(char *buf, struct EAPOLFrame *eapol_frame)
{
}

void InitEAPOLFrame(struct EAPOLFrame *eapol_frame)
{
    eapol_frame->dstaddr = {};
    eapol_frame->srcaddr = {};
    eapol_frame->ether_type = 0;
    eapol_frame->ieee8021x_version = 0;
    eapol_frame->ieee8021x_packet_type = 0;
    eapol_frame->ieee8021x_packet_length = 0;
    eapol_frame->eap_code = 0;
    eapol_frame->eap_id = 0;
    eapol_frame->eap_length = 0;
    eapol_frame->eap_type = 0;
    eapol_frame->eap_type_md5_data = nullptr;
    eapol_frame->eap_data.eap_type_md5_length = 0;
    eapol_frame->fail_reason_magic = 0;
    eapol_frame->fail_reason_length = 0;
    memset(eapol_frame->fail_reason, 0, sizeof(eapol_frame->fail_reason));
    eapol_frame->dhcp_ipinfo.dhcp = 0;
    eapol_frame->dhcp_ipinfo.ip4_ipaddr = 0;
    eapol_frame->dhcp_ipinfo.ip4_netmask = 0;
    eapol_frame->dhcp_ipinfo.gateway = 0;
    eapol_frame->dhcp_ipinfo.dns = 0;
}

void AppendPrivateProperty(
    char *buf,
    unsigned &len,
    struct EAPOLFrame *eapol_frame
)
{
}

void ParsePrivateProperty(
    const char *buf,
    unsigned len,
    struct EAPOLFrame *eapol_frame
)
{
}
