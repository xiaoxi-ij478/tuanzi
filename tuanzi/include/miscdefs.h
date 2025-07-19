#ifndef MISCDEFS_H_INCLUDED
#define MISCDEFS_H_INCLUDED

#include "stdpkgs.h"
#include "waithandle.h"

#define MAX_NIC_NAME_LEN 0x200

#define LNXMSG_MSGSZ (sizeof(struct LNXMSG) - offsetof(struct LNXMSG, arg1))

struct LNXMSG {
    long mtype;
    unsigned long arg1;
    unsigned long arg2;
};

enum MODIFY_MODE {
    BEFOER_LOGIN_DISABLE, // the source code writes as this, not typo
    BEFOER_LOGIN_ENABLE,
    AFTER_LOGIN_DISABLE,
    AFTER_LOGIN_ENABLE
};

enum LANG {
    LANG_INVALID,
    LANG_ENGLISH,
    LANG_CHINESE
};

struct SPUpGradeInfo {
    unsigned magic;
    unsigned type;
    unsigned length;
    unsigned su_newest_ver;
    std::string su_upgrade_url;
};

struct HIFailInfo {
    unsigned long field_0;
    unsigned long field_8;
    unsigned field_10;
};

struct DetectNICInfo {
    char nic_name[MAX_NIC_NAME_LEN];
    in_addr_t ipaddr;
    struct ether_addr macaddr;
    key_t thread_key;
    int msgid;
    bool disallow_multi_nic_ip;
};

struct _START_CENTERCONTROL_START_ {
    in_addr_t ipv4;
    unsigned ipv6[4];
    unsigned product;
    unsigned major_ver;
    unsigned minor_ver;
    std::string domain;
    unsigned port;
    char mac[12];
    bool field_38; // field_218 @ CClientCenterPeerManager
};

struct CHostEnt {
    struct hostent hostent_entry;
    struct CHostEnt *hostent_next;
    unsigned long last_update_time;
};

struct tagDownLoadPara {
    pthread_t thread_id; // pThread
    unsigned mtype;
    std::string url;
    std::string save_path;
    std::string save_filename;
    bool create_progress_dialog;
};

enum ADAPTER_STATUS {
    ADAPTER_INVALID = -1,
    ADAPTER_UP = 1,
    ADAPTER_DOWN,
    ADAPTER_DISABLE,
    ADAPTER_ENABLE,
    ADAPTER_ERROR
};

enum ADAPTER_TYPE {
    ADAPTER_WIRELESS,
    ADAPTER_WIRED
};

struct NICINFO {
    char ifname[IFNAMSIZ];
    struct ether_addr hwaddr;
    bool use_dhcp;
    bool is_wireless;
    unsigned short speed;
    in_addr_t dns;
    in_addr_t gateway;
    struct ether_addr gateway_mac;
    unsigned ipaddr_count;
    struct IPAddrNode {
        in_addr_t ipaddr;
        in_addr_t netmask;
        struct IPAddrNode *next;
    } *ipaddrs;
    unsigned ipaddr6_count;
    struct IP6AddrNode {
        struct in6_addr ipaddr;
        struct in6_addr netmask;
        struct IP6AddrNode *next;
    } *ip6addrs;
    struct NICINFO *next;
};

struct DHClientThreadStruct {
    char ipaddr[512];
    sem_t *semaphore;
};

struct DHCPIPInfo {
    bool dhcp_enabled;
    in_addr_t ip4_ipaddr;
    in_addr_t ip4_netmask;
    in_addr_t gateway;
    in_addr_t dns;
    struct ether_addr adapter_mac;
    struct ether_addr gateway_mac;
    unsigned ipaddr6_count;
    struct in6_addr ip6_my_ipaddr;
    struct in6_addr ip6_link_local_ipaddr;
    struct in6_addr ip6_ipaddr;
    struct in6_addr ip6_netmask;
};

struct NICsStatus {
    NICsStatus() = default;
    NICsStatus(const char *nic_name_l, bool is_up) : is_up(is_up) {
        strncpy(nic_name, nic_name_l, sizeof(nic_name));
    }
    char nic_name[IFNAMSIZ];
    bool is_up;
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

struct tagMsgItem {
    tagMsgItem() = default;
    tagMsgItem(
        unsigned ntype,
        const std::string &msgtime,
        const std::string &msg
    ) : ntype(ntype), msgtime(msgtime), msg(msg)
    {}
    unsigned ntype;
    std::string msgtime;
    std::string msg;
};

enum STATES {
    STATE_INVALID,
    STATE_DISCONNECTED,
    STATE_CONNECTING,
    STATE_ACQUIRED,
    STATE_AUTHENTICATING,
    STATE_AUTHENTICATED,
    STATE_HOLD,
    STATE_LOGOFF,
    REACQURE,
    STATE_SUCCESSNOTIFY = 10,
    STATE_BEGIN = 12,
    STATE_DISASSOC,
    STATE_ROAM_SUCCESS,
    STATE_END
};

enum OP_STATE {
    OP_STATE_0,
    OP_STATE_1
};

enum APP_QUIT_TYPE {
    APP_QUIT_TYPE_0,
    APP_QUIT_TYPE_1,
    APP_QUIT_TYPE_2
};

enum LOGOFF_REASON {
    LOGOFF_REASON_UNKNOWN_REASON = 0,
    LOGOFF_REASON_NORMAL_LOGOFF = 1,
    LOGOFF_REASON_RUIJIE_MULTIPLE_NIC = 2,
    LOGOFF_REASON_RUIJIE_ADDRESS_CHANGED = 3,
    LOGOFF_REASON_RUIJIE_PROXY_DETECTED = 4,
    LOGOFF_REASON_RUIJIE_NIC_NOT_FOUND = 6,
    LOGOFF_REASON_RUIJIE_AUTH_FAIL = 8,
    LOGOFF_REASON_RUIJIE_FORCE_OFFLINE_3 = 17,
    LOGOFF_REASON_RUIJIE_FORCE_OFFLINE_2 = 20,
    LOGOFF_REASON_RUIJIE_COULD_NOT_COMM_WITH_SERVER = 25,
    LOGOFF_REASON_RUIJIE_OTHERS_FAKING_MAC = 26,
    LOGOFF_REASON_RUIJIE_SOCKS_PROXY = 27,
    LOGOFF_REASON_RUIJIE_HTTP_PROXY = 28,
    LOGOFF_REASON_MULTIPLE_NIC = 102,
    LOGOFF_REASON_ADDRESS_CHANGED = 103,
    LOGOFF_REASON_PROXY_DETECTED = 104,
    LOGOFF_REASON_NIC_NOT_FOUND = 106,
    LOGOFF_REASON_AUTH_FAIL = 108,
    LOGOFF_REASON_FORCE_OFFLINE_3 = 117,
    LOGOFF_REASON_FORCE_OFFLINE_2 = 120,
    LOGOFF_REASON_COULD_NOT_COMM_WITH_SERVER = 125,
    LOGOFF_REASON_OTHERS_FAKING_MAC = 126,
    LOGOFF_REASON_SOCKS_PROXY = 127,
    LOGOFF_REASON_HTTP_PROXY = 128,
    LOGOFF_REASON_NIC_NOT_CONNECTED = 1001,
    LOGOFF_REASON_NIC_DISABLED = 1002,
    LOGOFF_REASON_COMM_FAIL_NO_RESPONSE = 1003,
    LOGOFF_REASON_COMM_FAIL_TIMEOUT = 1004,
    LOGOFF_REASON_FAIL_GETTING_DYNAMIC_IP = 1005,
    LOGOFF_REASON_MULTIPLE_IP = 1006,
    LOGOFF_REASON_IP_CHANGED = 1007,
    LOGOFF_REASON_MAC_CHANGED = 1008,
    LOGOFF_REASON_MAX = 2000
};

enum OS_TYPE {
    OS_INVALID = -1,
    OS_FEDORA,
    OS_UBUNTU,
    OS_CENTOS
};

enum REQUEST_TYPE {
    REQUEST_UNKNOWN_N1 = -1,
    REQUEST_UNKNOWN_0,
    REQUEST_HTTP,
    REQUEST_SOCK4,
    REQUEST_SOCK4A,
    REQUEST_SOCK5,
    REQUEST_FTP,
    REQUEST_POP3,
    REQUEST_NNTP = 8,
    REQUEST_MMS,
    REQUEST_TELNET = 11,
    REQUEST_FAKING_MAC = 12
};

enum TRANS_DIRECTION {
    TRANS_RECV = -1,
    TRANS_MINE,
    TRANS_SEND
};

struct TcpInfo {
    in_addr_t dstaddr;
    in_addr_t srcaddr;
    unsigned short dstport;
    unsigned short srcport;
    unsigned seq;
    unsigned ack_seq;
};

struct TCPIP {
    const struct ether_header *etherheader;
    const struct iphdr *ipheader;
    const struct tcphdr *tcpheader;
    const char *content;
    unsigned content_length;
};

struct UdpListenParam {
    key_t mainthread;
    in_addr_t su_ipaddr;
    char ndisname[512];
    WAIT_HANDLE event_udp_ready;
};

struct UserInfo {
    unsigned username_len;
    unsigned password_len;
    std::string username;
    std::string password;
};

struct updateArg_t {
    bool update;
    std::string update_file_path;
    std::string update_message;
};

struct tagWirelessSignal {
    tagWirelessSignal() = default;
    tagWirelessSignal(char *ssid_l, unsigned ssid_len, unsigned qual) :
        ssid_len(ssid_len), qual(qual) {
        memcpy(ssid, ssid_l, ssid_len);
    }
    char ssid[33];
    unsigned ssid_len;
    unsigned qual;
};

#endif // MISCDEFS_H_INCLUDED
