#ifndef COMPRESSOR_H_INCLUDED
#define COMPRESSOR_H_INCLUDED

unsigned int Decompress(
    const unsigned char *in,
    unsigned char *out,
    unsigned int in_len,
    unsigned int out_len // when this is 0 it is used to detect actual out len
);
unsigned int Compress(
    const unsigned char *in,
    unsigned char *out,
    unsigned int in_len,
    unsigned int out_len // when this is 0 it is used to detect actual out len
);

#endif // COMPRESSOR_H_INCLUDED
