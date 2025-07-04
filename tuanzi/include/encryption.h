#ifndef ENCRYPTION_H_INCLUDED
#define ENCRYPTION_H_INCLUDED

class CEncryption
{
    public:
        static int base64_decode(const char *src, char *dst);
        // the original implementation allocate the buffer in encode,
        // but pass allocated buffer in decode, why's that?
        // we allow passing allocated buffer only
//        static int base64_encode(const void *src, int len, char **dst);
        static int base64_encode(
            const char *src,
            unsigned len,
            char *dst
        );
//        static char *decrypt(char *buf);
//        static char *encrypt(const char *buf);
        static int decrypt(char *dst, const char *src);
        static int encrypt(char *dst, const char *src);
        static const char *key; // orig name: pKey
        static const char *enc_strs;
        static const char *base64_chars;
        static const char base64_cbytes[128];
};

#endif // ENCRYPTION_H_INCLUDED
