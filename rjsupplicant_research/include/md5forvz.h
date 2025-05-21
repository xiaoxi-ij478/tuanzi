#ifndef MD5FORVZ_H
#define MD5FORVZ_H

class CMD5ForVz
{
    public:
        CMD5ForVz();
        virtual ~CMD5ForVz();

        void Update(const unsigned char *buf, unsigned buflen);
        void Final2CharBuff(unsigned char *buf, int buflen);
        char *Final();

        static char *GetMD5(const unsigned char *buf, unsigned buflen);
        static void GetCharMd5(
            unsigned char *dst,
            const unsigned char *src,
            int srclen,
            int dstlen
        );

    private:
        MD5_CTX ctx;
};

#endif // MD5FORVZ_H
