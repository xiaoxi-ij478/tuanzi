#ifndef UTIL_H_INCLUDED
#define UTIL_H_INCLUDED

extern int TakeAppPath(std::string &dst);
extern void replace_all_distinct(
    std::string &str,
    const std::string &srcstr,
    const std::string &dststr
);
[[noreturn]] extern void chk_call_back();
extern unsigned addStringOnLineHead(
    const char *in_filename,
    const char *out_filename,
    const char *add_line_contain,
    const char *add_string
);
extern int FindChar(char to_find, const char *str, int begin, int end);
extern void ParseString(
    const std::string &str,
    char delim,
    std::vector<std::string> &dest
);
extern void ParseString(
    const std::string &str,
    char delim,
    std::vector<std::string> &dest,
    unsigned max_time
);
extern void TrimLeft(std::string &str, const std::string &chars);
extern void TrimRight(std::string &str, const std::string &chars);
extern unsigned HexCharToAscii(
    const std::string &str,
    char *buf,
    unsigned buflen
);
extern std::string HexToString(const char *buf, int buflen);
extern int ASCIIStrtoChar(const std::string &str, char *buf);
extern std::string AsciiToStr(
    const char *buf,
    unsigned len
);
extern unsigned MD5StrtoUChar(const std::string &str, char *buf);
extern void decode(char *buf, unsigned buflen);
extern int StringToHex(
    const std::string &str,
    char *retbuf,
    unsigned retbuflen
);
extern void WriteRegUserInfo(
    const struct UserInfo &info
#ifdef BUILDING_UPDATER
    , const std::string &filename
#endif // BUILDING_UPDATER
);
extern void ReadRegUserInfo(
    struct UserInfo &info
#ifdef BUILDING_UPDATER
    , const std::string &filename
#endif // BUILDING_UPDATER
);
extern void GetSuInternalVersion(unsigned &major, unsigned &minor);
extern void RadiusEncrpytPwd(
    const char *md5_challenge,
    unsigned md5_challenge_len,
    const char *password,
    unsigned password_len,
    char *outbuf
);
extern void RcvSvrList(const std::vector<std::string> &service_list);
extern void RcvSvrSwitchResult(const std::string &notify);

static inline std::string DWordToString(unsigned a)
{
    return std::to_string(a);
}

static inline void TrimAll(std::string &str, const std::string &chars)
{
    TrimLeft(str, chars);
    TrimRight(str, chars);
}

static inline void GSNRecvPacket(char *, int)
{
    logFile.AppendText("receive gsn relative command!");
}

static inline char GetHIRusultByLocal()
{
    return 0;
}

static inline std::string makeLower(const std::string &str)
{
    std::string ret;
    std::transform(str.cbegin(), str.cend(), std::back_inserter(ret), tolower);
    return ret;
}

static inline std::string makeUpper(const std::string &str)
{
    std::string ret;
    std::transform(str.cbegin(), str.cend(), std::back_inserter(ret), toupper);
    return ret;
}

static inline int MemCmpare(
    const char *buf1,
    int begin,
    int end,
    const char *buf2,
    int len
)
{
    if (!buf1 || !buf2 || end - begin + 1 < len)
        return -2;

    return memcmp(buf1 + begin, buf2, len);
}

static inline void RcvFlowMonitorParam(const void *arg)
{
    logFile.AppendText("recv flow monitor param");
    assert(arg);
}

static inline void RcvNetSecParam(const void *arg)
{
    logFile.AppendText("recv network security param");
    assert(arg);
}

static inline void decode(char *buf, unsigned buflen)
{
    // *INDENT-OFF*
    std::transform(
        buf,
        buf + buflen,
        buf,
        [](char i) {
            return
                ((~i & 0x10) >> 1) |
                ((~i & 0x20) >> 3) |
                ((~i & 0x40) >> 5) |
                ((~i & 0x80) >> 7) |
                ((~i & 0x08) << 1) |
                ((~i & 0x04) << 3) |
                ((~i & 0x02) << 5) |
                ((~i & 0x01) << 7);
        }
    );
    // *INDENT-ON*
}

static inline void encode(char *buf, unsigned buflen)
{
    return decode(buf, buflen);
}

static inline void HIPacketUpdate(const char *, int)
{
    logFile.AppendText("receive hi packet update command!");
}

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

#define free_list_with_custom_next(head, next) \
    free_list_with_func_custom_next(head, delete, next)

#define free_list_with_func(head, func) \
    free_list_with_func_custom_next(head, func, next)

#define free_list(head) \
    free_list_with_func_custom_next(head, delete, next)

#define UNUSED_VAR(name) (void)name

#endif // UTIL_H_INCLUDED
