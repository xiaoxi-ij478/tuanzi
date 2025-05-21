#ifndef COMPRESSOR_H_INCLUDED
#define COMPRESSOR_H_INCLUDED

extern unsigned Decompress(
    const unsigned char *in,
    unsigned char *out,
    unsigned in_len,
    unsigned out_len // when this is 0 it is used to detect actual out len
);
extern unsigned Compress(
    const unsigned char *in,
    unsigned char *out,
    unsigned in_len,
    unsigned out_len // when this is 0 it is used to detect actual out len
);

#endif // COMPRESSOR_H_INCLUDED
