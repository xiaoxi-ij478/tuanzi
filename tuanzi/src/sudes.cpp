#include "sudes.h"

CSuDES::CSuDES() : ivbuf(), keybuf()
{}

CSuDES::~CSuDES()
{
    delete[] ivbuf;
    delete[] keybuf;
}

int CSuDES::Decrypts(unsigned char *buf, unsigned buflen) const
{
    unsigned char tmp1[8] = {}, tmp2[8] = {}, tmp3[8] = {};
    assert(buf && buflen);

    if (!ivbuf || !keybuf)
        return 2;

    if (buflen & 7)
        return 1;

    buflen >>= 3;

    if (!buflen)
        return 1;

    deskey(keybuf, DE1);
    memcpy(tmp1, ivbuf, sizeof(char) * 8);

    while (buflen--) {
        memcpy(tmp3, buf, sizeof(char) * 8);
        des(tmp3, tmp2);

        for (int i = 0; i < 8; i++)
            tmp2[i] ^= tmp1[i];

        memcpy(tmp1, buf, sizeof(char) * 8);
        memcpy(buf, tmp2, sizeof(char) * 8);
        buf += 8;
    }

    return 0;
}

int CSuDES::Encrypts(unsigned char *buf, unsigned buflen) const
{
    unsigned char tmp1[8] = {}, tmp2[8] = {};
    assert(buf && buflen);

    if (!ivbuf || !keybuf)
        return 2;

    if (buflen & 7)
        return 1;

    buflen >>= 3;

    if (!buflen)
        return 1;

    deskey(keybuf, EN0);
    memcpy(tmp1, ivbuf, sizeof(char) * 8);

    while (buflen--) {
        memcpy(tmp2, buf, sizeof(char) * 8);

        for (int i = 0; i < 8; i++)
            tmp2[i] ^= tmp1[i];

        des(tmp2, tmp1);
        memcpy(buf, tmp1, sizeof(char) * 8);
        buf += 8;
    }

    return 0;
}

int CSuDES::SetIVBuf(const unsigned char *iv, unsigned ivlen) const
{
    assert(iv);

    if (ivlen != 8)
        return 1;

    memcpy(ivbuf ? : new unsigned char[8], iv, sizeof(char) * 8);
    return 0;
}

int CSuDES::SetKeyBuf(const unsigned char *key, unsigned keylen) const
{
    assert(key);

    if (keylen != 8)
        return 1;

    memcpy(keybuf ? : new unsigned char[8], key, sizeof(char) * 8);
    return 0;
}
