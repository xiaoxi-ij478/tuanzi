#ifndef ENCRYPTION_H_INCLUDED
#define ENCRYPTION_H_INCLUDED

class CEncryption
{
    public:
        static int base64_decode(const unsigned char *src, unsigned char *dst);
        // the original implementation allocate the buffer in encode,
        // but pass allocated buffer in decode, why's that?
        // we allow passing allocated buffer only
//        static int base64_encode(const void *src, int len, char **dst);
        static int base64_encode(
            const unsigned char *src,
            unsigned len,
            unsigned char *dst
        );
//        static char *decrypt(unsigned char *buf);
//        static char *encrypt(const unsigned char *buf);
        static int decrypt(unsigned char *dst, const unsigned char *src);
        static int encrypt(unsigned char *dst, const unsigned char *src);
        static const char *key; // orig name: pKey
        static const char *enc_strs;
        static const char *base64_chars;
        static const char base64_cbytes[128];
};

#endif // ENCRYPTION_H_INCLUDED
