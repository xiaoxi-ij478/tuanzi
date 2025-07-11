#ifndef EAPOLUTIL_H_INCLUDED
#define EAPOLUTIL_H_INCLUDED

#include "stdpkgs.h"
#include "netutil.h"

// in big-endian
#define FIELD_MAGIC 0x1311

struct SPUpGradeInfo {
    unsigned magic;
    unsigned type;
    unsigned length;
    unsigned su_newest_ver;
    std::string su_upgrade_url;
};

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

struct SuRadiusPrivate {
    unsigned su_newest_ver;
    std::string account_info;
    std::string persional_info;
    unsigned proxy_avoid;
    unsigned dialup_avoid;
    std::string su_upgrade_url;
    in_addr_t indicate_serv_ip;
    in_addr_t indicate_port;
    unsigned msg_client_port;
    unsigned hello_interv;
    char encrypt_key[8];
    char encrypt_iv[8];
    unsigned long server_utc_time;
    std::string fail_reason;
    std::string broadcase_info;
    unsigned su_reauth_interv;
    unsigned radius_type;
    std::string svr_switch_result;
    std::vector<std::string> services;
    std::string user_login_url;
    std::string utrust_url;
    unsigned is_show_utrust_url;
    unsigned delay_second_show_utrust_url;
    unsigned proxy_dectect_kinds;
    unsigned field_A4;
    unsigned parse_hello;
    unsigned parse_hello_inv;
    unsigned parse_hello_id;
    char direct_communication_highest_version_supported;
    unsigned direct_comm_heartbeat_flags;
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
extern void DhcpIpInfoToUChar(char *buf, struct EAPOLFrame *eapol_frame);
extern void EncapUCharDhcpIpInfo(char *buf, struct EAPOLFrame *eapol_frame);
extern void InitEAPOLFrame(struct EAPOLFrame *eapol_frame);
extern void AppendPrivateProperty(
    char *buf,
    unsigned &len,
    struct EAPOLFrame *eapol_frame
);
extern void ParsePrivateProperty(
    const char *buf,
    unsigned len,
    struct EAPOLFrame *eapol_frame
);

#endif // EAPOLUTIL_H_INCLUDED
