#include "all.h"
#include "encryption.h"
#include "util.h"
#include "suconfigfile.h"
#include "userconfig.h"

void CUserConfig::DecryptPassword(std::string &password)
{
    char buf[32] = {};
    CEncryption::decrypt(buf,password.c_str());
    password = buf;
}

void CUserConfig::DecryptUserName(std::string &username)
{
    char buf[32] = {};
    CEncryption::decrypt(buf,username.c_str());
    username = buf;
}

void CUserConfig::EncryptPassword(std::string &password)
{
    char buf[32] = {};
    CEncryption::encrypt(buf,password.c_str());
    password = buf;
}

void CUserConfig::EncryptUserName(std::string &username)
{
    char buf[32] = {};
    CEncryption::encrypt(buf,username.c_str());
    username = buf;
}

bool CUserConfig::ParseWirelessConf(
    const std::string &wireless_confstr,
    struct tagWirelessConf *conf
)
{
    std::vector<std::string> fields;
    assert(conf);

    if (wireless_confstr.empty())
        return false;

    ParseString(wireless_confstr, '\x01', fields);
    conf->field_8 = fields[0];
    conf->field_10_len =
        StringToHex(
            fields[1],
            conf->field_10,
            sizeof(conf->field_10)
        );
    conf->field_38 = fields[2];
    conf->field_40 = fields[3];

    if (
        StringToHex(
            fields[4],
            reinterpret_cast<char *>(&conf->macaddr),
            sizeof(conf->macaddr)
        ) != sizeof(conf->macaddr)
    )
        return false;

    return true;
}

void CUserConfig::PrepareForRunboot(bool)
{}

bool CUserConfig::ReadConfigParam(struct SaveConfigureInfo &info)
{
    CSuConfigFile conffile;

    if (!conffile.Open()) {
        g_log_Wireless.AppendText("ReadConfigParam file open error.");
        conffile.Unlock();
        return false;
    }

#define GET_INT(name, domain, key, defval) \
    info.name = conffile.GetPrivateProfileInt((domain), (key), (defval))
#define GET_STRING(name, domain, key, defval) \
    conffile.GetPrivateProfileString((domain), (key), (defval), info.name)
    GET_INT(public_modifysave, "PUBLIC", "ModifySave", 1);
    GET_INT(public_disablesavebutton, "PUBLIC", "DisableSaveButton", 0);
    GET_INT(public_savecheck, "PUBLIC", "SaveCheck", 0);
    GET_INT(public_dhcp, "PUBLIC", "DHCP", -1);
    GET_INT(public_runboot, "PUBLIC", "RunBoot", 0);
    GET_INT(public_runconnect, "PUBLIC", "RunConnect", 0);
    GET_INT(public_servicerun, "PUBLIC", "ServiceRun", 0);
    // we have nowhere to put this
//    GET_STRING(, "PUBLIC", "Language", "CHS");
    GET_STRING(public_adapter, "PUBLIC", "Adapter", "");
    GET_STRING(public_wirelessconf, "PUBLIC", "WirelessConf", "");
    GET_STRING(public_title, "PUBLIC", "Title", "Ruijie Supplicant");
    GET_STRING(public_service, "PUBLIC", "Service", "");
    GET_INT(public_sutype, "PUBLIC", "SuType", 0);
    GET_STRING(public_selfsvrurl, "PUBLIC", "SelfSvrURL", "");
    GET_STRING(public_authmode, "PUBLIC", "AuthMode", "EAPMD5");
    GET_STRING(public_secdomain, "PUBLIC", "SecDomain", "");
    GET_INT(wireless_modify, "WIRELESS", "Modify", 0);

    for (
        unsigned i = 0,
        t = conffile.GetPrivateProfileInt("WIRELESS", "Number", 0);
        i < t;
        i++
    ) {
        std::string wireless_conf_str;
        struct tagWirelessConf wireless_conf = {};
        conffile.GetPrivateProfileString(
            "WIRELESS",
            std::string("WirelessConf").append(IntToString(i)).c_str(),
            "",
            wireless_conf_str
        );

        if (ParseWirelessConf(wireless_conf_str, &wireless_conf))
            info.wireless_confs.push_back(wireless_conf);
    }

    GET_INT(domain_custom, "DOMAIN", "Custom", 0);
    GET_INT(domain_modify, "DOMAIN", "Modify", 1);
    GET_INT(domain_value, "DOMAIN", "Value", 0);
    GET_INT(domain_softconfcustom, "DOMAIN", "SoftconfCustom", 0);
    GET_INT(domain_supportdomainauth, "DOMAIN", "SupportDomainAuth", 0);
    GET_INT(domain_supportsoftrunauth, "DOMAIN", "SupportSoftrunAuth", 0);
    GET_INT(domain_supportosloginauth, "DOMAIN", "SupportOsloginAuth", 0);
    GET_INT(domain_supportautorun, "DOMAIN", "SupportAutoRun", 0);
    GET_INT(dhcpmode_custom, "DHCPMODE", "Custom", 0);
    GET_INT(dhcpmode_modify, "DHCPMODE", "Modify", 1);
    GET_INT(dhcpmode_value, "DHCPMODE", "Value", 0);
    GET_INT(dhcpmode_timeout, "DHCPMODE", "TimeOut", 30);
    GET_STRING(dhcpmode_dhcpwayname, "DHCPMODE", "DhcpWayName", "");
    GET_INT(dhcpmode_enablebeforeauth, "DHCPMODE", "EnableBeforeAuth", 0);
    GET_INT(macmode_custom, "MACMODE", "Custom", 0);
    GET_INT(macmode_modify, "MACMODE", "Modify", 1);
    GET_INT(macmode_value, "MACMODE", "Value", 0);
    GET_INT(authparam_custom, "AUTHPARAM", "Custom", 0);
    GET_INT(authparam_modify, "AUTHPARAM", "Modify", 1);
    GET_INT(authparam_authtimeout, "AUTHPARAM", "AuthTimeout", 3);
    GET_INT(authparam_heldtimeout, "AUTHPARAM", "HeldTimeout", 1);
    GET_INT(authparam_starttimeout, "AUTHPARAM", "StartTimeout", 3);
    GET_INT(authparam_startnumber, "AUTHPARAM", "StartNumber", 3);
    GET_INT(reauth_custom, "REAUTH", "Custom", 0);
    GET_INT(reauth_modify, "REAUTH", "Modify", 1);
    GET_INT(server_enablesvrswitch, "SERVER", "EnableSvrSwitch", 0);
    GET_INT(server_custom, "SERVER", "Custom", 0);
    GET_INT(server_modify, "SERVER", "Modify", 0);
    GET_INT(server_number, "SERVER", "Number", 0);

    for (unsigned i = 0, t = info.server_number; i < t; i++) {
        std::string server_name_str;
        std::vector<std::string> splited_name;
        conffile.GetPrivateProfileString(
            "SERVER",
            std::string("Name").append(IntToString(i)).c_str(),
            "",
            server_name_str
        );
        ParseString(server_name_str, '>', splited_name);
        info.server_names.push_back(std::move(splited_name[0]));
        info.server_alt_names.push_back(std::move(splited_name[1]));
    }

    GET_INT(authmode_supportnum, "AUTHMODE", "SupportNum", 1);
    GET_INT(authmode_support, "AUTHMODE", "Support", 1);
    GET_INT(secdomain_secdomainenable, "SECDOMAIN", "SecDomainEnable", 0);

    for (
        unsigned i = 0,
        t = conffile.GetPrivateProfileInt("SECDOMAIN", "SecDomainCount", 0);
        i < t;
        i++
    ) {
        std::string secdomain_str;
        conffile.GetPrivateProfileString(
            "SECDOMAIN",
            std::string("SecDomain").append(IntToString(i)).c_str(),
            "",
            secdomain_str
        );
        info.secdomains.push_back(std::move(secdomain_str));
    }

    GET_INT(other_authhintenable, "OTHER", "AuthHintEnable", 0);
    GET_INT(other_ipdclientsetup, "OTHER", "IPDClientSetup", 0);
    GET_INT(
        client_manager_center_issupport,
        "CLIENT_MANAGER_CENTER",
        "IsSupport",
        0
    );

    if (info.client_manager_center_issupport) {
        GET_STRING(
            client_manager_center_serveraddr,
            "CLIENT_MANAGER_CENTER",
            "ServerAddr",
            ""
        );
        GET_INT(
            client_manager_center_serverlistenport,
            "CLIENT_MANAGER_CENTER",
            "IsSupport",
            0
        );
        GET_STRING(
            client_manager_center_sharepassword,
            "CLIENT_MANAGER_CENTER",
            "SharePassword",
            ""
        );
    }

    GET_STRING(
        softproduct_releasever,
        "SOFTPRODUCT",
        "ReleaseVer",
        ""
    );
    GET_INT(
        softproduct_internalver_major,
        "SOFTPRODUCT",
        "InternalVer_Major",
        0
    );
    GET_INT(
        softproduct_internalver_minor,
        "SOFTPRODUCT",
        "InternalVer_Minor",
        0
    );
#undef GET_INT
#undef GET_STRING
    g_log_Wireless.AppendText(
        "nInternalMajorVersion=%d nInternalMinorVersion=%d",
        info.softproduct_internalver_major,
        info.softproduct_internalver_minor
    );
    conffile.Close();
    conffile.Unlock();
    return true;
}

bool CUserConfig::ReadSupplicantConf()
{
    if (ReadConfigParam(CtrlThread->configure_info)) {
        ReadUsernameAndPW(
            CtrlThread->configure_info.last_auth_username,
            CtrlThread->configure_info.last_auth_password
        );
        return true;
    }

    g_log_Wireless.AppendText("ReadConfigParam error.");
    return false;
}

void CUserConfig::ReadUsernameAndPW(
    std::string &username,
    std::string &password
)
{
    struct UserInfo userinfo = {};
    ReadRegUserInfo(userinfo);

    if (userinfo.unl2t1) {
        DecryptUserName(userinfo.ed2e1);
        username = userinfo.ed2e1;
    }

    if (userinfo.dcd2x) {
        DecryptPassword(userinfo.gr2a1);
        password = userinfo.gr2a1;
    }
}

void CUserConfig::RegisteGn(unsigned)
{}

void CUserConfig::RegisteGnForWin7(unsigned)
{}

void CUserConfig::RunNTService(bool)
{}

void CUserConfig::SaveConfigParam()
{
    CSuConfigFile conffile;
    conffile.Lock();

    if (!conffile.Open()) {
        conffile.Unlock();
        return;
    }

    // *INDENT-OFF*
#define CONF_INFO (CtrlThread->configure_info)
#define WRITE_DIRECT_INT(domain, key, val) \
    conffile.WritePrivateProfileString((domain), (key), IntToString(val).c_str())
#define WRITE_DIRECT_FIELD(domain, key, val) \
    conffile.WritePrivateProfileString((domain), (key), (val))
#define WRITE_INT(domain, key, name) \
    WRITE_DIRECT_INT(domain, key, CONF_INFO.name)
#define WRITE_FIELD(domain, key, name) \
    WRITE_DIRECT_FIELD(domain, key, CONF_INFO.name)
    // *INDENT-ON*
    if (CONF_INFO.public_savecheck) {
        WRITE_FIELD("PUBLIC", "Name", last_auth_username);
        WRITE_INT("PUBLIC", "EncryptCount", last_auth_username.length());

    } else {
        WRITE_DIRECT_FIELD("PUBLIC", "Name", "");
        WRITE_DIRECT_INT("PUBLIC", "EncryptCount", 0);
    }

    WRITE_INT("PUBLIC", "SaveCheck", public_savecheck);
    WRITE_DIRECT_FIELD(
        "PUBLIC",
        "Language",
        CChangeLanguage::Instance().GetLanguage() == LANG_ENGLISH ?
        "ENG" :
        "CHS"
    );

    if (CONF_INFO.public_dhcp != -1)
        WRITE_INT("PUBLIC", "DHCP", public_dhcp);

    WRITE_INT("PUBLIC", "RunBoot", public_runboot);
    WRITE_INT("PUBLIC", "RunConnect", public_runconnect);
    WRITE_INT("PUBLIC", "ServiceRun", public_servicerun);
    WRITE_FIELD("PUBLIC", "Adapter", public_adapter);
    WRITE_FIELD("PUBLIC", "WirelessConf", public_wirelessconf);
    WRITE_FIELD("PUBLIC", "Service", public_service);
    WRITE_FIELD("PUBLIC", "SecDomain", public_secdomain);
    WRITE_FIELD("PUBLIC", "AuthMode", public_authmode);
    WRITE_INT("DOMAIN", "Value", domain_value);
    WRITE_INT("DHCPMODE", "Value", dhcpmode_value);
    WRITE_INT("DHCPMODE", "TimeOut", dhcpmode_timeout);
    WRITE_FIELD("DHCPMODE", "DhcpWayName", dhcpmode_dhcpwayname);
    WRITE_INT("MACMODE", "Value", macmode_value);
    WRITE_INT("WIRELESS", "Number", wireless_confs.length());

    for (
        auto it = CONF_INFO.wireless_confs.cbegin();
        it != CONF_INFO.wireless_confs.cend();
        it++
    ) {
        std::string wireless_conf_name("WirelessConf");
        std::string field_10_converted =
            HexToString(
                it->field_10,
                it->field_10_len
            );
        std::string macaddr_converted =
            HexToString(
                reinterpret_cast<char *>(&it->macaddr),
                sizeof(it->macaddr)
            );
        std::string wireless_conf_val;
        wireless_conf_name.append(
            std::to_string(
                std::distance(
                    it,
                    CONF_INFO.wireless_confs.cbegin()
                )
            )
        );
        wireless_conf_val
        .append(it->field_8)
        .append('\x01')
        .append(field_10_converted)
        .append('\x01')
        .append(it->field_38)
        .append('\x01')
        .append(it->field_40)
        .append('\x01')
        .append(macaddr_converted)
        .append("\x01end");
        WRITE_DIRECT_FIELD(
            "WIRELESS",
            wireless_conf_name.c_str(),
            wireless_conf_val.c_str()
        );
    }

    conffile.Close();
    conffile.Unlock();
}

void CUserConfig::SaveSupplicantConf()
{
    SaveConfigParam();
    SaveUsernameAndPW(
        CtrlThread->configure_info.last_auth_username,
        CtrlThread->configure_info.last_auth_password,
        CtrlThread->configure_info.public_savecheck
    );
}

void CUserConfig::SaveUsernameAndPW(
    std::string username,
    std::string password,
    bool write_password
)
{
    struct UserInfo userinfo = {};

    if (!username.empty())
        EncryptUserName(username);

    if (!password.empty())
        EncryptUserName(password);

    userinfo.unl2t1 = username.length();
    userinfo.dcd2x = write_password ? password.length() : 0;
    userinfo.ed2e1 = username;
    userinfo.gr2a1 = write_password ? "" : password;
    WriteRegUserInfo(userinfo);
}

void CUserConfig::SuWriteConfigString(
    const char *domain,
    const char *key,
    const char *val
)
{
    CSuConfigFile conffile;
    conffile.Lock();

    if (conffile.Open()) {
        conffile.WritePrivateProfileString(domain, key, val);
        conffile.Close();
    }

    conffile.Unlock();
}

void CUserConfig::W95StartService(unsigned)
{}
