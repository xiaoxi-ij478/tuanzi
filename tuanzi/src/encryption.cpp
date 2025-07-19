#include "all.h"
#include "encryption.h"

[[maybe_unused]] const char *CEncryption::key = "abcdf12348";
const char *CEncryption::enc_strs =
    "~!:?$*<(qw2e5o7i8x12c6m67s98w43d2l45we82q3iuu1z4xle23rt4oxclle34e54u6r8m";
const char *CEncryption::base64_chars =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
const char CEncryption::base64_cbytes[128] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 62, 0, 0, 0, 63, 52,
    53, 54, 55, 56, 57, 58, 59, 60, 61, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 5,
    6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25,
    0, 0, 0, 0, 0, 0, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39,
    40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 0, 0, 0, 0, 0
};

int CEncryption::base64_decode(const char *src, char *dst)
{
    // this was written by myself
    unsigned srclen = strlen(src);
    unsigned written = 0;
    bool met_equal = false;
    char tmp[4] = {};

    // read 4 bytes from the base64 string one time
    for (unsigned i = 0; i < srclen / 4 && !met_equal; i++) {
        tmp[0] = *src++;
        tmp[1] = *src++;
        tmp[2] = *src++;
        tmp[3] = *src++;

        if (tmp[3] == '=')
            met_equal = true;

        *dst++ = base64_cbytes[tmp[0]] << 2 | base64_cbytes[tmp[1]] >> 4;
        *dst++ = (base64_cbytes[tmp[1]] & 0xf) << 4 | base64_cbytes[tmp[2]] >> 2;
        *dst++ = (base64_cbytes[tmp[2]] & 0x3) << 6 | base64_cbytes[tmp[3]];
        written += 3;
    }

//    if (srclen % 4) // data is truncated
//        return written;
    return written;
}

int CEncryption::base64_encode(const char *src, unsigned len, char *dst)
{
    // this was written by myself too
    unsigned written = 0;
    char tmp[3] = {};

    // process up to the 3-byte boundary
    for (unsigned i = 0; i < len / 3; i++) {
        tmp[0] = *src++;
        tmp[1] = *src++;
        tmp[2] = *src++;
        *dst++ = base64_chars[tmp[0] >> 2];
        *dst++ = base64_chars[(tmp[0] & 0x3) << 4 | tmp[1] >> 4];
        *dst++ = base64_chars[(tmp[1] & 0xf) << 2 | tmp[2] >> 6];
        *dst++ = base64_chars[tmp[2] & 0x3f];
        written += 4;
    }

    // process the remain data and fill with '='
    switch (len % 3) {
        case 0:
            break;

        case 1:
            tmp[0] = *src++;
            *dst++ = base64_chars[tmp[0] >> 2];
            *dst++ = base64_chars[(tmp[0] & 0x3) << 4];
            *dst++ = '=';
            *dst++ = '=';
            written += 4;
            break;

        case 2:
            tmp[0] = *src++;
            tmp[1] = *src++;
            *dst++ = base64_chars[tmp[0] >> 2];
            *dst++ = base64_chars[(tmp[0] & 0x3) << 4 | tmp[1] >> 4];
            *dst++ = base64_chars[(tmp[1] & 0xf) << 2];
            *dst++ = '=';
            written += 4;
            break;
    }

    return written;
}

int CEncryption::decrypt(char *dst, const char *src)
{
    int srclen = base64_decode(src, dst);

    for (int i = 0; i < srclen; i++)
        dst[i] ^= enc_strs[i % strlen(enc_strs)];

    return strlen(dst);
}

int CEncryption::encrypt(char *dst, const char *src)
{
    int srclen = strlen(src);
    char *dsrc = strdup(src);

    for (int i = 0; i < srclen; i++)
        dsrc[i] ^= enc_strs[i % strlen(enc_strs)];

    base64_encode(dsrc, srclen, dst);
    free(dsrc);
    dsrc = nullptr;
    return strlen(dst);
}
