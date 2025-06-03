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
extern void CoInitialize();
extern void CoUnInitialize(unsigned);
extern std::string DWordToString(unsigned a);
extern bool DecryptSuConfig(); // this is not working
extern bool EncryptSuConfig(); // this is not working
// it would not be used anyway...
extern void exec_cmd(const char *cmd, char *buf, int buflen);
extern float get_fedora_lib_version(const char *pkgname);
extern unsigned addStringOnLineHead(
    const char *in_filename,
    const char *out_filename,
    const char *add_line_contain,
    const char *add_string
);
extern int FindChar(char to_find, const char *str, int begin, int end);
extern int FindSub(
    const char *to_find_buf,
    unsigned to_find_buf_len,
    const char *buf,
    unsigned begin,
    unsigned end
);
extern int GKillTimer(timer_t timer);
extern void GOnTimer(union sigval);
extern void GSNRecvPacter(unsigned char *, int);
extern timer_t GSetTimer(
    int off_msec,
    void (*thread_function)(union sigval),
    struct TIMERPARAM *timer
);
extern void GetMD5File(const char *filename, char *result);
extern void ParseString(
    const std::string &str,
    char delim,
    std::vector<std::string> &dest
);
extern void TrimLeft(std::string &str, std::string chars);
extern void TrimRight(std::string &str, std::string chars);
extern void HIPacketUpdate(unsigned char *, int);
extern unsigned HexCharToAscii(
    const std::string &str,
    unsigned char *buf,
    unsigned buflen
);
extern std::string HexToString(const unsigned char *buf, int buflen);
extern int ASCIIStrtoChar(std::string str, unsigned char *buf);
extern std::string AsciiToStr(
    const unsigned char *buf,
    const unsigned &len
);
extern bool SuCreateDirectory(const std::string &dirname);
extern bool post_command(char c);
extern std::string IntToString(int num);
extern bool Is64BIT();
//extern void KillRunModeCheckTimer();
//extern void *OnRunModeCheckTimer(union sigval arg);
//extern void SetRunModeCheckTimer();
unsigned MD5StrtoUChar(std::string str, unsigned char *buf);
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
extern void RcvLoginURL(const std::string &arg);
extern void RcvNetSecParam(void *arg);
extern void RcvOpenUtrustUrlCmd(const std::string &arg);
extern void RcvStartAuthNotification();
extern void StrToLower(char *str);
extern bool convertInt(const char *str, int &result);
extern void decode(unsigned char *buf, int buflen);
extern void encode(unsigned char *buf, int buflen);
extern std::string makeLower(const std::string &str);
extern std::string makeUpper(const std::string &str);
extern void CopyDirTranPara(
    struct tagDirTranPara *dst,
    const struct tagDirTranPara *src
);

static inline void swap128(unsigned char *val)
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

#define free_list(head) \
    for (auto *h = head, *n = h->next; h; h = n, n = h ? nullptr : h->next) \
        delete h

#define free_list_with_func(head, func) \
    for (auto *h = head, *n = h->next; h; h = n, n = h ? nullptr : h->next) \
        func(h)

#define free_list_with_custom_next(head, next) \
    for (auto *h = head, *n = h->next; h; h = n, n = h ? nullptr : h->next) \
        delete h

#define free_list_with_func_custom_next(head, func, next) \
    for (auto *h = head, *n = h->next; h; h = n, n = h ? nullptr : h->next) \
        func(h)

#endif // UTIL_H_INCLUDED
