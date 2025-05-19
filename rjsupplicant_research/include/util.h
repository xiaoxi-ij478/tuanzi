#ifndef UTIL_H_INCLUDED
#define UTIL_H_INCLUDED

#include "waithandle.h"
#include "changelanguage.h"
#include "global.h"

void setAppEnvironment();
int TakeAppPath(std::string &dst);
void get_exe_name(std::string &dst);
enum LANG GetSysLanguage();
void InitLogFiles();
void replace_all_distinct(
    std::string &str,
    const std::string &srcstr,
    const std::string &dststr
);
[[noreturn]] void chk_call_back(int);
bool set_msg_config(const std::string &key, int val);
void ChangeSelfSvrParam(void *);
[[maybe_unused]] void CoInitialize();
[[maybe_unused]] void CoUnInitialize(unsigned int);
[[maybe_unused]] std::string DWordToString(unsigned int a);
[[maybe_unused]] bool DecryptSuConfig(); // this is not working
[[maybe_unused]] bool EncryptSuConfig(); // this is not working
// it would not be used anyway...
[[maybe_unused]] void exec_cmd(const char *cmd, char *buf, int buflen);
[[maybe_unused]] float get_fedora_lib_version(
    [[maybe_unused]] const char *pkgname
);
unsigned int addStringOnLineHead(
    const char *in_filename,
    const char *out_filename,
    const char *add_line_contain,
    const char *add_string
);
int FindChar(char to_find, const char *str, int begin, int end);
int FindSub(
    const char *to_find_buf,
    unsigned int to_find_buf_len,
    const char *buf,
    unsigned int begin,
    unsigned int end
);
[[maybe_unused]] int GKillTimer(timer_t timer);
[[maybe_unused]] void GOnTimer(union sigval);
void GSNRecvPacter(unsigned char *, int);
timer_t GSetTimer(
    int off_msec,
    void (*thread_function)(union sigval),
    struct TIMERPARAM *timer
);
[[maybe_unused]] void GetMD5File(const char *filename, char *result);
void ParseString(
    const std::string &str,
    char delim,
    std::vector<std::string> &dest
);
[[maybe_unused]] void TrimLeft(std::string &str, std::string chars);
[[maybe_unused]] void TrimRight(std::string &str, std::string chars);
void HIPacketUpdate(unsigned char *, int);
unsigned int HexCharToAscii(
    const std::string &str,
    unsigned char *buf,
    unsigned int buflen
);
std::string HexToString(const unsigned char *buf, int buflen);
[[maybe_unused]] int ASCIIStrtoChar(std::string str, unsigned char *buf);
std::string AsciiToStr(const unsigned char *buf, const unsigned int &len);
bool SuCreateDirectory(const std::string &dirname);
bool post_command(char c);
std::string IntToString(int num);
bool Is64BIT();
//void KillRunModeCheckTimer();
//void *OnRunModeCheckTimer(union sigval arg);
//void SetRunModeCheckTimer();
int do_quit();
int MemCmpare(const void *buf1, int begin, int end, const void *buf2, int len);
void RcvACLParam(void *arg);
void RcvCMD_GetProcessAndNetworkInfo();
void RcvFlowMonitorParam(void *arg);
void RcvIPMACChangeNotify();
void RcvLoginURL([[maybe_unused]] const std::string &arg);
void RcvNetSecParam(void *arg);
void RcvOpenUtrustUrlCmd(const std::string &arg);
void RcvStartAuthNotification();
[[maybe_unused]] void StrToLower(char *str);
bool convertInt(const char *str, int &result);
void decode(unsigned char *buf, int buflen);
void encode(unsigned char *buf, int buflen);
std::string makeLower(const std::string &str);
std::string makeUpper(const std::string &str);

//inline void swap32(unsigned char *val)
//{
//    unsigned char t;
//#define SWAP(a, b) do { t = (a); (a) = (b); (b) = t; } while(0)
//    SWAP(val[0], val[3]);
//    SWAP(val[1], val[2]);
//#undef SWAP
//}
//
//inline void swap64(unsigned char *val)
//{
//    unsigned char t;
//#define SWAP(a, b) do { t = (a); (a) = (b); (b) = t; } while(0)
//    SWAP(val[0], val[7]);
//    SWAP(val[1], val[6]);
//    SWAP(val[2], val[5]);
//    SWAP(val[3], val[4]);
//#undef SWAP
//}

inline void swap128(unsigned char *val)
{
    unsigned char t;
#define SWAP(a, b) do { t = (a); (a) = (b); (b) = t; } while(0)
    SWAP(val[0], val[15]);
    SWAP(val[1], val[14]);
    SWAP(val[2], val[13]);
    SWAP(val[3], val[12]);
    SWAP(val[4], val[11]);
    SWAP(val[5], val[10]);
    SWAP(val[6], val[9]);
    SWAP(val[7], val[8]);
#undef SWAP
}

#endif // UTIL_H_INCLUDED
