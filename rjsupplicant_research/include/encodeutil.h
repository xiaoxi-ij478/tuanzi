#ifndef ENCODEUTIL_H_INCLUDED
#define ENCODEUTIL_H_INCLUDED

int code_convert(
    const char *fromcode, const char *tocode,
    const char *inbuf, size_t inbytesleft,
    char *outbuf, size_t outbytesleft
);
int g2u(
    const char *inbuf, size_t inbytesleft,
    char *outbuf, size_t outbytesleft
);
int u2g(
    const char *inbuf, size_t inbytesleft,
    char *outbuf, size_t outbytesleft
);
int ConvertGBKToUtf8(
    std::string &outbuf,
    const char *inbuf, int inbytesleft
);
int ConvertUtf8ToGBK(
    char *outbuf, int outbytesleft,
    const char *inbuf, int inbytesleft
);
int ConvertUtf8ToGBK(
    const char *inbuf, int inbytesleft,
    std::string &outbuf
);


#endif // ENCODEUTIL_H_INCLUDED
