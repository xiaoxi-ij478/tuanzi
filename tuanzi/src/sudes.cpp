#include "all.h"
#include "sudes.h"

CSuDES::CSuDES() : ivbuf(), keybuf()
{}

CSuDES::~CSuDES()
{
    delete[] ivbuf;
    delete[] keybuf;
}

int CSuDES::Decrypts(char *buf, unsigned buflen) const
{
    char tmp1[8] = {}, tmp2[8] = {}, tmp3[8] = {};
    assert(buf && buflen);

    if (!ivbuf || !keybuf)
        return 2;

    if (buflen & 7)
        return 1;

    buflen >>= 3;

    if (!buflen)
        return 1;

    deskey(keybuf, DE1);
    memcpy(tmp1, ivbuf, sizeof(tmp1));

    while (buflen--) {
        memcpy(tmp3, buf, sizeof(tmp3));
        des(tmp3, tmp2);

        for (int i = 0; i < 8; i++)
            tmp2[i] ^= tmp1[i];

        memcpy(tmp1, buf, sizeof(tmp1));
        memcpy(buf, tmp2, sizeof(tmp2));
        buf += 8;
    }

    return 0;
}

int CSuDES::Encrypts(char *buf, unsigned buflen) const
{
    char tmp1[8] = {}, tmp2[8] = {};
    assert(buf && buflen);

    if (!ivbuf || !keybuf)
        return 2;

    if (buflen & 7)
        return 1;

    buflen >>= 3;

    if (!buflen)
        return 1;

    deskey(keybuf, EN0);
    memcpy(tmp1, ivbuf, sizeof(tmp1));

    while (buflen--) {
        memcpy(tmp2, buf, sizeof(tmp2));

        for (int i = 0; i < 8; i++)
            tmp2[i] ^= tmp1[i];

        des(tmp2, tmp1);
        memcpy(buf, tmp1, sizeof(tmp1));
        buf += 8;
    }

    return 0;
}

int CSuDES::SetIVBuf(const char *iv, unsigned ivlen) const
{
    assert(iv);

    if (ivlen != 8)
        return 1;

    memcpy(ivbuf ? : new char[8], iv, sizeof(char) * 8);
    return 0;
}

int CSuDES::SetKeyBuf(const char *key, unsigned keylen) const
{
    assert(key);

    if (keylen != 8)
        return 1;

    memcpy(keybuf ? : new char[8], key, sizeof(char) * 8);
    return 0;
}
