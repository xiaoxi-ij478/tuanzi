#ifndef SUPF_H_INCLUDED
#define SUPF_H_INCLUDED

enum SUPF_EVENT_TYPE {
    SUPF_STATE,
    SUPF_MSG
};

enum SupfCmdType {
    SUPF_SCAN_CMD,
    SUPF_START_CMD,
    SUPF_STOP_CMD
};

enum SupfMsg {
    SUPF_MSG_SCAN_RES,
    SUPF_MSG_EAP_ERR,
    SUPF_MSG_EAP_SUC
};

enum SupfState {
    SUPF_STOP,
    SUPF_START,
    SUPF_WLAN_NOFOUND,
    SUPF_DISASSOC,
    SUPF_ASSOCIATING,
    SUPF_ASSOCIATED,
    SUPF_CONNECTING,
    SUPF_AUTHENTICATING,
    SUPF_4WAY_HANDSHAKE,
    SUPF_GROUP_HANDSHAKE,
    SUPF_COMPLETE_SUCCESS,
    SUPF_AUTH_TIMEOUT
};

enum EAP_TYPE_RFC {
    RFC_EAP_NONE,
    RFC_EAP_IDENTITY,
    RFC_EAP_NOTIFICATION,
    RFC_EAP_NAK,
    RFC_EAP_MD5,
    RFC_EAP_OTP,
    RFC_EAP_GTC,
    RFC_EAP_TLS = 0xD,
    RFC_EAP_LEAP = 0x11,
    RFC_EAP_SIM,
    RFC_EAP_TTLS = 0x15,
    RFC_EAP_AKA = 0x17,
    RFC_EAP_PEAP = 0x19,
    RFC_EAP_MSCHAPV2 = 0x1A,
    RFC_EAP_TLV = 0x21,
    RFC_AP_TNC = 0x26,
    RFC_EAP_FAST = 0x2B,
    RFC_EAP_PAX = 0x2E,
    RFC_EAP_PSK,
    RFC_EAP_SAKE,
    RFC_EAP_IKEV2,
    RFC_EAP_AKA_PRIME,
    RFC_EAP_GPSK,
    RFC_EAP_EXPANDED = 0xFE,
};

typedef void (*supf_event_callback)(enum SUPF_EVENT_TYPE, const void *);

struct su_wpa_ie {
    int wpa_ie_set;
    int proto;
    int pairwise_cipher;
    int group_cipher;
    int key_mgmt;
    int capabilities;
    int mgmt_group_cipher;
};

struct SupfWlanScanRes {
    unsigned int flags;
    unsigned char ssid[33];
    size_t ssid_len;
    unsigned char bssid[6];
    int freq;
    unsigned short beacon_int;
    unsigned short caps;
    int qual;
    int noise;
    int level;
    struct su_wpa_ie wpa_ie;
};

struct ScanCmdCtx {
    struct SupfWlanScanRes *res;
    int num;
};

struct StartCmdCtx {
    enum EAP_TYPE_RFC eap_type;
    enum EAP_TYPE_RFC eap_type_internal;
    char identity[256];
    char password[256];
    char ssid[32];
    int ssid_len;
    unsigned char *private_data;
    int private_len;
};

struct SuPlatformParam {
    char driver_name[64];
    char ifname[128];
    supf_event_callback event_callback;
    char *debug_file;
};

struct SupfCmd {
    enum SupfCmdType cmd_type;
    void *cmd_ctx;
};

struct SupfMsgData {
    enum SupfMsg msg;
    const void *buf;
    int len;
};


#endif // SUPF_H_INCLUDED
