#ifndef UTIL_H_INCLUDED
#define UTIL_H_INCLUDED

#include "waithandle.h"
#include "changelanguage.h"
#include "global.h"

extern void setAppEnvironment();
extern int TakeAppPath(std::string &dst);
extern void get_exe_name(std::string &dst);
extern enum LANG GetSysLanguage();
extern void InitLogFiles();
extern void replace_all_distinct(
    std::string &str,
    const std::string &srcstr,
    const std::string &dststr
);
[[noreturn]] extern void chk_call_back(int);
extern bool set_msg_config(const std::string &key, int val);
extern void ChangeSelfSvrParam(void *);
[[maybe_unused]] extern void CoInitialize();
[[maybe_unused]] extern void CoUnInitialize(unsigned int);
[[maybe_unused]] extern std::string DWordToString(unsigned int a);
[[maybe_unused]] extern bool DecryptSuConfig(); // this is not working
[[maybe_unused]] extern bool EncryptSuConfig(); // this is not working
// it would not be used anyway...
[[maybe_unused]] extern void exec_cmd(const char *cmd, char *buf, int buflen);
[[maybe_unused]] extern float get_fedora_lib_version(
    [[maybe_unused]] const char *pkgname
);
extern unsigned int addStringOnLineHead(
    const char *in_filename,
    const char *out_filename,
    const char *add_line_contain,
    const char *add_string
);
extern int FindChar(char to_find, const char *str, int begin, int end);
extern int FindSub(
    const char *to_find_buf,
    unsigned int to_find_buf_len,
    const char *buf,
    unsigned int begin,
    unsigned int end
);
[[maybe_unused]] extern int GKillTimer(timer_t timer);
[[maybe_unused]] extern void GOnTimer(union sigval);
extern void GSNRecvPacter(unsigned char *, int);
extern timer_t GSetTimer(
    int off_msec,
    void (*thread_function)(union sigval),
    struct TIMERPARAM *timer
);
[[maybe_unused]] extern void GetMD5File(const char *filename, char *result);
extern void ParseString(
    const std::string &str,
    char delim,
    std::vector<std::string> &dest
);
[[maybe_unused]] extern void TrimLeft(std::string &str, std::string chars);
[[maybe_unused]] extern void TrimRight(std::string &str, std::string chars);
extern void HIPacketUpdate(unsigned char *, int);
extern unsigned int HexCharToAscii(
    const std::string &str,
    unsigned char *buf,
    unsigned int buflen
);
extern std::string HexToString(const unsigned char *buf, int buflen);
[[maybe_unused]] extern int ASCIIStrtoChar(std::string str, unsigned char *buf);
extern std::string AsciiToStr(
    const unsigned char *buf,
    const unsigned int &len
);
extern bool SuCreateDirectory(const std::string &dirname);
extern bool post_command(char c);
extern std::string IntToString(int num);
extern bool Is64BIT();
extern void KillRunModeCheckTimer();
extern void *OnRunModeCheckTimer(union sigval arg);
extern void SetRunModeCheckTimer();
extern int do_quit();
extern int MemCmpare(
    const void *buf1,
    int begin,
    int end,
    const void *buf2,
    int len
);
extern void RcvACLParam(void *arg);
extern void RcvCMD_GetProcessAndNetworkInfo();
extern void RcvFlowMonitorParam(void *arg);
extern void RcvIPMACChangeNotify();
extern void RcvLoginURL([[maybe_unused]] const std::string &arg);
extern void RcvNetSecParam(void *arg);
extern void RcvOpenUtrustUrlCmd(const std::string &arg);
extern void RcvStartAuthNotification();
[[maybe_unused]] extern void StrToLower(char *str);
extern bool convertInt(const char *str, int &result);
extern void decode(unsigned char *buf, int buflen);
extern void encode(unsigned char *buf, int buflen);
extern std::string makeLower(const std::string &str);
extern std::string makeUpper(const std::string &str);

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
