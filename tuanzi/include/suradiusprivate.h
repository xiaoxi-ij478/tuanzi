#ifndef SURADIUSPRIVATE_H_INCLUDED
#define SURADIUSPRIVATE_H_INCLUDED

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
    std::string broadcast_str;
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
    bool direct_communication_highest_version_supported;
    unsigned direct_comm_heartbeat_flags;
};

#endif // SURADIUSPRIVATE_H_INCLUDED
