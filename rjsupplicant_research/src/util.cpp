#include "compressor.h"
#include "cmdutil.h"
#include "changelanguage.h"
#include "timeutil.h"
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

void get_exe_name(std::string &dst)
{
    char s[1024] = {};
    int r = readlink("/proc/self/exe", s, sizeof(s));
    dst = r == -1 ? "" : s;
}

enum LANG GetSysLanguage()
{
    return strncmp(getenv("LANG"), "zh_", 3) ? LANG_ENGLISH : LANG_CHINESE;
}

void InitLogFiles()
{
#define INIT_LOG_OBJ(obj, rel_path) obj.CreateLogFile_S(g_strAppPath + rel_path, 3)
    INIT_LOG_OBJ(logFile_debug,       "log/Debug_001.log");
    INIT_LOG_OBJ(g_logFile_Ser,       "log/Debug_Server.log");
    INIT_LOG_OBJ(g_logFile_start,     "log/Debug_start_yf.log");
    INIT_LOG_OBJ(g_log_Wireless,      "log/Debug_Wireless_8021x.log");
    INIT_LOG_OBJ(g_logFile_proxy,     "log/Debug_Proxy.log");
    INIT_LOG_OBJ(g_Logoff,            "log/Debug_Logoff.log");
    INIT_LOG_OBJ(g_dhcpDug,           "log/Debug_dhcp.log");
    INIT_LOG_OBJ(logFile,             "log/Debug_logfile.log");
    INIT_LOG_OBJ(g_logSystem,         "log/Debug_system.log");
    INIT_LOG_OBJ(g_Update,            "log/Debug_update.log");
    INIT_LOG_OBJ(g_eapPeapLog,        "log/Debug_eapPeap.log");
    INIT_LOG_OBJ(g_rjPrivateParselog, "log/jPrivateParse.log");
    INIT_LOG_OBJ(g_uilog,             "log/ui.log");
    INIT_LOG_OBJ(g_WlanStateLog,      "log/wlanState.log");
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

void CoUnInitialize(unsigned)
{}

std::string DWordToString(unsigned a)
{
    std::ostringstream oss;
    oss << a;
    return oss.str();
}

bool DecryptSuConfig()
{
    std::ifstream ifs("SuConfig.dat", std::ios::binary);
    std::ofstream ofs("SuConfig_Decrypt.dat", std::ios::binary);
    unsigned char *ibuf = nullptr;
    unsigned char *obuf = nullptr;
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
    ibuf = new unsigned char[orig_len];
    ifs.seekg(0, std::ios::beg);
    ifs.read(reinterpret_cast<char *>(ibuf), orig_len);
    ifs.close();
    comp_len = Decompress(ibuf, obuf, orig_len, 0);
    obuf = new unsigned char[comp_len];
    Decompress(ibuf, obuf, orig_len, comp_len);

    if (!ofs.write(reinterpret_cast<const char *>(obuf), comp_len)) {
        g_logSystem.AppendText(
            "ERROR: write file %s failed.\n",
            "SuConfig_Decrypt.dat"
        );
        return false;
    }

    return true;
}

bool EncryptSuConfig()
{
    std::ifstream ifs("SuConfig.dat", std::ios::binary);
    std::ofstream ofs("SuConfig_Encrypt.dat", std::ios::binary);
    unsigned char *ibuf = nullptr;
    unsigned char *obuf = nullptr;
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
    ibuf = new unsigned char[orig_len];
    ifs.seekg(0, std::ios::beg);
    ifs.read(reinterpret_cast<char *>(ibuf), orig_len);
    ifs.close();
    comp_len = Compress(ibuf, obuf, orig_len, 0);
    obuf = new unsigned char[comp_len];
    Compress(ibuf, obuf, orig_len, comp_len);

    if (!ofs.write(reinterpret_cast<const char *>(obuf), comp_len)) {
        g_logSystem.AppendText(
            "ERROR: write file %s failed.\n",
            "SuConfig_Encrypt.dat"
        );
        return false;
    }

    return true;
}

void exec_cmd(const char *cmd, char *buf, int buflen)
{
    FILE *fp = popen(cmd, "r");
    unsigned read_len = 0;

    if (!fp) {
        rj_printf_debug("exec_cmd popen null:%s\n", strerror(errno));
        return;
    }

    if ((read_len = fread(buf, 1, buflen - 1, fp)) <= 0) {
        rj_printf_debug("exec_cmd fread size <= 0\n");

        if (buflen)
            *buf = 0;

    } else
        buf[read_len] = 0;

    pclose(fp);
}

float get_fedora_lib_version([[maybe_unused]] const char *pkgname)
{
    // I'm a Debian fan, so I don't know how to use yum
    // yum list installed |grep $pkgname |awk 'NR==1 {print $2}'
    // printf "%s version=%s\n" "get_fedora_lib_version" $version
    return 0.0;
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

void GSNRecvPacter(unsigned char *, int)
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
    unsigned char digest[16] = {};
    unsigned char buf[0x200] = {};
    MD5_CTX ctx;

    if (!ifs)
        return;

    MD5Init(&ctx);

    while (!ifs.eof()) {
        ifs.read(reinterpret_cast<char *>(buf), sizeof(buf));
        MD5Update(&ctx, buf, sizeof(buf));
    }

    ifs.close();
    MD5Final(digest, &ctx);

    for (unsigned i = 0; i < 16; i++) {
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

    while (std::getline(iss, tmp, delim)) {
        dest.push_back(tmp);

        while (iss.get() == ' ');

        if (iss.eof())
            break;

        iss.unget();
    }
}

void TrimLeft(std::string &str, std::string chars)
{
    if (str.find_first_not_of(chars) == std::string::npos)
        str.clear();

    str.erase(0, str.find_first_not_of(chars) - 1);
}

void TrimRight(std::string &str, std::string chars)
{
    if (str.find_last_not_of(chars) == std::string::npos)
        str.clear();

    str.erase(str.find_last_not_of(chars) + 1);
}

void HIPacketUpdate(unsigned char *, int)
{
    logFile.AppendText("receive hi packet update command!");
}

unsigned HexCharToAscii(
    const std::string &str,
    unsigned char *buf,
    unsigned buflen
)
{
    if (str.length() & 1 || buflen < str.length() << 1)
        return 0;

    for (
        std::string::const_iterator it = str.cbegin();
        it != str.cend();
        it++, buf++
    ) {
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

std::string HexToString(const unsigned char *buf, int buflen)
{
    std::string ret;
    unsigned char upper = 0, lower = 0;

    for (; buflen; buflen--, buf++) {
        upper = *buf >> 4;
        lower = *buf & 0xF;

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
int ASCIIStrtoChar(std::string str, unsigned char *buf)
{
    if (!str.length())
        return 0;

    for (
        std::string::const_iterator it = str.cbegin();
        it != str.cend() && it - str.cbegin() < 254;
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

std::string AsciiToStr(const unsigned char *buf, const unsigned &len)
{
    return std::string(reinterpret_cast<const char *>(buf), len);
}

unsigned MD5StrtoUChar(std::string str, unsigned char *buf)
{
    if (!str.length())
        return 0;

    for (
        std::string::const_iterator it = str.cbegin();
        it != str.cend();
        it++, buf++
    ) {
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

bool SuCreateDirectory(const std::string &dirname)
{
    // "mkdir -p -m 666 $dirname"
    std::vector<std::string> pathnames;
    std::string tmp;

    if (mkdir(dirname.c_str(), 0666) != -1)
        return true;

    if (errno != ENOENT)
        return true;

    ParseString(dirname, '/', pathnames);

    for (const std::string &path : pathnames) {
        if (!path.empty())
            tmp.append("/");

        tmp.append(path);

        if (mkdir(tmp.c_str(), 0666) == -1)
            return true;
    }

    return true;
}

bool post_command(char c)
{
    for (int i = 0; i < 3; i++) {
        if (write(g_rwpipe[1], &c, 1) != -1)
            return true;

        perror("post_command write pipe");
    }

    return false;
}

std::string IntToString(int num)
{
    return std::to_string(num);
}

// hey, would you see rjsupplicant.sh?
// this implementation is the same as the C version
//function is64BIT()
//{
//  os=$(getconf LONG_BIT);
//  if [ $os == "32" ];  then
//      return 1;
//  fi
//  return 0;
//}
bool Is64BIT()
{
    // we'll use statically calculated value
    // maybe we'll support arm64 one day, so include it
#if defined(__x86_64__) || defined(__aarch64__)
    return true;
#elif defined(__i386__) || defined(__arm__)
    return false;
#else
#error Your platform is not supported
#endif
}

//void KillRunModeCheckTimer()
//{
//    if (g_runModetimer) {
//        my_timer_delete(g_runModetimer);
//        g_runModetimer = nullptr;
//    }
//}
//
//void *OnRunModeCheckTimer(union sigval arg)
//{
//
//}
//
//void SetRunModeCheckTimer()
//{
//    struct sigevent sev = {};
//    struct itimerspec new_time = { { 1, 0 }, { 1, 0 } };
//
//    if (g_runModetimer)
//        return;
//
//    sev.sigev_notify = SIGEV_THREAD;
//    sev.sigev_notify_function = &OnRunModeCheckTimer;
//    sev.sigev_value.sival_int = 1;
//
//    if (my_timer_create(CLOCK_REALTIME, &sev, &g_runModetimer) == -1) {
//        g_uilog.AppendText("SetRunModeCheckTimer my_timer_create error");
//        return;
//    }
//
//    if (my_timer_settime(g_runModetimer, CLOCK_REALTIME, &new_time, nullptr) == -1)
//        g_uilog.AppendText("SetRunModeCheckTimer my_timer_settime error");
//}

int MemCmpare(const void *buf1, int begin, int end, const void *buf2, int len)
{
    if (!buf1 || !buf2 || end - begin + 1 < len)
        return -2;

    return
        !!memcmp(
            reinterpret_cast<const void *>(
                reinterpret_cast<const char *>(buf1) + begin
            ),
            buf2, len
        );
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

void RcvFlowMonitorParam(void *arg)
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

void RcvNetSecParam(void *arg)
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

void decode(unsigned char *buf, int buflen)
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

void encode(unsigned char *buf, int buflen)
{
    return decode(buf, buflen);
}

std::string makeLower(const std::string &str)
{
    std::string ret;

    for (const unsigned char i : str)
        ret.push_back(tolower(i));

    return ret;
}

std::string makeUpper(const std::string &str)
{
    std::string ret;

    for (const unsigned char i : str)
        ret.push_back(toupper(i));

    return ret;
}
