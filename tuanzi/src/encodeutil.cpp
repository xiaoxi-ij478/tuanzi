#include "all.h"
#include "encodeutil.h"

int code_convert(
    const char *fromcode, const char *tocode,
    const char *inbuf, size_t inbytesleft,
    char *outbuf, size_t outbytesleft
)
{
    iconv_t cd = iconv_open(tocode, fromcode);

    if (cd == reinterpret_cast<iconv_t>(-1))
        return -1;

    if (iconv(
                cd,
                const_cast<char **>(&inbuf),
                &inbytesleft,
                &outbuf,
                &outbytesleft
            ) == static_cast<size_t>(-1))
        return -1;

    iconv_close(cd);
    return 0;
}

int g2u(
    const char *inbuf, size_t inbytesleft,
    char *outbuf, size_t outbytesleft
)
{
    return code_convert("gbk", "utf-8", inbuf, inbytesleft, outbuf, outbytesleft);
}

int u2g(
    const char *inbuf, size_t inbytesleft,
    char *outbuf, size_t outbytesleft
)
{
    return code_convert("utf-8", "gbk", inbuf, inbytesleft, outbuf, outbytesleft);
}

int ConvertGBKToUtf8(
    std::string &outbuf,
    const char *inbuf, int inbytesleft
)
{
    char *outb = new char[3 * inbytesleft];
    g2u(inbuf, inbytesleft, outb, 3 * inbytesleft);
    outbuf = outb;
//    outbuf.assign(outb, 3 * inbytesleft);
    delete[] outb;
    outb = nullptr;
    return outbuf.length();
}

int ConvertUtf8ToGBK(
    char *outbuf, int outbytesleft,
    const char *inbuf, int inbytesleft
)
{
    u2g(inbuf, inbytesleft, outbuf, outbytesleft);
    return outbytesleft;
}

int ConvertUtf8ToGBK(
    const char *inbuf, int inbytesleft,
    std::string &outbuf
)
{
    char *outb = new char[4 * inbytesleft];
    u2g(inbuf, inbytesleft, outb, 3 * inbytesleft);
    outbuf = outb;
//    outbuf.assign(outb, 4 * inbytesleft);
    delete[] outb;
    outb = nullptr;
    return outbuf.length();
}
