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

void setAppEnvironment()
{
    char *path = getenv("PATH");
    char *new_path = nullptr;

    if (strstr(path, "/sbin"))
        return;

    new_path = new char[strlen(path) + strlen(":/sbin") + 1];
    strcpy(new_path, path);
    strcat(new_path, ":/sbin");
    setenv("PATH", new_path, 1);
    delete[] new_path;
    new_path = nullptr;
}

int TakeAppPath(std::string &dst)
{
    std::string t;

    if (!g_strAppPath.empty()) {
        dst = g_strAppPath;
        return 1;
    }

    for (int i = 0; i < 3; i++) {
        get_exe_name(t);

        if (t.rfind('/') != std::string::npos) {
            dst = t.substr(0, t.rfind('/') + 1);
            return 1;
        }
    }

    return -1;
}

void InitLogFiles()
{
#define INIT_LOG_OBJ(obj, rel_path) (obj).CreateLogFile_S(g_strAppPath + (rel_path), 3)
    INIT_LOG_OBJ(logFile_debug, "log/Debug_001.log");
    INIT_LOG_OBJ(g_logFile_Ser, "log/Debug_Server.log");
    INIT_LOG_OBJ(g_logFile_start, "log/Debug_start_yf.log");
    INIT_LOG_OBJ(g_log_Wireless, "log/Debug_Wireless_8021x.log");
    INIT_LOG_OBJ(g_logFile_proxy, "log/Debug_Proxy.log");
    INIT_LOG_OBJ(g_Logoff, "log/Debug_Logoff.log");
    INIT_LOG_OBJ(g_dhcpDug, "log/Debug_dhcp.log");
    INIT_LOG_OBJ(logFile, "log/Debug_logfile.log");
    INIT_LOG_OBJ(g_logSystem, "log/Debug_system.log");
    INIT_LOG_OBJ(g_Update, "log/Debug_update.log");
    INIT_LOG_OBJ(g_eapPeapLog, "log/Debug_eapPeap.log");
    INIT_LOG_OBJ(g_rjPrivateParselog, "log/jPrivateParse.log");
    INIT_LOG_OBJ(g_uilog, "log/ui.log");
    INIT_LOG_OBJ(g_WlanStateLog, "log/wlanState.log");
    INIT_LOG_OBJ(g_logContextControl, "log/Debug_ContextControl.log");
#undef INIT_LOG_OBJ
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

void chk_call_back(int)
{
    rj_printf_debug("程序资源遭到破坏\n");
    exit(0);
}

bool set_msg_config(const std::string &key, int val)
{
    // "sysctl -w kernel.$key=$val >&-"
    std::ofstream ofs;

    if (key != "msgmax" && key != "msgmnb" && key != "msgmni")
        return false;

    ofs.open("/proc/sys/kernel/" + key);

    if (!ofs || !(ofs << val))
        return false;

    ofs.close();
    return true;
}

void ChangeSelfSvrParam(void *)
{
    logFile.AppendText("change self svr param");
}

void CoInitialize()
{}

void CoUnInitialize()
{}

std::string DWordToString(unsigned a)
{
    return std::to_string(a);
}

bool DecryptSuConfig()
{
    std::ifstream ifs("SuConfig.dat");
    std::ofstream ofs("SuConfig_Decrypt.dat");
    char *ibuf = nullptr;
    char *obuf = nullptr;
    unsigned long orig_len = 0;
    unsigned long comp_len = 0;

    if (!ifs) {
        g_logSystem.AppendText("ERROR: Open file %s failed.\n", "SuConfig.dat");
        return false;
    }

    if (!ofs) {
        g_logSystem.AppendText("ERROR: Open file %s failed.\n", "SuConfig_Decrypt.dat");
        return false;
    }

    ifs.seekg(0, std::ios::end);
    orig_len = ifs.tellg();
    ibuf = new char[orig_len];
    ifs.seekg(0, std::ios::beg);
    ifs.read(ibuf, orig_len);
    ifs.close();
    comp_len = Decompress(ibuf, obuf, orig_len, 0);
    obuf = new char[comp_len];
    Decompress(ibuf, obuf, orig_len, comp_len);

    if (!ofs.write(obuf, comp_len)) {
        g_logSystem.AppendText(
            "ERROR: write file %s failed.\n",
            "SuConfig_Decrypt.dat"
        );
        return false;
    }

    ofs.close();
    return true;
}

bool EncryptSuConfig()
{
    std::ifstream ifs("SuConfig.dat");
    std::ofstream ofs("SuConfig_Encrypt.dat");
    char *ibuf = nullptr;
    char *obuf = nullptr;
    unsigned long orig_len = 0;
    unsigned long comp_len = 0;

    if (!ifs) {
        g_logSystem.AppendText("ERROR: Open file %s failed.\n", "SuConfig.dat");
        return false;
    }

    if (!ofs) {
        g_logSystem.AppendText(
            "ERROR: Open file %s failed.\n",
            "SuConfig_Encrypt.dat"
        );
        return false;
    }

    ifs.seekg(0, std::ios::end);
    orig_len = ifs.tellg();
    ibuf = new char[orig_len];
    ifs.seekg(0, std::ios::beg);
    ifs.read(ibuf, orig_len);
    ifs.close();
    comp_len = Compress(ibuf, obuf, orig_len, 0);
    obuf = new char[comp_len];
    Compress(ibuf, obuf, orig_len, comp_len);

    if (!ofs.write(obuf, comp_len)) {
        g_logSystem.AppendText(
            "ERROR: write file %s failed.\n",
            "SuConfig_Encrypt.dat"
        );
        return false;
    }

    ofs.close();
    return true;
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
    std::string line;
    unsigned modified_line = 0;

    if (!ifs || !ofs) {
        g_log_Wireless.AppendText("addStringOnLineHead open file failed.");
        return 0;
    }

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
    if (!str || begin < 0 || end < 0 || end > strlen(str))
        return -1;

    for (int i = begin; i < end; i++)
        if (str[i] == to_find)
            return i;

    return -1;
}

int FindSub(
    const char *to_find_buf,
    unsigned to_find_buf_len,
    const char *buf,
    unsigned begin,
    unsigned end
)
{
    bool match = true;

    if (!buf || end - begin + 1 < to_find_buf_len)
        return -1;

    for (unsigned i = begin; i < end; i++) {
        for (unsigned j = 0; j < to_find_buf_len; j++)
            if (to_find_buf[j] != buf[i + j]) {
                match = false;
                break;
            }

        if (match)
            return i;

        match = true;
    }

    return -1;
}

int GKillTimer(timer_t timer)
{
    int ret = 0;

    if ((ret = my_timer_delete(timer)) != -1)
        return ret;

    perror("timer_delete ");
    return -1;
}

void GOnTimer(union sigval)
{}

void GSNRecvPacket(char *, int)
{
    logFile.AppendText("receive gsn relative command!");
}

timer_t GSetTimer(
    int off_msec,
    void (*thread_function)(union sigval),
    struct TIMERPARAM *timer
)
{
    struct sigevent sev = {};
    struct itimerspec new_time = {
        { off_msec / 1000, off_msec % 1000 },
        { off_msec / 1000, off_msec % 1000 }
    };
    timer_t timerid = 0;

    if (!thread_function || !timer)
        return 0;

    memset(&sev, 0, sizeof(sev));
    sev.sigev_notify_function = thread_function;
    sev.sigev_value.sival_ptr = timer;
    sev.sigev_notify = SIGEV_THREAD;

    if (my_timer_create(CLOCK_REALTIME, &sev, &timerid) < 0)
        return 0;

    timer->ti = timerid;
    return my_timer_settime(timerid, TIMER_ABSTIME, &new_time, nullptr) < 0
           ? 0
           : timerid;
}

void GetMD5File(const char *filename, char *result)
{
    std::ifstream ifs(filename);
    char digest[16] = {};
    char buf[512] = {};
    MD5_CTX ctx;

    if (!ifs)
        return;

    MD5Init(&ctx);

    while (!ifs.eof()) {
        ifs.read(buf, sizeof(buf));
        MD5Update(&ctx, reinterpret_cast<unsigned char *>(buf), sizeof(buf));
    }

    ifs.close();
    MD5Final(reinterpret_cast<unsigned char *>(digest), &ctx);

    for (unsigned i = 0; i < sizeof(digest); i++) {
        *result++ = (digest[i] >> 4) + '0';
        *result++ = (digest[i] & 0xf) + '0';
    }
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
        dest.push_back(tmp);
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
        dest.push_back(tmp);

    if (iss.eof())
        return;

    dest.push_back(str.substr(iss.tellg()));
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

void HIPacketUpdate(const char *, int)
{
    logFile.AppendText("receive hi packet update command!");
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

std::string IntToString(int num)
{
    return std::to_string(num);
}

void KillRunModeCheckTimer()
{
    if (g_runModetimer) {
        my_timer_delete(g_runModetimer);
        g_runModetimer = 0;
    }
}

void OnRunModeCheckTimer(union sigval arg)
{
    if (is_run_background() != g_background) {
        g_uilog.AppendText(
            "OnRunModeCheckTimer runmode change is_run_background=%d,g_background=%d",
            !g_background,
            g_background
        );
        post_command('c');
    }

    if (modify_password_timeout(false) > 0)
        message_info(CChangeLanguage::Instance().LoadString(273));
}

void SetRunModeCheckTimer()
{
    struct sigevent sev = {};
    struct itimerspec new_time = { { 1, 0 }, { 1, 0 } };

    if (g_runModetimer)
        return;

    sev.sigev_notify = SIGEV_THREAD;
    sev.sigev_notify_function = OnRunModeCheckTimer;
    sev.sigev_value.sival_int = 1;

    if (my_timer_create(CLOCK_REALTIME, &sev, &g_runModetimer) == -1) {
        g_uilog.AppendText("SetRunModeCheckTimer my_timer_create error");
        return;
    }

    if (my_timer_settime(g_runModetimer, CLOCK_REALTIME, &new_time, nullptr) == -1)
        g_uilog.AppendText("SetRunModeCheckTimer my_timer_settime error");
}

int MemCmpare(const char *buf1, int begin, int end, const char *buf2, int len)
{
    if (!buf1 || !buf2 || end - begin + 1 < len)
        return -2;

    return memcmp(buf1 + begin, buf2, len);
}

void RcvACLParam(void *arg)
{
    logFile.AppendText("recv acl param");
    assert(arg);
}

void RcvCMD_GetProcessAndNetworkInfo()
{
    logFile.AppendText("recv cmd of get process and network info");
}

void RcvFlowMonitorParam(const void *arg)
{
    logFile.AppendText("recv flow monitor param");
    assert(arg);
}

void RcvIPMACChangeNotify()
{
    logFile.AppendText("recv ip mac change notify");
}

void RcvLoginURL([[maybe_unused]] const std::string &arg)
{
    logFile.AppendText("recv login url");
}

void RcvNetSecParam(const void *arg)
{
    logFile.AppendText("recv network security param");
    assert(arg);
}

void RcvOpenUtrustUrlCmd(const std::string &arg)
{
    logFile.AppendText("recv open utrust url cmd,url=%s", arg.c_str());
}

void RcvStartAuthNotification()
{
    logFile.AppendText("recv start auth notification");
}

void StrToLower(char *str)
{
    if (!str || !strlen(str))
        return;

    for (unsigned i = 0; i < strlen(str); i++)
        str[i] = tolower(str[i]);
}

bool convertInt(const char *str, int &result)
{
    result = strtol(str, nullptr, 10);
    return true;
}

void decode(char *buf, unsigned buflen)
{
    if (buflen <= 0)
        return;

    while (buflen--) {
        *buf = ((~*buf & 0x10) >> 1) |
               ((~*buf & 0x20) >> 3) |
               ((~*buf & 0x40) >> 5) |
               ((~*buf & 0x80) >> 7) |
               ((~*buf & 0x08) << 1) |
               ((~*buf & 0x04) << 3) |
               ((~*buf & 0x02) << 5) |
               ((~*buf & 0x01) << 7);
        buf++;
    }
}

void encode(char *buf, unsigned buflen)
{
    return decode(buf, buflen);
}

std::string makeLower(const std::string &str)
{
    std::string ret;
    ret.resize(str.length());
    std::transform(str.cbegin(), str.cend(), ret.begin(), tolower);
    return ret;
}

std::string makeUpper(const std::string &str)
{
    std::string ret;
    ret.resize(str.length());
    std::transform(str.cbegin(), str.cend(), ret.begin(), toupper);
    return ret;
}

int StringToHex(
    const std::string &str,
    char *retbuf,
    int retbuflen
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

void SimulateSuLogoff(char *buf, unsigned buflen)
{
    logFile.AppendText("receive simulate su logoff command!");
    CtrlThread->PostThreadMessage(
        SIMULATE_SU_LOGOFF_MTYPE,
        buflen,
        reinterpret_cast<unsigned long>(buf)
    );
}

bool SetLanFlag(unsigned flag)
{
    std::string regini_path;
    dictionary *ini = nullptr;
    FILE *fp = nullptr;
    TakeAppPath(regini_path);
    regini_path.append("\\").append("fileReg.ini");

    if (!(ini = iniparser_load(regini_path.c_str()))) {
        g_logSystem.AppendText(
            "ini create[path=%s]failed",
            regini_path.c_str()
        );
        return false;
    }

    iniparser_set(ini, "System:lantype", std::to_string(flag).c_str());

    if (!(fp = fopen(regini_path.c_str(), "w")))
        return false;

    iniparser_dump_ini(ini, fp);
    fclose(fp);
    iniparser_freedict(ini);
    return true;
}

void RecvSecdomainPacket(char *buf, unsigned buflen)
{
    logFile.AppendText("receive secdomain update command!");
    PostThreadMessage(
        theApp.thread_key,
        RECEIVE_SEC_DOMAIN_MTYPE,
        buflen,
        reinterpret_cast<unsigned long>(buf)
    );
}

void CopyGradeInfo(struct SPUpGradeInfo &dst, const struct SPUpGradeInfo &src)
{
    dst = src;
}

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

char GetHIRusultByLocal()
{
    return 0;
}

extern void RcvSvrList(const std::vector<std::string> &service_list)
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
            IntToString(CtrlThread->private_properties.services.size()).c_str()
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
                    IntToString(
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

bool IsUpgrade(unsigned ver)
{
    unsigned major = 0, minor = 0;
    GetSuInternalVersion(major, minor);
    g_log_Wireless.AppendText(
        "GetSuInternalVersion majorVer=%d, minorVer=%d",
        major,
        minor
    );
    g_log_Wireless.AppendText(
        "IsUpgrade newHI=%d, oldHI=%d",
        ver >> 16,
        (minor & 0xff) | ((major & 0xff) << 8)
    );
    return (ver >> 16) > ((minor & 0xff) | ((major & 0xff) << 8));
}

bool GetHIResult(
    const std::vector<struct HIFailInfo> &a1,
    unsigned long a2,
    unsigned a3
)
{
    struct HIFailInfo *info = new struct HIFailInfo;

    if (!info) {
        rj_printf_debug("Memory allocation error\n");
        return false;
    }

    info->field_0 = 0;
    info->field_8 = a2;
    info->field_10 = a3;
    return PostThreadMessage(
               theApp.thread_key,
               GET_HI_RESULT_MTYPE,
               reinterpret_cast<unsigned long>(&a1),
               reinterpret_cast<unsigned long>(info)
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
    CtrlThread->field_1139 = 0;
}

void RcvModifyPasswordResult(bool change_success, const char *fail_msg)
{
    std::string fail_msg_str;
    struct tagPasSecurityInfo *secinfo = nullptr;
    logFile.AppendText("recv modify password result");
    modify_password_timeout(true);

    if (change_success) {
        secinfo = CPasswordModifier::GetPasswordSecurityInfo();
        secinfo->result = 1;
        secinfo->force_offline = 0;
        secinfo->password_modify_message.clear();
        CPasswordModifier::SetPasswordSecurityInfo(secinfo);
        CPasswordModifier::UpdateToNewPassword();
        message_info(CChangeLanguage::Instance().LoadString(274) + '\n');

    } else if (fail_msg) {
        ConvertUtf8ToGBK(fail_msg, strlen(fail_msg), fail_msg_str);
        message_info(fail_msg_str.append("\n"));
    }
}

void DoWithServiceSwitch_NoRuijieNas()
{
    char tmpbuf[128] = {};
    char finalbuf[300] = {};
    unsigned finalbuflen = 0;
    struct DHCPIPInfo dhcp_ipinfo = {};
#define PUT_TYPE(type) finalbuf[finalbuflen++] = (type)
#define PUT_LENGTH(length) finalbuf[finalbuflen++] = (length)
#define PUT_DATA(buf, buflen) \
    do { \
        memcpy(&finalbuf[finalbuflen], (buf), (buflen)); \
        finalbuflen += (buflen); \
    } while (0)
#define PUT_DATA_IMMEDIATE_BYTE(byte) finalbuf[finalbuflen++] = (byte)
#define PUT_DATA_IMMEDIATE_UINT32(value) \
    do { \
        *reinterpret_cast<uint32_t *>(&finalbuf[finalbuflen]) = \
                htonl(value); \
        finalbuflen += 4; \
    } while (0)
    ConvertUtf8ToGBK(
        tmpbuf,
        sizeof(tmpbuf),
        CtrlThread->configure_info.last_auth_username.c_str(),
        CtrlThread->configure_info.last_auth_username.length()
    );
    PUT_TYPE(0x01);
    PUT_LENGTH(0x01);
    PUT_DATA_IMMEDIATE_BYTE(0x06);
    PUT_TYPE(0x03);
    PUT_LENGTH(strlen(tmpbuf));
    PUT_DATA(tmpbuf, strlen(tmpbuf));
    InitDhcpIpInfo(dhcp_ipinfo);
    CtrlThread->GetDHCPInfoParam(dhcp_ipinfo);
    PUT_TYPE(0x04);
    PUT_LENGTH(0x04);
    PUT_DATA_IMMEDIATE_UINT32(ntohl(dhcp_ipinfo.ip4_ipaddr));
    PUT_TYPE(0x10);
    PUT_LENGTH(0x01);
    PUT_DATA_IMMEDIATE_BYTE(dhcp_ipinfo.dhcp_enabled);
    PUT_TYPE(0x05);
    PUT_LENGTH(0x06);
    CtrlThread->GetAdapterMac(
        reinterpret_cast<struct ether_addr *>(&finalbuf[finalbuflen])
    );
    finalbuflen += 6;
    ConvertUtf8ToGBK(
        tmpbuf,
        sizeof(tmpbuf),
        CtrlThread->service_name.c_str(),
        CtrlThread->service_name.length()
    );
    PUT_TYPE(0x09);
    PUT_LENGTH(strlen(tmpbuf));
    PUT_DATA(tmpbuf, strlen(tmpbuf));
#undef PUT_DATA
#undef PUT_LENGTH
#undef PUT_TYPE
#undef PUT_DATA_IMMEDIATE_BYTE
#undef PUT_DATA_IMMEDIATE_UINT32
    assert(finalbuflen <= 300);

    if (CtrlThread->dir_tran_srv) {
        CtrlThread->dir_tran_srv->PostToSam(finalbuf, finalbuflen);
        logFile.AppendText("no ruijie nas switch service successly");
    }
}

void DoWithServiceSwitch_RuijieNas()
{
    CtrlThread->field_1139 = true;
    CtrlThread->PostThreadMessage(REAUTH_MTYPE, 0, 0);
}

void InitAppMain()
{
    CtrlThread = new CContextControlThread;
    CtrlThread->CreateThread(nullptr, false);

    if (CtrlThread->StartThread()) {
        ShowLocalMsg("Create Main Thread Failed", "RG-SU");
        return;
    }

    g_log_Wireless.AppendText(
        "CtrlThread->GetMessageID msgid=%d\n",
        CtrlThread->GetMessageID()
    );
}

void ServiceSwitch(const std::string &new_name)
{
    auto pos =
        std::find(
            CtrlThread->configure_info.server_names.cbegin(),
            CtrlThread->configure_info.server_names.cend(),
            new_name
        );

    if (pos == CtrlThread->configure_info.server_names.cend())
        return;

    CtrlThread->service_name =
        *std::next(
            CtrlThread->configure_info.server_alt_names.cbegin(),
            std::distance(CtrlThread->configure_info.server_names.cbegin(), pos)
        );

    if (CtrlThread->IsRuijieNas())
        DoWithServiceSwitch_RuijieNas();

    else
        DoWithServiceSwitch_NoRuijieNas();
}

void TrimAll(std::string &str, const std::string &chars)
{
    TrimLeft(str, chars);
    TrimRight(str, chars);
}
