#ifndef MD5FORVZ_H_INCLUDED
#define MD5FORVZ_H_INCLUDED

class CMD5ForVz
{
    public:
        CMD5ForVz();
        virtual ~CMD5ForVz();

        void Update(const char *buf, unsigned buflen);
        void Final2CharBuff(char *buf, unsigned buflen);
        char *Final();

        static char *GetMD5(const char *buf, unsigned buflen);
        static void GetCharMd5(
            char *dst,
            const char *src,
            unsigned srclen,
            unsigned dstlen
        );

    private:
        MD5_CTX ctx;
};

#endif // MD5FORVZ_H_INCLUDED
