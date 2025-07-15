#ifndef EAPOLUTIL_H_INCLUDED
#define EAPOLUTIL_H_INCLUDED

#include "stdpkgs.h"
#include "netutil.h"
#include "util.h"

#define FIELD_MAGIC 0x1311

struct EAPOLFrame {
    struct ether_addr dstaddr;
    struct ether_addr srcaddr;
    struct ether_addr field_C;
    unsigned short ether_type;
    char ieee8021x_version;
    enum IEEE8021X_PACKET_TYPE ieee8021x_packet_type;
    unsigned short ieee8021x_packet_length;
    enum EAP_CODES eap_code;
    char eap_id;
    unsigned short eap_length;
    unsigned field_1C;
    enum EAP_TYPES eap_type;
    union {
        uint8_t eap_type_md5_length;
        char *eap_type_notif_data;
    };
    char *eap_type_md5_data;
    unsigned fail_reason_magic;
    unsigned short fail_reason_length;
    char fail_reason[1500];
    struct SPUpGradeInfo upgrade_info;
    unsigned proxy_avoid_magic;
    char proxy_avoid_val;
    char proxy_avoid_val2;
    unsigned dont_know_field_magic;
    char dont_know_field_val;
    char dont_know_field_val2;
    unsigned parse_hello_magic;
    char parse_hello_val;
    char parse_hello_val2;
    unsigned parse_hello_id;
    unsigned parse_hello_inv;
    char parse_hello_val3;
    struct DHCPIPInfo dhcp_ipinfo;
    char field_6C0;
};

extern struct eapolpkg *ChangeToUChar(
    const struct EAPOLFrame *eapol_frame,
    unsigned *length
);
extern struct EAPOLFrame *ChangeToEAPOLFrame(
    const struct eapolpkg *eapol_pkg,
    unsigned length
);
extern struct eapolpkg *CreateEapolPacket(
    const struct EAPOLFrame *eapol_frame,
    unsigned *length
);
extern void DeleteFrameMemory(struct EAPOLFrame *eapol_frame);
extern void DhcpIpInfoToUChar(char *buf, const struct EAPOLFrame *eapol_frame);
extern void EncapUCharDhcpIpInfo(
    char *buf,
    const struct EAPOLFrame *eapol_frame
);
extern void InitEAPOLFrame(struct EAPOLFrame *eapol_frame);
extern void AppendPrivateProperty(
    char *buf,
    unsigned &len,
    const struct EAPOLFrame *eapol_frame
);
extern void ParsePrivateProperty(
    const char *buf,
    unsigned len,
    struct EAPOLFrame *eapol_frame
);
extern void EncapProgrammName(const std::string &prog_name, char *buf);
extern void EncapUCharVersionNumber(char *buf);

#endif // EAPOLUTIL_H_INCLUDED
