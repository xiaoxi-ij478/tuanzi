#ifndef MD5CHECKSUM_H_INCLUDED
#define MD5CHECKSUM_H_INCLUDED

class CMD5Checksum
{
    public:
        CMD5Checksum();
        virtual ~CMD5Checksum();

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

#endif // MD5CHECKSUM_H_INCLUDED
