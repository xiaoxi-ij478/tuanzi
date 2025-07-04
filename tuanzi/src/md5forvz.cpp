#include "all.h"
#include "md5forvz.h"

CMD5ForVz::CMD5ForVz() : ctx()
{
    MD5Init_Vz(&ctx);
}

CMD5ForVz::~CMD5ForVz()
{}

void CMD5ForVz::Update(const char *buf, unsigned buflen)
{
    MD5Update_Vz(&ctx, buf, buflen);
}

char *CMD5ForVz::Final()
{
    char digest[16] = {};
    char *digest_txt = new char[16 * 2 + 1];
    char upper = 0, lower = 0;
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

void CMD5ForVz::Final2CharBuff(char *buf, int buflen)
{
    if (buflen < 16) // buffer size is not enough
        return;

    MD5Final_Vz(buf, &ctx);
}

char *CMD5ForVz::GetMD5(const char *buf, unsigned buflen)
{
    CMD5ForVz cksum;
    cksum.Update(buf, buflen);
    return cksum.Final();
}

void CMD5ForVz::GetCharMd5(
    char *dst,
    const char *src,
    int srclen,
    int dstlen
)
{
    CMD5ForVz cksum;
    cksum.Update(src, srclen);
    cksum.Final2CharBuff(dst, dstlen);
}
