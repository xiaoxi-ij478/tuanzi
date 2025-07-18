#ifndef SAVECONFIGUREINFO_H_INCLUDED
#define SAVECONFIGUREINFO_H_INCLUDED

#include "miscdefs.h"

struct SaveConfigureInfo {
    std::string last_auth_username;
    std::string last_auth_password;
    std::string public_title;
    std::string public_service;
    std::string public_selfsvrurl;
    std::string public_secdomain;
    std::string public_authmode;
    std::string public_adapter;
    std::string public_wirelessconf;
    unsigned public_sutype;
    bool public_modifysave;
    bool public_disablesavebutton;
    bool public_savecheck;
    bool public_runboot;
    bool public_runconnect;
    int public_dhcp;
    unsigned field_58;
    struct DHCPIPInfo dhcp_ipinfo;
    bool domain_custom;
    bool domain_modify;
    bool domain_value;
    bool domain_softconfcustom;
    bool domain_supportdomainauth;
    bool domain_supportosloginauth;
    bool domain_supportsoftrunauth;
    bool domain_supportautorun;
    bool dhcpmode_custom;
    bool dhcpmode_modify;
    unsigned dhcpmode_value;
    unsigned dhcpmode_timeout;
    std::string dhcpmode_dhcpwayname;
    bool dhcpmode_enablebeforeauth;
    bool macmode_custom;
    bool macmode_modify;
    bool field_E3;
    unsigned macmode_value;
    bool authparam_custom;
    bool authparam_modify;
    unsigned authparam_authtimeout;
    unsigned authparam_heldtimeout;
    unsigned authparam_starttimeout;
    unsigned authparam_startnumber;
    bool reauth_custom;
    bool reauth_modify;
    bool is_autoreconnect;
    unsigned autoreconnect;
    bool server_enablesvrswitch;
    bool server_custom;
    bool server_modify;
    unsigned server_number;
    std::vector<std::string> server_names;
    std::vector<std::string> server_alt_names;
    bool secdomain_secdomainenable;
    std::vector<std::string> secdomains;
    unsigned authmode_supportnum;
    unsigned authmode_support;
    bool wireless_modify;
    bool other_authhintenable;
    bool other_ipdclientsetup;
    bool client_manager_center_issupport;
    std::string client_manager_center_serveraddr;
    unsigned short client_manager_center_serverlistenport;
    std::string client_manager_center_sharepassword;
    std::string softproduct_releasever;
    unsigned softproduct_internalver_major;
    unsigned softproduct_internalver_minor;
    bool public_servicerun;
    std::vector<struct tagWirelessConf> wireless_confs;
};

#endif // SAVECONFIGUREINFO_H_INCLUDED
