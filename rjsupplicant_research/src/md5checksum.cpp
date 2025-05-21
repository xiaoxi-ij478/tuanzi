#include "md5checksum.h"

CMD5Checksum::CMD5Checksum() : ctx()
{
    MD5Init(&ctx);
}

CMD5Checksum::~CMD5Checksum()
{}

void CMD5Checksum::Update(unsigned char *buf, unsigned buflen)
{
    MD5Update(&ctx, buf, buflen);
}

char *CMD5Checksum::Final()
{
    unsigned char digest[16] = {};
    char *digest_txt = new char[33];
    unsigned char upper = 0, lower = 0;
    Final2CharBuff(digest, 16);

    for (int i = 0; i < 16; i++) {
        upper = digest[i] >> 4;
        lower = digest[i] & 0xf;

        if (upper <= 9)
            digest_txt[i << 1] = upper + '0';

        else if (upper >= 10 && upper <= 15)
            digest_txt[i << 1] = upper - 10 + 'a';

        if (lower <= 9)
            digest_txt[(i << 1) + 1] = lower + '0';

        else if (lower >= 10 && lower <= 15)
            digest_txt[(i << 1) + 1] = lower - 10 + 'a';
    }

    return digest_txt;
}

void CMD5Checksum::Final2CharBuff(unsigned char *buf, int buflen)
{
    if (buflen < 16) // buffer size is not enough
        return;

    MD5Final(buf, &ctx);
}

char *CMD5Checksum::GetMD5(unsigned char *buf, unsigned buflen)
{
    CMD5Checksum cksum;
    cksum.Update(buf, buflen);
    return cksum.Final();
}

void CMD5Checksum::GetCharMd5(
    unsigned char *dst,
    unsigned char *src,
    int srclen,
    int dstlen
)
{
    CMD5Checksum cksum;
    cksum.Update(src, srclen);
    cksum.Final2CharBuff(dst, dstlen);
}
