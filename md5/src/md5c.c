/* MD5C.C - RSA Data Security, Inc., MD5 message-digest algorithm
 */

/* Copyright (C) 1991-2, RSA Data Security, Inc. Created 1991. All
rights reserved.

License to copy and use this software is granted provided that it
is identified as the "RSA Data Security, Inc. MD5 Message-Digest
Algorithm" in all material mentioning or referencing this software
or this function.

License is also granted to make and use derivative works provided
that such works are identified as "derived from the RSA Data
Security, Inc. MD5 Message-Digest Algorithm" in all material
mentioning or referencing the derived work.

RSA Data Security, Inc. makes no representations concerning either
the merchantability of this software or the suitability of this
software for any particular purpose. It is provided "as is"
without express or implied warranty of any kind.

These notices must be retained in any copies of any part of this
documentation and/or software.
 */

#include "mglobal.h"
#include "mmd5.h"

/* Constants for MD5Transform routine.
 */

#define S11 7
#define S12 12
#define S13 17
#define S14 22
#define S21 5
#define S22 9
#define S23 14
#define S24 20
#define S31 4
#define S32 11
#define S33 16
#define S34 23
#define S41 6
#define S42 10
#define S43 15
#define S44 21

#define S11_Vz 7
#define S12_Vz 12
#define S13_Vz 17
#define S14_Vz 22
#define S21_Vz 5
#define S22_Vz 9
#define S23_Vz 14
#define S24_Vz 20
#define S31_Vz 5
#define S32_Vz 11
#define S33_Vz 16
#define S34_Vz 23
#define S41_Vz 6
#define S42_Vz 10
#define S43_Vz 15
#define S44_Vz 19

static void MD5Transform PROTO_LIST ((UINT4 [4], unsigned char [64]));
static void MD5Transform_Vz PROTO_LIST ((UINT4 [4], unsigned char [64]));
static void Encode PROTO_LIST
  ((unsigned char *, UINT4 *, unsigned int));
static void Decode PROTO_LIST
  ((UINT4 *, unsigned char *, unsigned int));
static void MD5_memcpy PROTO_LIST ((POINTER, POINTER, unsigned int));
static void MD5_memset PROTO_LIST ((POINTER, int, unsigned int));

static unsigned char PADDING[64] = {
  0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

/* F, G, H and I are basic MD5 functions.
 */
#define F(x, y, z) (((x) & (y)) | ((~x) & (z)))
#define G(x, y, z) (((x) & (z)) | ((y) & (~z)))
#define H(x, y, z) ((x) ^ (y) ^ (z))
#define I(x, y, z) ((y) ^ ((x) | (~z)))

/* ROTATE_LEFT rotates x left n bits.
 */
#define ROTATE_LEFT(x, n) (((x) << (n)) | ((x) >> (32-(n))))

/* FF, GG, HH, and II transformations for rounds 1, 2, 3, and 4.
Rotation is separate from addition to prevent recomputation.
 */
#define FF(a, b, c, d, x, s, ac) { \
 (a) += F ((b), (c), (d)) + (x) + (UINT4)(ac); \
 (a) = ROTATE_LEFT ((a), (s)); \
 (a) += (b); \
  }
#define GG(a, b, c, d, x, s, ac) { \
 (a) += G ((b), (c), (d)) + (x) + (UINT4)(ac); \
 (a) = ROTATE_LEFT ((a), (s)); \
 (a) += (b); \
  }
#define HH(a, b, c, d, x, s, ac) { \
 (a) += H ((b), (c), (d)) + (x) + (UINT4)(ac); \
 (a) = ROTATE_LEFT ((a), (s)); \
 (a) += (b); \
  }
#define II(a, b, c, d, x, s, ac) { \
 (a) += I ((b), (c), (d)) + (x) + (UINT4)(ac); \
 (a) = ROTATE_LEFT ((a), (s)); \
 (a) += (b); \
  }
#define FF_Vz(a, b, c, d, x, s, ac) { \
 (a) += F ((b), (c), (d)) + (x) + (UINT4)(ac); \
 (a) = ROTATE_LEFT ((a), (s)); \
 (a) += (b) + 2; \
  }
#define GG_Vz(a, b, c, d, x, s, ac) { \
 (a) += G ((b), (c), (d)) + (x) + (UINT4)(ac); \
 (a) = ROTATE_LEFT ((a), (s)); \
 (a) += (b) - 2; \
  }
#define HH_Vz(a, b, c, d, x, s, ac) { \
 (a) += H ((b), (c), (d)) + (x) + (UINT4)(ac); \
 (a) = ROTATE_LEFT ((a), (s)); \
 (a) += (b) - 1; \
  }
#define II_Vz(a, b, c, d, x, s, ac) { \
 (a) += I ((b), (c), (d)) + (x) + (UINT4)(ac); \
 (a) = ROTATE_LEFT ((a), (s)); \
 (a) += (b) + 1; \
  }

/* MD5 initialization. Begins an MD5 operation, writing a new context.
 */
void MD5Init (context)
MD5_CTX *context;                                        /* context */
{
  context->count[0] = context->count[1] = 0;
  /* Load magic initialization constants.
*/
  context->state[0] = 0x67452301;
  context->state[1] = 0xefcdab89;
  context->state[2] = 0x98badcfe;
  context->state[3] = 0x10325476;
}

/* modified */
void MD5Init_Vz (context)
MD5_CTX *context;                                        /* context */
{
  context->count[0] = context->count[1] = 0;
  /* Load magic initialization constants.
*/
  context->state[0] = 0x50137246;
  context->state[1] = 0x8acf9dbe;
  context->state[2] = 0xc9efaced;
  context->state[3] = 0x25647013;
}

/* MD5 block update operation. Continues an MD5 message-digest
  operation, processing another message block, and updating the
  context.
 */
void MD5Update (context, input, inputLen)
MD5_CTX *context;                                        /* context */
const unsigned char *input;                          /* input block */
unsigned int inputLen;                     /* length of input block */
{
  unsigned int i, index, partLen;

  /* Compute number of bytes mod 64 */
  index = (unsigned int)((context->count[0] >> 3) & 0x3F);

  /* Update number of bits */
  if ((context->count[0] += ((UINT4)inputLen << 3))
   < ((UINT4)inputLen << 3))
 context->count[1]++;
  context->count[1] += ((UINT4)inputLen >> 29);

  partLen = 64 - index;

  /* Transform as many times as possible.
*/
  if (inputLen >= partLen) {
 MD5_memcpy
   ((POINTER)&context->buffer[index], (POINTER)input, partLen);
 MD5Transform (context->state, context->buffer);

 for (i = partLen; i + 63 < inputLen; i += 64)
   MD5Transform (context->state, &input[i]);

 index = 0;
  }
  else
 i = 0;

  /* Buffer remaining input */
  MD5_memcpy
 ((POINTER)&context->buffer[index], (POINTER)&input[i],
  inputLen-i);
}

/* modified */
void MD5Update_Vz (context, input, inputLen)
MD5_CTX *context;                                        /* context */
const unsigned char *input;                                /* input block */
unsigned int inputLen;                     /* length of input block */
{
  unsigned int i, index, partLen;

  /* Compute number of bytes mod 64 */
  index = (unsigned int)((context->count[0] >> 3) & 0x3F);

  /* Update number of bits */
  if ((context->count[0] += ((UINT4)inputLen << 3))
   < ((UINT4)inputLen << 3))
 context->count[1]++;
  context->count[1] += ((UINT4)inputLen >> 29);

  partLen = 64 - index;

  /* Transform as many times as possible.
*/
  if (inputLen >= partLen) {
 MD5_memcpy
   ((POINTER)&context->buffer[index], (POINTER)input, partLen);
 MD5Transform_Vz (context->state, context->buffer);

 for (i = partLen; i + 63 < inputLen; i += 64)
   MD5Transform_Vz (context->state, &input[i]);

 index = 0;
  }
  else
 i = 0;

  /* Buffer remaining input */
  MD5_memcpy
 ((POINTER)&context->buffer[index], (POINTER)&input[i],
  inputLen-i);
}

/* MD5 finalization. Ends an MD5 message-digest operation, writing the
  the message digest and zeroizing the context.
 */
void MD5Final (digest, context)
unsigned char digest[16];                         /* message digest */
MD5_CTX *context;                                       /* context */
{
  unsigned char bits[8];
  unsigned int index, padLen;

  /* Save number of bits */
  Encode (bits, context->count, 8);

  /* Pad out to 56 mod 64.
*/
  index = (unsigned int)((context->count[0] >> 3) & 0x3f);
  padLen = (index < 56) ? (56 - index) : (120 - index);
  MD5Update (context, PADDING, padLen);

  /* Append length (before padding) */
  MD5Update (context, bits, 8);

  /* Store state in digest */
  Encode (digest, context->state, 16);

  /* Zeroize sensitive information.
*/
  MD5_memset ((POINTER)context, 0, sizeof (*context));
}

/* modified */
void MD5Final_Vz (digest, context)
unsigned char digest[16];                         /* message digest */
MD5_CTX *context;                                       /* context */
{
  unsigned char bits[8];
  unsigned int index, padLen;

  /* Save number of bits */
  Encode (bits, context->count, 8);

  /* Pad out to 56 mod 64.
*/
  index = (unsigned int)((context->count[0] >> 3) & 0x3f);
  padLen = (index < 56) ? (56 - index) : (120 - index);
  MD5Update_Vz (context, PADDING, padLen);

  /* Append length (before padding) */
  MD5Update_Vz (context, bits, 8);

  /* Store state in digest */
  Encode (digest, context->state, 16);

  /* Zeroize sensitive information.
*/
  MD5_memset ((POINTER)context, 0, sizeof (*context));
}

/* MD5 basic transformation. Transforms state based on block.
 */
static void MD5Transform (state, block)
UINT4 state[4];
unsigned char block[64];
{
  UINT4 a = state[0], b = state[1], c = state[2], d = state[3], x[16];

  Decode (x, block, 64);

  /* Round 1 */
  FF (a, b, c, d, x[ 0], S11, 0xd76aa478); /* 1 */
  FF (d, a, b, c, x[ 1], S12, 0xe8c7b756); /* 2 */
  FF (c, d, a, b, x[ 2], S13, 0x242070db); /* 3 */
  FF (b, c, d, a, x[ 3], S14, 0xc1bdceee); /* 4 */
  FF (a, b, c, d, x[ 4], S11, 0xf57c0faf); /* 5 */
  FF (d, a, b, c, x[ 5], S12, 0x4787c62a); /* 6 */
  FF (c, d, a, b, x[ 6], S13, 0xa8304613); /* 7 */
  FF (b, c, d, a, x[ 7], S14, 0xfd469501); /* 8 */
  FF (a, b, c, d, x[ 8], S11, 0x698098d8); /* 9 */
  FF (d, a, b, c, x[ 9], S12, 0x8b44f7af); /* 10 */
  FF (c, d, a, b, x[10], S13, 0xffff5bb1); /* 11 */
  FF (b, c, d, a, x[11], S14, 0x895cd7be); /* 12 */
  FF (a, b, c, d, x[12], S11, 0x6b901122); /* 13 */
  FF (d, a, b, c, x[13], S12, 0xfd987193); /* 14 */
  FF (c, d, a, b, x[14], S13, 0xa679438e); /* 15 */
  FF (b, c, d, a, x[15], S14, 0x49b40821); /* 16 */

 /* Round 2 */
  GG (a, b, c, d, x[ 1], S21, 0xf61e2562); /* 17 */
  GG (d, a, b, c, x[ 6], S22, 0xc040b340); /* 18 */
  GG (c, d, a, b, x[11], S23, 0x265e5a51); /* 19 */
  GG (b, c, d, a, x[ 0], S24, 0xe9b6c7aa); /* 20 */
  GG (a, b, c, d, x[ 5], S21, 0xd62f105d); /* 21 */
  GG (d, a, b, c, x[10], S22,  0x2441453); /* 22 */
  GG (c, d, a, b, x[15], S23, 0xd8a1e681); /* 23 */
  GG (b, c, d, a, x[ 4], S24, 0xe7d3fbc8); /* 24 */
  GG (a, b, c, d, x[ 9], S21, 0x21e1cde6); /* 25 */
  GG (d, a, b, c, x[14], S22, 0xc33707d6); /* 26 */
  GG (c, d, a, b, x[ 3], S23, 0xf4d50d87); /* 27 */
  GG (b, c, d, a, x[ 8], S24, 0x455a14ed); /* 28 */
  GG (a, b, c, d, x[13], S21, 0xa9e3e905); /* 29 */
  GG (d, a, b, c, x[ 2], S22, 0xfcefa3f8); /* 30 */
  GG (c, d, a, b, x[ 7], S23, 0x676f02d9); /* 31 */
  GG (b, c, d, a, x[12], S24, 0x8d2a4c8a); /* 32 */

  /* Round 3 */
  HH (a, b, c, d, x[ 5], S31, 0xfffa3942); /* 33 */
  HH (d, a, b, c, x[ 8], S32, 0x8771f681); /* 34 */
  HH (c, d, a, b, x[11], S33, 0x6d9d6122); /* 35 */
  HH (b, c, d, a, x[14], S34, 0xfde5380c); /* 36 */
  HH (a, b, c, d, x[ 1], S31, 0xa4beea44); /* 37 */
  HH (d, a, b, c, x[ 4], S32, 0x4bdecfa9); /* 38 */
  HH (c, d, a, b, x[ 7], S33, 0xf6bb4b60); /* 39 */
  HH (b, c, d, a, x[10], S34, 0xbebfbc70); /* 40 */
  HH (a, b, c, d, x[13], S31, 0x289b7ec6); /* 41 */
  HH (d, a, b, c, x[ 0], S32, 0xeaa127fa); /* 42 */
  HH (c, d, a, b, x[ 3], S33, 0xd4ef3085); /* 43 */
  HH (b, c, d, a, x[ 6], S34,  0x4881d05); /* 44 */
  HH (a, b, c, d, x[ 9], S31, 0xd9d4d039); /* 45 */
  HH (d, a, b, c, x[12], S32, 0xe6db99e5); /* 46 */
  HH (c, d, a, b, x[15], S33, 0x1fa27cf8); /* 47 */
  HH (b, c, d, a, x[ 2], S34, 0xc4ac5665); /* 48 */

  /* Round 4 */
  II (a, b, c, d, x[ 0], S41, 0xf4292244); /* 49 */
  II (d, a, b, c, x[ 7], S42, 0x432aff97); /* 50 */
  II (c, d, a, b, x[14], S43, 0xab9423a7); /* 51 */
  II (b, c, d, a, x[ 5], S44, 0xfc93a039); /* 52 */
  II (a, b, c, d, x[12], S41, 0x655b59c3); /* 53 */
  II (d, a, b, c, x[ 3], S42, 0x8f0ccc92); /* 54 */
  II (c, d, a, b, x[10], S43, 0xffeff47d); /* 55 */
  II (b, c, d, a, x[ 1], S44, 0x85845dd1); /* 56 */
  II (a, b, c, d, x[ 8], S41, 0x6fa87e4f); /* 57 */
  II (d, a, b, c, x[15], S42, 0xfe2ce6e0); /* 58 */
  II (c, d, a, b, x[ 6], S43, 0xa3014314); /* 59 */
  II (b, c, d, a, x[13], S44, 0x4e0811a1); /* 60 */
  II (a, b, c, d, x[ 4], S41, 0xf7537e82); /* 61 */
  II (d, a, b, c, x[11], S42, 0xbd3af235); /* 62 */
  II (c, d, a, b, x[ 2], S43, 0x2ad7d2bb); /* 63 */
  II (b, c, d, a, x[ 9], S44, 0xeb86d391); /* 64 */

  state[0] += a;
  state[1] += b;
  state[2] += c;
  state[3] += d;

  /* Zeroize sensitive information.
*/
  MD5_memset ((POINTER)x, 0, sizeof (x));
}

static void MD5Transform_Vz (state, block)
UINT4 state[4];
unsigned char block[64];
{
  UINT4 a = state[0], b = state[1], c = state[2], d = state[3], x[16];

  Decode (x, block, 64);

  /* Round 1 */
  FF_Vz (a, b, c, d, x[ 0], S11_Vz, 0xd76aa478); /* 1 */
  FF_Vz (d, a, b, c, x[ 1], S12_Vz, 0xe8c7b756); /* 2 */
  FF_Vz (c, d, a, b, x[ 2], S13_Vz, 0x242070db); /* 3 */
  FF_Vz (b, c, d, a, x[ 3], S14_Vz, 0xc1bdceee); /* 4 */
  FF_Vz (a, b, c, d, x[ 4], S11_Vz, 0xf57c0faf); /* 5 */
  FF_Vz (d, a, b, c, x[ 5], S12_Vz, 0x4787c62a); /* 6 */
  FF_Vz (c, d, a, b, x[ 6], S13_Vz, 0xa8304613); /* 7 */
  FF_Vz (b, c, d, a, x[ 7], S14_Vz, 0xfd469501); /* 8 */
  FF_Vz (a, b, c, d, x[ 8], S11_Vz, 0x698098d8); /* 9 */
  FF_Vz (d, a, b, c, x[ 9], S12_Vz, 0x8b44f7af); /* 10 */
  FF_Vz (c, d, a, b, x[10], S13_Vz, 0xffff5bb1); /* 11 */
  FF_Vz (b, c, d, a, x[11], S14_Vz, 0x895cd7be); /* 12 */
  FF_Vz (a, b, c, d, x[12], S11_Vz, 0x6b901122); /* 13 */
  FF_Vz (d, a, b, c, x[13], S12_Vz, 0xfd987163); /* 14 */ /* ac */
  FF_Vz (c, d, a, b, x[14], S13_Vz, 0xa679438e); /* 15 */
  FF_Vz (b, c, d, a, x[15], S14_Vz, 0x49b40821); /* 16 */

 /* Round 2 */
  GG_Vz (a, b, c, d, x[ 1], S21_Vz, 0xf61e2562); /* 17 */
  GG_Vz (d, a, b, c, x[ 6], S22_Vz, 0xc040b340); /* 18 */
  GG_Vz (c, d, a, b, x[11], S23_Vz, 0x265e5a51); /* 19 */
  GG_Vz (b, c, d, a, x[ 0], S24_Vz, 0xe9b6c7aa); /* 20 */
  GG_Vz (a, b, c, d, x[ 5], S21_Vz, 0xd62f105d); /* 21 */
  GG_Vz (d, a, b, c, x[10], S22_Vz,  0x2442453); /* 22 */ /* ac */
  GG_Vz (c, d, a, b, x[15], S23_Vz, 0xd8a1e681); /* 23 */
  GG_Vz (b, c, d, a, x[ 4], S24_Vz, 0xe7d3fbc8); /* 24 */
  GG_Vz (a, b, c, d, x[ 9], S21_Vz, 0x21e1cde6); /* 25 */
  GG_Vz (d, a, b, c, x[14], S22_Vz, 0xc33707d6); /* 26 */
  GG_Vz (c, d, a, b, x[ 3], S23_Vz, 0xf4d50d87); /* 27 */
  GG_Vz (b, c, d, a, x[ 8], S24_Vz, 0x455a14ed); /* 28 */
  GG_Vz (a, b, c, d, x[13], S21_Vz, 0xa9e3e905); /* 29 */
  GG_Vz (d, a, b, c, x[ 2], S22_Vz, 0xfcefa3f8); /* 30 */
  GG_Vz (c, d, a, b, x[ 7], S23_Vz, 0x676f02d9); /* 31 */
  GG_Vz (b, c, d, a, x[12], S24_Vz, 0x8d2a4c8a); /* 32 */

  /* Round 3 */
  HH_Vz (a, b, c, d, x[ 5], S31_Vz, 0xfffa3492); /* 33 */ /* ac */
  HH_Vz (d, a, b, c, x[ 8], S32_Vz, 0x8771f681); /* 34 */
  HH_Vz (c, d, a, b, x[11], S33_Vz, 0x6d9d6122); /* 35 */
  HH_Vz (b, c, d, a, x[14], S34_Vz, 0xfde5380c); /* 36 */
  HH_Vz (a, b, c, d, x[ 1], S31_Vz, 0xa4beea44); /* 37 */
  HH_Vz (d, a, b, c, x[ 4], S32_Vz, 0x4bdecfa9); /* 38 */
  HH_Vz (c, d, a, b, x[ 7], S33_Vz, 0xf6bb4b60); /* 39 */
  HH_Vz (b, c, d, a, x[10], S34_Vz, 0xbebfbc70); /* 40 */
  HH_Vz (a, b, c, d, x[13], S31_Vz, 0x289b7ec6); /* 41 */
  HH_Vz (d, a, b, c, x[ 0], S32_Vz, 0xeaa127fa); /* 42 */
  HH_Vz (c, d, a, b, x[ 3], S33_Vz, 0xd4ef3085); /* 43 */
  HH_Vz (b, c, d, a, x[ 6], S34_Vz,  0x4881d05 + block[9]); /* 44 */ /* add data's 10th byte */
  HH_Vz (a, b, c, d, x[ 9], S31_Vz, 0xd9d4d039); /* 45 */
  HH_Vz (d, a, b, c, x[12], S32_Vz, 0xe6db99e5); /* 46 */
  HH_Vz (c, d, a, b, x[15], S33_Vz, 0x1fa27cf8); /* 47 */
  HH_Vz (b, c, d, a, x[ 2], S34_Vz, 0xc4ac5665); /* 48 */

  /* Round 4 */
  II_Vz (a, b, c, d, x[ 0], S41_Vz, 0xf4292244); /* 49 */
  II_Vz (d, a, b, c, x[ 7], S42_Vz, 0x432aff97); /* 50 */
  II_Vz (c, d, a, b, x[14], S43_Vz, 0xab9423a7); /* 51 */
  II_Vz (b, c, d, a, x[ 5], S44_Vz, 0xfc93a039); /* 52 */
  II_Vz (a, b, c, d, x[12], S41_Vz, 0x655659c3); /* 53 */ /* ac */
  II_Vz (d, a, b, c, x[ 3], S42_Vz, 0x8f0ccc92); /* 54 */
  II_Vz (c, d, a, b, x[10], S43_Vz, 0xffeff47d); /* 55 */
  II_Vz (b, c, d, a, x[ 1], S44_Vz, 0x85845dd1); /* 56 */
  II_Vz (a, b, c, d, x[ 8], S41_Vz, 0x6fa87e4f); /* 57 */
  II_Vz (d, a, b, c, x[15], S42_Vz, 0xfe2ce6e0); /* 58 */
  II_Vz (c, d, a, b, x[ 6], S43_Vz, 0xa3014314); /* 59 */
  II_Vz (b, c, d, a, x[13], S44_Vz, 0x4e0811a1); /* 60 */
  II_Vz (a, b, c, d, x[ 4], S41_Vz, 0xf7537e82); /* 61 */
  II_Vz (d, a, b, c, x[11], S42_Vz, 0xbd3af335); /* 62 */ /* ac */
  II_Vz (c, d, a, b, x[ 2], S43_Vz, 0x2ad7d2bb); /* 63 */
  II_Vz (b, c, d, a, x[ 9], S44_Vz, 0xeb866391); /* 64 */ /* ac */

  state[0] += a;
  state[1] += b;
  state[2] += c;
  state[3] += d;

  /* Zeroize sensitive information.
*/
  MD5_memset ((POINTER)x, 0, sizeof (x));
}

/* Encodes input (UINT4) into output (unsigned char). Assumes len is
  a multiple of 4.
 */
static void Encode (output, input, len)
unsigned char *output;
UINT4 *input;
unsigned int len;
{
  unsigned int i, j;

  for (i = 0, j = 0; j < len; i++, j += 4) {
 output[j] = (unsigned char)(input[i] & 0xff);
 output[j+1] = (unsigned char)((input[i] >> 8) & 0xff);
 output[j+2] = (unsigned char)((input[i] >> 16) & 0xff);
 output[j+3] = (unsigned char)((input[i] >> 24) & 0xff);
  }
}

/* Decodes input (unsigned char) into output (UINT4). Assumes len is
  a multiple of 4.
 */
static void Decode (output, input, len)
UINT4 *output;
unsigned char *input;
unsigned int len;
{
  unsigned int i, j;

  for (i = 0, j = 0; j < len; i++, j += 4)
 output[i] = ((UINT4)input[j]) | (((UINT4)input[j+1]) << 8) |
   (((UINT4)input[j+2]) << 16) | (((UINT4)input[j+3]) << 24);
}

/* Note: Replace "for loop" with standard memcpy if possible.
 */

static void MD5_memcpy (output, input, len)
POINTER output;
POINTER input;
unsigned int len;
{
  unsigned int i;

  for (i = 0; i < len; i++)
 output[i] = input[i];
}

/* Note: Replace "for loop" with standard memset if possible.
 */
static void MD5_memset (output, value, len)
POINTER output;
int value;
unsigned int len;
{
  unsigned int i;

  for (i = 0; i < len; i++)
 ((char *)output)[i] = (char)value;
}
