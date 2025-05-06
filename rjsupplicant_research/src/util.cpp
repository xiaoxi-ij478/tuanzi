#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cassert>
#include <cerrno>
#include <ctime>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cstdarg>
#include <csignal>
#include <unistd.h>
#include <sys/sysinfo.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/msg.h>

#include "mmd5.h"
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
    char s[1024] = { 0 };
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


void replace_all_distinct(std::string &str, const std::string &srcstr,
                          const std::string &dststr)
{
    size_t special_pos = 0;

    while ((special_pos = str.find(srcstr)) != std::string::npos)
        str.replace(special_pos, srcstr.length(), dststr);
}

[[noreturn]] void chk_call_back(int)
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

[[maybe_unused]] void CoInitialize() {}

[[maybe_unused]] void CoUnInitialize(unsigned int) {}

[[maybe_unused]] std::string DWordToString(unsigned int a)
{
    std::ostringstream oss;
    oss << a;
    return oss.str();
}

[[maybe_unused]] bool DecryptSuConfig()
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

[[maybe_unused]] bool EncryptSuConfig()
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
        g_logSystem.AppendText("ERROR: Open file %s failed.\n", "SuConfig_Encrypt.dat");
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
        g_logSystem.AppendText("ERROR: write file %s failed.\n",
                               "SuConfig_Encrypt.dat");
        return false;
    }

    return true;
}

[[maybe_unused]] void exec_cmd(const char *cmd, char *buf, int buflen)
{
    FILE *fp = popen(cmd, "r");
    unsigned int read_len = 0;

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

[[maybe_unused]] float get_fedora_lib_version(const char *pkgname)
{
    // I'm a Debian fan, so I don't know how to use yum
    // yum list installed |grep $pkgname |awk 'NR==1 {print $2}'
    // printf "%s version=%s\n" "get_fedora_lib_version" $version
    return 0.0;
}

unsigned int addStringOnLineHead(
    const char *in_filename,
    const char *out_filename,
    const char *add_line_contain,
    const char *add_string
)
{
    std::ifstream ifs(in_filename);
    std::ofstream ofs(out_filename, std::ios::trunc);
    std::string line;
    unsigned int modified_line = 0;

    if (!ifs || !ofs) {
        g_log_Wireless.AppendText("addStringOnLineHead open file failed.");
        return 0;
    }

    while (std::getline(ifs, line)) {
        if (line.find(add_line_contain) != std::string::npos) {
            line.insert(0, line);
            modified_line++;
        }

        ofs << line << std::endl;
    }

    ifs.close();
    ofs.close();
    return modified_line;
}

int FindChar(
    char to_find,
    const char *str,
    int begin,
    int end
)
{
    if (!str || begin < 0 || end < 0 || end > strlen(str))
        return -1;

    for (int i = begin; i < end; i++)
        if (str[i] == to_find)
            return str[i];

    return -1;
}

int FindSub(
    const unsigned char *buf,
    unsigned int buflen,
    const unsigned char *to_find_buf,
    unsigned int to_find_buf_len,
    unsigned int begin,
    unsigned int end
)
{
    bool match = true;

    if (
        !buf ||
        !to_find_buf ||
        end > buflen ||
        begin + to_find_buf_len > buflen
    )
        return -1;

    for (unsigned int i = begin; i < end; i++) {
        for (unsigned int j = 0; j < to_find_buf_len; j++)
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

[[maybe_unused]] int GKillTimer(timer_t timer)
{
    int ret = 0;

    if ((ret = my_timer_delete(timer)) != -1)
        return ret;

    perror("timer_delete ");
    return -1;
}

[[maybe_unused]] void GOnTimer(union sigval) {}

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
    struct sigevent sev = { 0 };
    struct itimerspec newtime = { 0 };
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
    newtime.it_interval.tv_sec = newtime.it_value.tv_sec = off_msec / 1000;
    newtime.it_value.tv_nsec = newtime.it_interval.tv_nsec = off_msec % 1000;
    return my_timer_settime(timerid, TIMER_ABSTIME, &newtime, nullptr) < 0
           ? 0
           : timerid;
}

[[maybe_unused]] void GetMD5File(const char *filename, char *result)
{
    std::ifstream ifs(filename);
    unsigned char digest[16] = { 0 };
    unsigned char buf[0x200] = { 0 };
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

    for (unsigned int i = 0; i < 16; i++) {
        *result++ = (digest[i] >> 4) + '0';
        *result++ = (digest[i] & 0xf) + '0';
    }
}

enum OS_TYPE get_os_type()
{
    // the original implementation uses /etc/issue
    // cat /etc/issue |awk 'NR==1 {print $1}'
    // we use /etc/os-release
    // I only use Debian, so code may be inaccurate
    // If you find any errors, notice me
    std::ifstream ifs("/etc/os-release");
    std::vector<std::string> val;
    std::string line;

    if (!ifs)
        return OS_INVALID;

    while (std::getline(ifs, line)) {
        split(val, line, '=');

        if (val[0] == "ID")
            break;
    }

    ifs.close();

    if (val[1] == "debian" || val[1] == "ubuntu")
        return OS_UBUNTU;

    if (val[1] == "fedora")
        return OS_FEDORA;

    if (val[1] == "centos")
        return OS_CENTOS;

    return OS_INVALID;
}

void split(std::vector<std::string> &dest, const std::string &str, char delim)
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

[[maybe_unused]] void TrimLeft(std::string &str, std::string chars)
{
    if (str.find_first_not_of(chars) == std::string::npos)
        str.clear();

    str.erase(0, str.find_first_not_of(chars) - 1);
}

[[maybe_unused]] void TrimRight(std::string &str, std::string chars)
{
    if (str.find_last_not_of(chars) == std::string::npos)
        str.clear();

    str.erase(str.find_last_not_of(chars) + 1);
}

void HIPacketUpdate(unsigned char *, int)
{
    logFile.AppendText("receive hi packet update command!");
}

unsigned int HexCharToAscii(
    const std::string &str,
    unsigned char *buf,
    unsigned int buflen
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

    return str.length() >> 1;
}

std::string HexToString(const unsigned char *buf, int buflen)
{
    std::string ret;
    unsigned char upper = 0, lower = 0;

    for (; buflen; buflen--, buf++) {
        upper = *buf >> 4;
        lower = *buf & 0xF;

        if (/* upper >= 0 && */upper <= 9)
            ret.push_back(upper + '0');

        else if (upper >= 10 && upper <= 15)
            ret.push_back(upper - 10 + 'A');

        if (/* lower >= 0 && */lower <= 9)
            ret.push_back(lower + '0');

        else if (lower >= 10 && lower <= 15)
            ret.push_back(lower - 10 + 'A');
    }

    return ret;
}
