#ifndef SUDES_H
#define SUDES_H

class CSuDES
{
    public:
        CSuDES();
        virtual ~CSuDES();
        int Decrypts(unsigned char *buf, unsigned buflen) const;
        int Encrypts(unsigned char *buf, unsigned buflen) const;
        int SetIVBuf(const unsigned char *iv, unsigned ivlen) const;
        int SetKeyBuf(const unsigned char *key, unsigned keylen) const;

    private:
        unsigned char *ivbuf;
        unsigned char *keybuf;
};

#endif // SUDES_H
