#include "all.h"
#include "compressor.h"
#include "cmdutil.h"
#include "timeutil.h"
#include "directtransfer.h"
#include "dirtranstags.h"
#include "encodeutil.h"
#include "mtypes.h"
#include "threadutil.h"
#include "userconfig.h"
#include "suconfigfile.h"
#include "msgutil.h"
#include "psutil.h"
#include "netutil.h"
#include "directtransrv.h"
#include "changelanguage.h"
#include "passwordmodifier.h"
#include "contextcontrolthread.h"
#include "global.h"
#include "util.h"

int TakeAppPath(std::string &dst)
{
    if (!g_strAppPath.empty()) {
        dst = g_strAppPath;
        return 1;
    }

    for (int i = 0; i < 3; i++) {
        std::string t;
        get_exe_name(t);

        if (t.rfind('/') != std::string::npos) {
            dst = t.substr(0, t.rfind('/') + 1);
            return 1;
        }
    }

    return -1;
}

void replace_all_distinct(
    std::string &str,
    const std::string &srcstr,
    const std::string &dststr
)
{
    size_t special_pos = 0;

    while ((special_pos = str.find(srcstr)) != std::string::npos)
        str.replace(special_pos, srcstr.length(), dststr);
}

void chk_call_back()
{
    rj_printf_debug("程序资源遭到破坏\n");
    exit(0);
}

unsigned addStringOnLineHead(
    const char *in_filename,
    const char *out_filename,
    const char *add_line_contain,
    const char *add_string
)
{
    std::ifstream ifs(in_filename);
    std::ofstream ofs(out_filename, std::ios::trunc);
    unsigned modified_line = 0;

    if (!ifs || !ofs) {
        g_log_Wireless.AppendText("addStringOnLineHead open file failed.");
        return 0;
    }

    std::string line;

    while (std::getline(ifs, line)) {
        if (line.find(add_line_contain) != std::string::npos) {
            ofs << add_string;
            modified_line++;
        }

        ofs << line << std::endl;
    }

    ifs.close();
    ofs.close();
    return modified_line;
}

int FindChar(char to_find, const char *str, int begin, int end)
{
    if (!str || begin < 0 || begin > end)
        return -2;

    const char *pos = std::find(str + begin, str + end, to_find);
    return pos == str + end ? pos - str : -1;
}

void ParseString(
    const std::string &str,
    char delim,
    std::vector<std::string> &dest
)
{
    std::istringstream iss(str);
    std::string tmp;
    dest.clear();

    while (std::getline(iss, tmp, delim))
        dest.emplace_back(tmp);
}

void ParseString(
    const std::string &str,
    char delim,
    std::vector<std::string> &dest,
    unsigned max_time
)
{
    unsigned i = 0;
    std::istringstream iss(str);
    std::string tmp;
    dest.clear();

    while (i++ < max_time && std::getline(iss, tmp, delim))
        dest.emplace_back(tmp);

    if (iss.eof())
        return;

    dest.emplace_back(str.substr(iss.tellg()));
}

void TrimLeft(std::string &str, const std::string &chars)
{
    std::string::size_type first = str.find_first_not_of(chars);

    if (first == std::string::npos) {
        str.clear();
        return;
    }

    if (first)
        str.erase(str.cbegin(), std::next(str.cbegin(), first));
}

void TrimRight(std::string &str, const std::string &chars)
{
    std::string::size_type last = str.find_last_not_of(chars);

    if (last == std::string::npos) {
        str.clear();
        return;
    }

    if (last != str.length())
        str.erase(std::next(str.cbegin(), last + 1), str.cend());
}

unsigned HexCharToAscii(
    const std::string &str,
    char *buf,
    unsigned buflen
)
{
    if (str.length() & 1 || buflen < str.length() << 1)
        return 0;

    for (auto it = str.cbegin(); it != str.cend(); it++, buf++) {
        if (*it >= '0' && *it <= '9')
            *buf = *it - '0';

        else if (*it >= 'A' && *it <= 'F')
            *buf = *it - 'A' + 10;

        else if (*it >= 'a' && *it <= 'f')
            *buf = *it - 'a' + 10;

        it++;
        *buf <<= 4;

        if (*it >= '0' && *it <= '9')
            *buf |= *it - '0';

        else if (*it >= 'A' && *it <= 'F')
            *buf |= *it - 'A' + 10;

        else if (*it >= 'a' && *it <= 'f')
            *buf |= *it - 'a' + 10;
    }

    *buf = 0;
    return str.length() >> 1;
}

std::string HexToString(const char *buf, int buflen)
{
    std::string ret;

    for (; buflen; buflen--, buf++) {
        char upper = *buf >> 4, lower = *buf & 0xF;

        if (/* upper >= 0 && */ upper <= 9)
            ret.push_back(upper + '0');

        else if (upper >= 10 && upper <= 15)
            ret.push_back(upper - 10 + 'A');

        if (/* lower >= 0 && */ lower <= 9)
            ret.push_back(lower + '0');

        else if (lower >= 10 && lower <= 15)
            ret.push_back(lower - 10 + 'A');
    }

    return ret;
}

// should be used to decode mac address
int ASCIIStrtoChar(const std::string &str, char *buf)
{
    if (!str.length())
        return 0;

    for (
        auto it = str.cbegin();
        it != str.cend() && std::distance(it, str.cbegin()) < 254;
        it++
    ) {
        if (*it == ':')
            continue;

        if (*it >= '0' && *it <= '9')
            *buf = *it - '0';

        else if (*it >= 'A' && *it <= 'F')
            *buf = *it - 'A' + 10;

        else if (*it >= 'a' && *it <= 'f')
            *buf = *it - 'a' + 10;

        it++;
        *buf <<= 4;

        if (*it >= '0' && *it <= '9')
            *buf |= *it - '0';

        else if (*it >= 'A' && *it <= 'F')
            *buf |= *it - 'A' + 10;

        else if (*it >= 'a' && *it <= 'f')
            *buf |= *it - 'a' + 10;

        buf++;
    }

    *buf = 0;
    return str.length() > 254 ? 255 : str.length();
}

std::string AsciiToStr(const char *buf, unsigned len)
{
    return std::string(buf, len);
}

unsigned MD5StrtoUChar(const std::string &str, char *buf)
{
    if (!str.length())
        return 0;

    for (auto it = str.cbegin(); it != str.cend(); it++, buf++) {
        if (*it >= '0' && *it <= '9')
            *buf = *it - '0';

        else if (*it >= 'A' && *it <= 'F')
            *buf = *it - 'A' + 10;

        else if (*it >= 'a' && *it <= 'f')
            *buf = *it - 'a' + 10;

        it++;
        *buf <<= 4;

        if (*it >= '0' && *it <= '9')
            *buf |= *it - '0';

        else if (*it >= 'A' && *it <= 'F')
            *buf |= *it - 'A' + 10;

        else if (*it >= 'a' && *it <= 'f')
            *buf |= *it - 'a' + 10;
    }

    *buf = 0;
    return str.length() >> 1;
}

int StringToHex(
    const std::string &str,
    char *retbuf,
    unsigned retbuflen
)
{
    if (str.length() & 1)
        return -3;

    if (retbuflen < str.length() / 2)
        return -1;

    if (!str.length())
        return 0;

    for (auto it = str.cbegin(); it != str.cend(); it++, retbuf++) {
        if (*it >= '0' && *it <= '9')
            *retbuf = *it - '0';

        else if (*it >= 'A' && *it <= 'F')
            *retbuf = *it - 'A' + 10;

        else if (*it >= 'a' && *it <= 'f')
            *retbuf = *it - 'a' + 10;

        else
            return -2;

        it++;
        *retbuf <<= 4;

        if (*it >= '0' && *it <= '9')
            *retbuf |= *it - '0';

        else if (*it >= 'A' && *it <= 'F')
            *retbuf |= *it - 'A' + 10;

        else if (*it >= 'a' && *it <= 'f')
            *retbuf |= *it - 'a' + 10;

        else
            return -2;
    }

    *retbuf = 0;
    return str.length() / 2;
}

void WriteRegUserInfo(
    const struct UserInfo &info
#ifdef BUILDING_UPDATER
    , const std::string &filename
#endif // BUILDING_UPDATER
)
{
    std::string apppath;
    dictionary *ini = nullptr;
    FILE *fp = nullptr;
#ifdef BUILDING_UPDATER
    apppath = filename;
#else
    TakeAppPath(apppath);
    apppath.append("fileReg.ini");
#endif // BUILDING_UPDATER

    if (!(ini = iniparser_load(apppath.c_str()))) {
        g_logSystem.AppendText("ini create[path=%s]failed", apppath.c_str());
        return;
    }

    iniparser_set(
        ini,
        "pu32list:unl2t1",
        std::to_string(info.username_len).c_str()
    );
    iniparser_set(
        ini,
        "pu32list:dcd2x",
        std::to_string(info.password_len).c_str()
    );
    iniparser_set(ini, "pu32list:ed2e1", info.username.c_str());
    iniparser_set(ini, "pu32list:gr2a1", info.password.c_str());

    if (!(fp = fopen(apppath.c_str(), "w")))
        return;

    iniparser_dump_ini(ini, fp);
    iniparser_freedict(ini);
    fclose(fp);
}

void ReadRegUserInfo(
    struct UserInfo &info
#ifdef BUILDING_UPDATER
    , const std::string &filename
#endif // BUILDING_UPDATER
)
{
    std::string apppath;
    dictionary *ini = nullptr;
#ifdef BUILDING_UPDATER
    apppath = filename;
#else
    TakeAppPath(apppath);
    apppath.append("fileReg.ini");
#endif // BUILDING_UPDATER

    // the original implementation include a "ini create[path=%s]failed"
    // but we have nowhere to put it (we use a different load strategy)
    if (!(ini = iniparser_load(apppath.c_str()))) {
        g_logSystem.AppendText("ini load[path=%s]failed", apppath.c_str());
        return;
    }

    info.username_len = iniparser_getint(ini, "pu32list:unl2t1", 5);
    info.password_len = iniparser_getint(ini, "pu32list:dcd2x", 5);
    info.username = iniparser_getstring(ini, "pu32list:ed2e1", "");
    info.password = iniparser_getstring(ini, "pu32list:gr2a1", "");
    iniparser_freedict(ini);
}

//bool SetLanFlag(unsigned flag)
//{
//    std::string regini_path;
//    dictionary *ini = nullptr;
//    FILE *fp = nullptr;
//    TakeAppPath(regini_path);
//    regini_path.append("\\").append("fileReg.ini");
//
//    if (!(ini = iniparser_load(regini_path.c_str()))) {
//        g_logSystem.AppendText(
//            "ini create[path=%s]failed",
//            regini_path.c_str()
//        );
//        return false;
//    }
//
//    iniparser_set(ini, "System:lantype", std::to_string(flag).c_str());
//
//    if (!(fp = fopen(regini_path.c_str(), "w")))
//        return false;
//
//    iniparser_dump_ini(ini, fp);
//    fclose(fp);
//    iniparser_freedict(ini);
//    return true;
//}

void GetSuInternalVersion(unsigned &major, unsigned &minor)
{
    if (
        CtrlThread &&
        CtrlThread->configure_info.softproduct_internalver_major &&
        CtrlThread->configure_info.softproduct_internalver_minor
    ) {
        major = CtrlThread->configure_info.softproduct_internalver_major;
        minor = CtrlThread->configure_info.softproduct_internalver_minor;
        g_log_Wireless.AppendText(
            "GetSuInternalVersion: majorVer=%d minorVer=%d.",
            major,
            minor
        );

    } else {
        major = 1;
        minor = 30;
        g_log_Wireless.AppendText("GetSuInternalVersion: use default version.");
    }
}

void RadiusEncrpytPwd(
    const char *md5_challenge,
    unsigned md5_challenge_len,
    const char *password,
    unsigned password_len,
    char *outbuf
)
{
    char tmpbuf[528] = {};
    char md5buf[16] = {};
    unsigned username_len = 0;
    MD5_CTX md5ctx;
    ConvertUtf8ToGBK(
        tmpbuf,
        512,
        CtrlThread->configure_info.last_auth_username.c_str(),
        CtrlThread->configure_info.last_auth_username.length()
    );
    username_len = strlen(tmpbuf);

    if (
        !md5_challenge ||
        md5_challenge_len != 16 ||
        !password ||
        password_len & 0xF ||
        !password_len ||
        !outbuf
    )
        return;

    for (unsigned i = 0; i < password_len >> 4; i++) {
        memcpy(&tmpbuf[username_len], i ? md5buf : md5_challenge, 16);
        MD5Init(&md5ctx);
        MD5Update(
            &md5ctx,
            reinterpret_cast<unsigned char *>(tmpbuf),
            username_len + 16
        );
        MD5Final(reinterpret_cast<unsigned char *>(md5buf), &md5ctx);

        for (unsigned j = 0; j < 16; j++)
            md5buf[j] ^= password[(i << 4) + j];

        memcpy(&outbuf[i << 4], md5buf, 16);
    }
}

void RcvSvrList(const std::vector<std::string> &service_list)
{
    CSuConfigFile conffile;

    for (const std::string &service : service_list)
        logFile.AppendText(service.c_str());

    if (!CtrlThread->IsServerlistUpdate(service_list)) {
        CtrlThread->service_list_updated = false;
        return;
    }

    CtrlThread->service_list_updated = true;

    if (CtrlThread->private_properties.services.empty())
        return;

    conffile.Lock();

    if (conffile.Open()) {
        conffile.WritePrivateProfileString("SERVER", "Custom", "1");
        conffile.WritePrivateProfileString("SERVER", "Modify", "0");
        conffile.WritePrivateProfileString(
            "SERVER",
            "Number",
            std::to_string(CtrlThread->private_properties.services.size()).c_str()
        );

        for (
            auto it = CtrlThread->private_properties.services.cbegin();
            it != CtrlThread->private_properties.services.cend();
            it++
        )
            conffile.WritePrivateProfileString(
                "SERVER",
                std::string("Name")
                .append(
                    std::to_string(
                        std::distance(
                            CtrlThread->private_properties.services.cbegin(),
                            it
                        )
                    )
                ).c_str(),
                (*it + '>' + *it).c_str()
            );
    }

    conffile.Close();
    conffile.Unlock();
    CtrlThread->configure_info.server_custom = 1;
    CtrlThread->configure_info.server_modify = 1;
    CtrlThread->configure_info.server_names.clear();
    CtrlThread->configure_info.server_alt_names.clear();
    CtrlThread->configure_info.server_names =
        CtrlThread->configure_info.server_alt_names =
            CtrlThread->private_properties.services;
    g_uilog.AppendText("RcvSvrList(WM_UPDATA_MAIN_WINDOW)");
    shownotify(
        CChangeLanguage::Instance().LoadString(200),
        CChangeLanguage::Instance().LoadString(96),
        10000
    );
}

void RcvSvrSwitchResult(const std::string &notify)
{
    if (notify.empty())
        return;

    shownotify(notify, CChangeLanguage::Instance().LoadString(95), 0);

    if (notify != "切换成功!")
        return;

    CtrlThread->configure_info.public_service = CtrlThread->service_name;
    CUserConfig::SuWriteConfigString(
        "PUBLIC",
        "Service",
        CtrlThread->configure_info.public_service.c_str()
    );
    AddMsgItem(5, notify);
    g_uilog.AppendText("RcvSvrSwitchResult(WM_UPDATA_MAIN_WINDOW)");
    CtrlThread->private_properties.svr_switch_result.clear();
    CtrlThread->changing_service = false;
}
