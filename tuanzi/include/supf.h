#ifndef SUPF_H_INCLUDED
#define SUPF_H_INCLUDED

#define WPA_KEY_MGMT_IEEE8021X 1

enum SUPF_EVENT_TYPE {
    SUPF_STATE,
    SUPF_MSG
};

enum SupfCmdType {
    SUPF_SCAN_CMD,
    SUPF_START_CMD,
    SUPF_STOP_CMD
};

enum SupfPipeCmdType : char {
    SUPF_PIPE_STOP_CMD,
    SUPF_PIPE_START_CMD,
    SUPF_PIPE_SCAN_CMD,
    SUPF_PIPE_EXIT_CMD
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

// actually from wpa_supplicant/src/eap_common/eap_defs.h
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
    unsigned flags;
    char ssid[33];
    size_t ssid_len;
    char bssid[6];
    int freq;
    unsigned short beacon_int;
    unsigned short caps;
    int qual;
    int noise;
    int level;
    struct su_wpa_ie wpa_ie;
};

struct ScanCmdCtx {
//    struct SupfWlanScanRes *res;
//    int num;
    int num;
    struct SupfWlanScanRes res[];
};

struct StartCmdCtx {
    enum EAP_TYPE_RFC eap_type;
    enum EAP_TYPE_RFC eap_type_internal;
    char identity[256];
    char password[256];
    char ssid[32];
    int ssid_len;
    char *private_data;
    int private_len;
};

struct SuPlatformParam {
    char driver_name[64];
    char ifname[128];
//    supf_event_callback event_callback;
    char *debug_file;
};

struct SupfCmd {
    enum SupfCmdType cmd_type;
    struct StartCmdCtx *cmd_ctx;
};

struct SupfMsgData {
    enum SupfMsg msg;
    const void *buf;
    unsigned len;
};

struct SupfPipeStateMsgData {
    enum SUPF_EVENT_TYPE type;
    union {
        enum SupfState state; // type == SUPF_STATE
        struct { // type == SUPF_MSG
            enum SupfMsg msg;
            unsigned len; // data's length
//            char data[]; // exact data
        };
    };
};

struct SupfPipeCmdMsgData {
    enum SupfPipeCmdType cmd;
    struct { // used only when cmd == SUPF_PIPE_START_CMD
        unsigned data_len;
        char private_data[];
    };
};

extern const char *getSupfMsgText(enum SupfMsg msg);
extern const char *getSupfStateText(enum SupfState state);
extern void *supf_event_callback_recv_fun(void *);
extern unsigned generate_network_config(
    int *read_config_pipe,
    const char *ssid,
    const char *identity,
    const char *password
);
extern unsigned generate_cmd_arguments(
    int *num,
    char ***arguments,
    const struct SuPlatformParam *param,
    int config_pipe_read
);
extern void *supf_thread_function(void *);
extern unsigned su_platform_init(const struct SuPlatformParam *param);
extern unsigned su_platform_deinit();
extern unsigned wpa_global_init(const struct SuPlatformParam *param);
extern void wpa_global_free();
extern unsigned su_platform_cmd(struct SupfCmd *cmd);
extern unsigned supf_start(struct StartCmdCtx *start_ctx);
extern void supf_msg_handle(const struct SupfMsgData *msg_data);
extern void supf_state_handle(enum SupfState state);
extern unsigned supf_wlan_scan();
extern int supf_write_config_to_pipe(const char *config_buf, int *pipe_read);

#endif // SUPF_H_INCLUDED
