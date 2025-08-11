#ifndef UTIL_H_INCLUDED
#define UTIL_H_INCLUDED

extern void setAppEnvironment();
extern int TakeAppPath(std::string& dst);
extern void InitLogFiles();
extern void replace_all_distinct(
    std::string& str,
    const std::string& srcstr,
    const std::string& dststr
);
[[noreturn]] extern void chk_call_back(int);
extern bool set_msg_config(const std::string& key, int val);
extern void ChangeSelfSvrParam(void *);
extern void CoInitialize();
extern void CoUnInitialize();
extern std::string DWordToString(unsigned a);
extern bool DecryptSuConfig(); // this is not working
extern bool EncryptSuConfig(); // this is not working
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
extern void GSNRecvPacket(char *, int);
extern timer_t GSetTimer(
    int off_msec,
    void (*thread_function)(union sigval),
    struct TIMERPARAM *timer
);
extern void GetMD5File(const char *filename, char *result);
extern void ParseString(
    const std::string& str,
    char delim,
    std::vector<std::string>& dest
);
extern void ParseString(
    const std::string& str,
    char delim,
    std::vector<std::string>& dest,
    unsigned max_time
);
extern void TrimLeft(std::string& str, const std::string& chars);
extern void TrimRight(std::string& str, const std::string& chars);
extern void HIPacketUpdate(const char *, int);
extern unsigned HexCharToAscii(
    const std::string& str,
    char *buf,
    unsigned buflen
);
extern std::string HexToString(const char *buf, int buflen);
extern int ASCIIStrtoChar(const std::string& str, char *buf);
extern std::string AsciiToStr(
    const char *buf,
    unsigned len
);
extern std::string IntToString(int num);
extern unsigned MD5StrtoUChar(const std::string& str, char *buf);
extern int MemCmpare(
    const char *buf1,
    int begin,
    int end,
    const char *buf2,
    int len
);
extern void RcvACLParam(const void *arg);
extern void RcvCMD_GetProcessAndNetworkInfo();
extern void RcvFlowMonitorParam(const void *arg);
extern void RcvIPMACChangeNotify();
extern void RcvLoginURL(const std::string& arg);
extern void RcvNetSecParam(const void *arg);
extern void RcvOpenUtrustUrlCmd(const std::string& arg);
extern void RcvStartAuthNotification();
extern void StrToLower(char *str);
extern bool convertInt(const char *str, int& result);
extern void decode(char *buf, unsigned buflen);
extern void encode(char *buf, unsigned buflen);
extern std::string makeLower(const std::string& str);
extern std::string makeUpper(const std::string& str);
extern int StringToHex(
    const std::string& str,
    char *retbuf,
    unsigned retbuflen
);
extern void WriteRegUserInfo(
    const struct UserInfo& info
#ifdef BUILDING_UPDATER
    , const std::string& filename
#endif // BUILDING_UPDATER
);
extern void ReadRegUserInfo(
    struct UserInfo& info
#ifdef BUILDING_UPDATER
    , const std::string& filename
#endif // BUILDING_UPDATER
);
extern void SimulateSuLogoff(char *buf, unsigned buflen);
extern bool SetLanFlag(unsigned flag);
extern void RecvSecdomainPacket(char *buf, unsigned buflen);
extern void CopyGradeInfo(
    struct SPUpGradeInfo& dst,
    const struct SPUpGradeInfo& src
);
extern void GetSuInternalVersion(unsigned& major, unsigned& minor);
extern void RadiusEncrpytPwd(
    const char *md5_challenge,
    unsigned md5_challenge_len,
    const char *password,
    unsigned password_len,
    char *outbuf
);
extern char GetHIRusultByLocal();
extern void RcvSvrList(const std::vector<std::string>& service_list);
extern bool IsUpgrade(unsigned ver);
extern bool GetHIResult(
    const std::vector<struct HIFailInfo>& a1,
    unsigned long a2,
    unsigned a3
);
extern void RcvSvrSwitchResult(const std::string& notify);
extern void RcvModifyPasswordResult(bool change_success, const char *fail_msg);
extern void DoWithServiceSwitch_NoRuijieNas();
extern void DoWithServiceSwitch_RuijieNas();
extern void InitAppMain();
extern void ServiceSwitch(const std::string& service_name);
extern void TrimAll(std::string& str, const std::string& chars);

static inline void swap128(char *val)
{
    char t;
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

#define free_list_with_func_custom_next(head, func, next) \
    for (auto *h = (head), *n = h->next; h; h = n, n = h ? h->next : nullptr) \
        func(h)

#define __free_list_delete_operator(o) delete (o)

#define free_list_with_custom_next(head, next) \
    free_list_with_func_custom_next(head, __free_list_delete_operator, next)

#define free_list_with_func(head, func) \
    free_list_with_func_custom_next(head, func, next)

#define free_list(head) \
    free_list_with_func_custom_next(head, __free_list_delete_operator, next)

#define UNUSED_VAR(name) (void)name

#endif // UTIL_H_INCLUDED
