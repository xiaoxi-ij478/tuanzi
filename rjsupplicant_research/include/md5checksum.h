#ifndef MD5CHECKSUM_H
#define MD5CHECKSUM_H

class CMD5Checksum
{
    public:
        CMD5Checksum();
        virtual ~CMD5Checksum();

        void Update(unsigned char *buf, unsigned int buflen);
        char *Final();
        void Final2CharBuff(unsigned char *buf, int buflen);
        static char *GetMD5(unsigned char *buf, unsigned int buflen);
        static void GetCharMd5(
            unsigned char *dst,
            unsigned char *src,
            int srclen,
            int dstlen
        );

    private:
        MD5_CTX ctx;
};

#endif // MD5CHECKSUM_H
