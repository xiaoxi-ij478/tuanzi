/*
	Copyright (C) 2009  Gabriel A. Petursson

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "ampheck.h"
#include "ripemd128.h"

#define RIPEMD128_R1(x, y, z)  (x ^ y ^ z)
#define RIPEMD128_R2(x, y, z)  (((x & y) | (~x & z)) + 0x5a827999)
#define RIPEMD128_R3(x, y, z)  (((x | ~y) ^ z)       + 0x6ed9eba1)
#define RIPEMD128_R4(x, y, z)  (((x & z) | (y & ~z)) + 0x8f1bbcdc)
#define RIPEMD128_R5(x, y, z)  (((x & z) | (y & ~z)) + 0x50a28be6)
#define RIPEMD128_R6(x, y, z)  (((x | ~y) ^ z)       + 0x5c4dd124)
#define RIPEMD128_R7(x, y, z)  (((x & y) | (~x & z)) + 0x6d703ef3)
#define RIPEMD128_R8 RIPEMD128_R1

#define RIPEMD128_PRC(a, b, c, d, idx, rot, rnd) { \
	wv[a] = ROL(wv[a] + RIPEMD128_R##rnd(wv[b], wv[c], wv[d]) + idx, rot); \
}

/* the operate const changes, so we should not define it here */
#define RIPEMD128_R1_Vz(x, y, z, co)  ((x ^ y ^ z)          + co)
#define RIPEMD128_R2_Vz(x, y, z, co)  (((x & y) | (~x & z)) + co)
#define RIPEMD128_R3_Vz(x, y, z, co)  (((x | ~y) ^ z)       + co)
#define RIPEMD128_R4_Vz(x, y, z, co)  (((x & z) | (y & ~z)) + co)
#define RIPEMD128_R5_Vz(x, y, z, co)  (((x & z) | (y & ~z)) + co)
#define RIPEMD128_R6_Vz(x, y, z, co)  (((x | ~y) ^ z)       + co)
#define RIPEMD128_R7_Vz(x, y, z, co)  (((x & y) | (~x & z)) + co)
#define RIPEMD128_R8_Vz RIPEMD128_R1_Vz

#define RIPEMD128_PRC_Vz(a, b, c, d, co, idx, rot, rnd) { \
	wv[a] = ROL(wv[a] + RIPEMD128_R##rnd##_Vz(wv[b], wv[c], wv[d], co) + idx, rot); \
}

void ampheck_ripemd128_init(struct ampheck_ripemd128 *ctx)
{
	ctx->h[0] = 0x67452301;
	ctx->h[1] = 0xefcdab89;
	ctx->h[2] = 0x98badcfe;
	ctx->h[3] = 0x10325476;

	ctx->length = 0;
}

/* modified */
void ampheck_ripemd128_init_Vz(struct ampheck_ripemd128 *ctx)
{
	ctx->h[0] = 0x10257436;
	ctx->h[1] = 0xa8bd9cfe;
	ctx->h[2] = 0x9efcad8b;
	ctx->h[3] = 0x12375460;

	ctx->length = 0;
}

void ampheck_ripemd128_transform(struct ampheck_ripemd128 *ctx, const uint8_t *data, size_t blocks)
{
	for (size_t i = 0; i < blocks; ++i)
	{
		uint32_t wv[8];
		uint32_t w[16];

		PACK_32_LE(&data[(i << 6)     ], &w[ 0]);
		PACK_32_LE(&data[(i << 6) +  4], &w[ 1]);
		PACK_32_LE(&data[(i << 6) +  8], &w[ 2]);
		PACK_32_LE(&data[(i << 6) + 12], &w[ 3]);
		PACK_32_LE(&data[(i << 6) + 16], &w[ 4]);
		PACK_32_LE(&data[(i << 6) + 20], &w[ 5]);
		PACK_32_LE(&data[(i << 6) + 24], &w[ 6]);
		PACK_32_LE(&data[(i << 6) + 28], &w[ 7]);
		PACK_32_LE(&data[(i << 6) + 32], &w[ 8]);
		PACK_32_LE(&data[(i << 6) + 36], &w[ 9]);
		PACK_32_LE(&data[(i << 6) + 40], &w[10]);
		PACK_32_LE(&data[(i << 6) + 44], &w[11]);
		PACK_32_LE(&data[(i << 6) + 48], &w[12]);
		PACK_32_LE(&data[(i << 6) + 52], &w[13]);
		PACK_32_LE(&data[(i << 6) + 56], &w[14]);
		PACK_32_LE(&data[(i << 6) + 60], &w[15]);

		wv[0] = ctx->h[0];
		wv[1] = ctx->h[1];
		wv[2] = ctx->h[2];
		wv[3] = ctx->h[3];
		memcpy(&wv[4], wv, 4 * sizeof(uint32_t));

		RIPEMD128_PRC(0, 1, 2, 3, w[ 0], 11, 1);
		RIPEMD128_PRC(3, 0, 1, 2, w[ 1], 14, 1);
		RIPEMD128_PRC(2, 3, 0, 1, w[ 2], 15, 1);
		RIPEMD128_PRC(1, 2, 3, 0, w[ 3], 12, 1);
		RIPEMD128_PRC(0, 1, 2, 3, w[ 4],  5, 1);
		RIPEMD128_PRC(3, 0, 1, 2, w[ 5],  8, 1);
		RIPEMD128_PRC(2, 3, 0, 1, w[ 6],  7, 1);
		RIPEMD128_PRC(1, 2, 3, 0, w[ 7],  9, 1);
		RIPEMD128_PRC(0, 1, 2, 3, w[ 8], 11, 1);
		RIPEMD128_PRC(3, 0, 1, 2, w[ 9], 13, 1);
		RIPEMD128_PRC(2, 3, 0, 1, w[10], 14, 1);
		RIPEMD128_PRC(1, 2, 3, 0, w[11], 15, 1);
		RIPEMD128_PRC(0, 1, 2, 3, w[12],  6, 1);
		RIPEMD128_PRC(3, 0, 1, 2, w[13],  7, 1);
		RIPEMD128_PRC(2, 3, 0, 1, w[14],  9, 1);
		RIPEMD128_PRC(1, 2, 3, 0, w[15],  8, 1);

		RIPEMD128_PRC(0, 1, 2, 3, w[ 7],  7, 2);
		RIPEMD128_PRC(3, 0, 1, 2, w[ 4],  6, 2);
		RIPEMD128_PRC(2, 3, 0, 1, w[13],  8, 2);
		RIPEMD128_PRC(1, 2, 3, 0, w[ 1], 13, 2);
		RIPEMD128_PRC(0, 1, 2, 3, w[10], 11, 2);
		RIPEMD128_PRC(3, 0, 1, 2, w[ 6],  9, 2);
		RIPEMD128_PRC(2, 3, 0, 1, w[15],  7, 2);
		RIPEMD128_PRC(1, 2, 3, 0, w[ 3], 15, 2);
		RIPEMD128_PRC(0, 1, 2, 3, w[12],  7, 2);
		RIPEMD128_PRC(3, 0, 1, 2, w[ 0], 12, 2);
		RIPEMD128_PRC(2, 3, 0, 1, w[ 9], 15, 2);
		RIPEMD128_PRC(1, 2, 3, 0, w[ 5],  9, 2);
		RIPEMD128_PRC(0, 1, 2, 3, w[ 2], 11, 2);
		RIPEMD128_PRC(3, 0, 1, 2, w[14],  7, 2);
		RIPEMD128_PRC(2, 3, 0, 1, w[11], 13, 2);
		RIPEMD128_PRC(1, 2, 3, 0, w[ 8], 12, 2);

		RIPEMD128_PRC(0, 1, 2, 3, w[ 3], 11, 3);
		RIPEMD128_PRC(3, 0, 1, 2, w[10], 13, 3);
		RIPEMD128_PRC(2, 3, 0, 1, w[14],  6, 3);
		RIPEMD128_PRC(1, 2, 3, 0, w[ 4],  7, 3);
		RIPEMD128_PRC(0, 1, 2, 3, w[ 9], 14, 3);
		RIPEMD128_PRC(3, 0, 1, 2, w[15],  9, 3);
		RIPEMD128_PRC(2, 3, 0, 1, w[ 8], 13, 3);
		RIPEMD128_PRC(1, 2, 3, 0, w[ 1], 15, 3);
		RIPEMD128_PRC(0, 1, 2, 3, w[ 2], 14, 3);
		RIPEMD128_PRC(3, 0, 1, 2, w[ 7],  8, 3);
		RIPEMD128_PRC(2, 3, 0, 1, w[ 0], 13, 3);
		RIPEMD128_PRC(1, 2, 3, 0, w[ 6],  6, 3);
		RIPEMD128_PRC(0, 1, 2, 3, w[13],  5, 3);
		RIPEMD128_PRC(3, 0, 1, 2, w[11], 12, 3);
		RIPEMD128_PRC(2, 3, 0, 1, w[ 5],  7, 3);
		RIPEMD128_PRC(1, 2, 3, 0, w[12],  5, 3);

		RIPEMD128_PRC(0, 1, 2, 3, w[ 1], 11, 4);
		RIPEMD128_PRC(3, 0, 1, 2, w[ 9], 12, 4);
		RIPEMD128_PRC(2, 3, 0, 1, w[11], 14, 4);
		RIPEMD128_PRC(1, 2, 3, 0, w[10], 15, 4);
		RIPEMD128_PRC(0, 1, 2, 3, w[ 0], 14, 4);
		RIPEMD128_PRC(3, 0, 1, 2, w[ 8], 15, 4);
		RIPEMD128_PRC(2, 3, 0, 1, w[12],  9, 4);
		RIPEMD128_PRC(1, 2, 3, 0, w[ 4],  8, 4);
		RIPEMD128_PRC(0, 1, 2, 3, w[13],  9, 4);
		RIPEMD128_PRC(3, 0, 1, 2, w[ 3], 14, 4);
		RIPEMD128_PRC(2, 3, 0, 1, w[ 7],  5, 4);
		RIPEMD128_PRC(1, 2, 3, 0, w[15],  6, 4);
		RIPEMD128_PRC(0, 1, 2, 3, w[14],  8, 4);
		RIPEMD128_PRC(3, 0, 1, 2, w[ 5],  6, 4);
		RIPEMD128_PRC(2, 3, 0, 1, w[ 6],  5, 4);
		RIPEMD128_PRC(1, 2, 3, 0, w[ 2], 12, 4);

		RIPEMD128_PRC(4, 5, 6, 7, w[ 5],  8, 5);
		RIPEMD128_PRC(7, 4, 5, 6, w[14],  9, 5);
		RIPEMD128_PRC(6, 7, 4, 5, w[ 7],  9, 5);
		RIPEMD128_PRC(5, 6, 7, 4, w[ 0], 11, 5);
		RIPEMD128_PRC(4, 5, 6, 7, w[ 9], 13, 5);
		RIPEMD128_PRC(7, 4, 5, 6, w[ 2], 15, 5);
		RIPEMD128_PRC(6, 7, 4, 5, w[11], 15, 5);
		RIPEMD128_PRC(5, 6, 7, 4, w[ 4],  5, 5);
		RIPEMD128_PRC(4, 5, 6, 7, w[13],  7, 5);
		RIPEMD128_PRC(7, 4, 5, 6, w[ 6],  7, 5);
		RIPEMD128_PRC(6, 7, 4, 5, w[15],  8, 5);
		RIPEMD128_PRC(5, 6, 7, 4, w[ 8], 11, 5);
		RIPEMD128_PRC(4, 5, 6, 7, w[ 1], 14, 5);
		RIPEMD128_PRC(7, 4, 5, 6, w[10], 14, 5);
		RIPEMD128_PRC(6, 7, 4, 5, w[ 3], 12, 5);
		RIPEMD128_PRC(5, 6, 7, 4, w[12],  6, 5);

		RIPEMD128_PRC(4, 5, 6, 7, w[ 6],  9, 6);
		RIPEMD128_PRC(7, 4, 5, 6, w[11], 13, 6);
		RIPEMD128_PRC(6, 7, 4, 5, w[ 3], 15, 6);
		RIPEMD128_PRC(5, 6, 7, 4, w[ 7],  7, 6);
		RIPEMD128_PRC(4, 5, 6, 7, w[ 0], 12, 6);
		RIPEMD128_PRC(7, 4, 5, 6, w[13],  8, 6);
		RIPEMD128_PRC(6, 7, 4, 5, w[ 5],  9, 6);
		RIPEMD128_PRC(5, 6, 7, 4, w[10], 11, 6);
		RIPEMD128_PRC(4, 5, 6, 7, w[14],  7, 6);
		RIPEMD128_PRC(7, 4, 5, 6, w[15],  7, 6);
		RIPEMD128_PRC(6, 7, 4, 5, w[ 8], 12, 6);
		RIPEMD128_PRC(5, 6, 7, 4, w[12],  7, 6);
		RIPEMD128_PRC(4, 5, 6, 7, w[ 4],  6, 6);
		RIPEMD128_PRC(7, 4, 5, 6, w[ 9], 15, 6);
		RIPEMD128_PRC(6, 7, 4, 5, w[ 1], 13, 6);
		RIPEMD128_PRC(5, 6, 7, 4, w[ 2], 11, 6);

		RIPEMD128_PRC(4, 5, 6, 7, w[15],  9, 7);
		RIPEMD128_PRC(7, 4, 5, 6, w[ 5],  7, 7);
		RIPEMD128_PRC(6, 7, 4, 5, w[ 1], 15, 7);
		RIPEMD128_PRC(5, 6, 7, 4, w[ 3], 11, 7);
		RIPEMD128_PRC(4, 5, 6, 7, w[ 7],  8, 7);
		RIPEMD128_PRC(7, 4, 5, 6, w[14],  6, 7);
		RIPEMD128_PRC(6, 7, 4, 5, w[ 6],  6, 7);
		RIPEMD128_PRC(5, 6, 7, 4, w[ 9], 14, 7);
		RIPEMD128_PRC(4, 5, 6, 7, w[11], 12, 7);
		RIPEMD128_PRC(7, 4, 5, 6, w[ 8], 13, 7);
		RIPEMD128_PRC(6, 7, 4, 5, w[12],  5, 7);
		RIPEMD128_PRC(5, 6, 7, 4, w[ 2], 14, 7);
		RIPEMD128_PRC(4, 5, 6, 7, w[10], 13, 7);
		RIPEMD128_PRC(7, 4, 5, 6, w[ 0], 13, 7);
		RIPEMD128_PRC(6, 7, 4, 5, w[ 4],  7, 7);
		RIPEMD128_PRC(5, 6, 7, 4, w[13],  5, 7);

		RIPEMD128_PRC(4, 5, 6, 7, w[ 8], 15, 8);
		RIPEMD128_PRC(7, 4, 5, 6, w[ 6],  5, 8);
		RIPEMD128_PRC(6, 7, 4, 5, w[ 4],  8, 8);
		RIPEMD128_PRC(5, 6, 7, 4, w[ 1], 11, 8);
		RIPEMD128_PRC(4, 5, 6, 7, w[ 3], 14, 8);
		RIPEMD128_PRC(7, 4, 5, 6, w[11], 14, 8);
		RIPEMD128_PRC(6, 7, 4, 5, w[15],  6, 8);
		RIPEMD128_PRC(5, 6, 7, 4, w[ 0], 14, 8);
		RIPEMD128_PRC(4, 5, 6, 7, w[ 5],  6, 8);
		RIPEMD128_PRC(7, 4, 5, 6, w[12],  9, 8);
		RIPEMD128_PRC(6, 7, 4, 5, w[ 2], 12, 8);
		RIPEMD128_PRC(5, 6, 7, 4, w[13],  9, 8);
		RIPEMD128_PRC(4, 5, 6, 7, w[ 9], 12, 8);
		RIPEMD128_PRC(7, 4, 5, 6, w[ 7],  5, 8);
		RIPEMD128_PRC(6, 7, 4, 5, w[10], 15, 8);
		RIPEMD128_PRC(5, 6, 7, 4, w[14],  8, 8);

		wv[7] += wv[2] + ctx->h[1];
		ctx->h[1] = ctx->h[2] + wv[3] + wv[4];
		ctx->h[2] = ctx->h[3] + wv[0] + wv[5];
		ctx->h[3] = ctx->h[0] + wv[1] + wv[6];
		ctx->h[0] = wv[7];
	}
}

/*__int64 __fastcall RipeMD128::CoreTransform(RipeMD128 *this)
{
  int wv[0]; // esi
  int wv[3]; // r8d
  int wv[2]; // r10d
  int wv[1]; // r9d
  int wv[0]; // esi
  int wv[3]; // r8d
  int wv[2]; // r10d
  int wv[1]; // r9d
  int wv[0]; // esi
  int wv[3]; // r8d
  int wv[2]; // r10d
  int wv[1]; // r9d
  int wv[0]; // esi
  int wv[3]; // r8d
  int wv[2]; // r10d
  int wv[1]; // r9d
  int wv[0]; // esi
  int wv[3]; // r8d
  int wv[2]; // r10d
  int wv[1]; // r9d
  int wv[0]; // esi
  int wv[3]; // r8d
  int wv[2]; // r10d
  int wv[1]; // r9d
  int wv[0]; // esi
  int wv[3]; // r8d
  int wv[2]; // r10d
  int wv[1]; // r9d
  int wv[0]; // esi
  int wv[3]; // r8d
  int wv[2]; // r10d
  int wv[1]; // r9d
  int wv[0]; // esi
  int wv[3]; // r8d
  int wv[2]; // r10d
  int wv[1]; // r9d
  int wv[0]; // esi
  int wv[3]; // r8d
  int wv[2]; // r10d
  int wv[1]; // r9d
  int wv[0]; // esi
  int wv[3]; // r8d
  int wv[2]; // r10d
  int wv[1]; // r9d
  int wv[0]; // esi
  int wv[3]; // r8d
  int wv[2]; // r10d
  int wv[1]; // r9d
  int wv[0]; // esi
  int wv[3]; // r8d
  int wv[2]; // r10d
  int wv[1]; // r9d
  int wv[0]; // esi
  int wv[3]; // r8d
  int wv[2]; // r10d
  int wv[1]; // r9d
  int wv[0]; // esi
  int wv[3]; // r8d
  int wv[2]; // r10d
  int wv[1]; // r9d
  int wv[0]; // esi
  int wv[3]; // r8d
  int wv[2]; // ebx
  int wv[1]; // r10d
  int wv[7]; // r11d
  int wv[6]; // ecx
  int wv[5]; // edx
  int wv[4]; // ebx
  int wv[7]; // r11d
  int wv[6]; // ecx
  int wv[5]; // edx
  int wv[4]; // ebx
  int wv[7]; // r11d
  int wv[6]; // ecx
  int wv[5]; // edx
  int wv[4]; // ebx
  int wv[7]; // r11d
  int wv[6]; // ecx
  int wv[5]; // edx
  int wv[4]; // ebx
  int wv[7]; // r11d
  int wv[6]; // ecx
  int wv[5]; // edx
  int wv[4]; // ebx
  int wv[7]; // r11d
  int wv[6]; // ecx
  int wv[5]; // edx
  int wv[4]; // ebx
  int wv[7]; // r11d
  int wv[6]; // r12d
  int wv[5]; // edx
  int wv[4]; // ebx
  int wv[7]; // r11d
  int wv[6]; // r12d
  int wv[5]; // edx
  int wv[4]; // ecx
  int wv[7]; // r11d
  int wv[6]; // r12d
  int wv[5]; // edx
  int wv[4]; // ecx
  int wv[7]; // ebx
  int wv[6]; // r12d
  int wv[5]; // r11d
  int wv[4]; // edx
  int wv[7]; // ecx
  int wv[6]; // r14d
  int wv[5]; // r11d
  int wv[4]; // edx
  int wv[7]; // r13d
  int wv[6]; // r14d
  int wv[5]; // r11d
  int wv[4]; // ecx
  int wv[7]; // r13d
  int wv[6]; // r14d
  int wv[5]; // r15d
  int wv[4]; // ecx
  int wv[7]; // r13d
  int wv[6]; // r14d
  int wv[5]; // r15d
  int wv[4]; // ecx
  int wv[7]; // r13d
  int v131; // r9d
  int wv[6]; // r11d
  int wv[5]; // r15d
  int wv[4]; // ecx
  int wv[7]; // r13d
  int wv[6]; // r11d
  int wv[5]; // r15d
  uint32_t v138; // esi
  uint32_t v139; // r11d
  __int64 result; // rax
  unsigned int v141[18]; // [rsp+0h] [rbp-78h]


    wv[0] = this->h[0];
    wv[1] = this->h[1];
    wv[2] = this->h[2];
    wv[3] = this->h[3];
  wv[0] = __ROR4__((wv[3] ^ wv[1] ^ wv[2]) + wv[0] + *(_DWORD *)this->buffer      + 2, 21);
  wv[3] = __ROR4__((wv[1] ^ wv[2] ^ wv[0]) + wv[3] + *(_DWORD *)&this->buffer[4]  + 2, 18);
  wv[2] = __ROR4__((wv[3] ^ wv[0] ^ wv[1]) + wv[2] + *(_DWORD *)&this->buffer[8]  + 2, 17);
  wv[1] = __ROR4__((wv[2] ^ wv[0] ^ wv[3]) + wv[1] + *(_DWORD *)&this->buffer[12] + 2, 20);
  wv[0] = __ROR4__((wv[1] ^ wv[3] ^ wv[2]) + wv[0] + *(_DWORD *)&this->buffer[16] + 2, 27);
  wv[3] = __ROR4__((wv[0] ^ wv[2] ^ wv[1]) + wv[3] + *(_DWORD *)&this->buffer[20] + 2, 24);
  wv[2] = __ROR4__((wv[3] ^ wv[1] ^ wv[0]) + wv[2] + *(_DWORD *)&this->buffer[24] + 2, 25);
  wv[1] = __ROR4__((wv[2] ^ wv[0] ^ wv[3]) + wv[1] + *(_DWORD *)&this->buffer[28] + 2, 23);
  wv[0] = __ROR4__((wv[1] ^ wv[3] ^ wv[2]) + wv[0] + *(_DWORD *)&this->buffer[32] + 2, 21);
  wv[3] = __ROR4__((wv[0] ^ wv[2] ^ wv[1]) + wv[3] + *(_DWORD *)&this->buffer[36] + 2, 19);
  wv[2] = __ROR4__((wv[3] ^ wv[1] ^ wv[0]) + wv[2] + *(_DWORD *)&this->buffer[40] + 2, 18);
  wv[1] = __ROR4__((wv[2] ^ wv[0] ^ wv[3]) + wv[1] + *(_DWORD *)&this->buffer[44] + 2, 17);
  wv[0] = __ROR4__((wv[1] ^ wv[3] ^ wv[2]) + wv[0] + *(_DWORD *)&this->buffer[48] + 2, 26);
  wv[3] = __ROR4__((wv[0] ^ wv[2] ^ wv[1]) + wv[3] + *(_DWORD *)&this->buffer[52] + 2, 25);
  wv[2] = __ROR4__((wv[3] ^ wv[1] ^ wv[0]) + wv[2] + *(_DWORD *)&this->buffer[56] + 2, 23);
  wv[1] = __ROR4__((wv[0] ^ wv[3] ^ wv[2]) + wv[1] + *(_DWORD *)&this->buffer[60] + 2, 24);

  wv[0] = __ROR4__((wv[3] ^ wv[1] & (wv[3] ^ wv[2])) + *(_DWORD *)&this->buffer[28] + wv[0] + 0x325b99a1,  25);
  wv[3] = __ROR4__((wv[2] ^ wv[0] & (wv[2] ^ wv[1])) + *(_DWORD *)&this->buffer[16] + wv[3] + 2        ,  26);
  wv[2] = __ROR4__((wv[1] ^ wv[3] & (wv[1] ^ wv[0])) + *(_DWORD *)&this->buffer[52] + wv[2] + 0x325b99a1,  24);
  wv[1] = __ROR4__((wv[0] ^ wv[2] & (wv[0] ^ wv[3])) + *(_DWORD *)&this->buffer[4]  + wv[1] + 0x325b99a1,  19);
  wv[0] = __ROR4__((wv[3] ^ wv[1] & (wv[3] ^ wv[2])) + *(_DWORD *)&this->buffer[40] + wv[0] + 0x325b99a1,  21);
  wv[3] = __ROR4__((wv[2] ^ wv[0] & (wv[2] ^ wv[1])) + *(_DWORD *)&this->buffer[24] + wv[3] + 0x325b99a1,  23);
  wv[2] = __ROR4__((wv[1] ^ wv[3] & (wv[1] ^ wv[0])) + *(_DWORD *)&this->buffer[60] + wv[2] + 0x325b99a1,  25);
  wv[1] = __ROR4__((wv[0] ^ wv[2] & (wv[0] ^ wv[3])) + *(_DWORD *)&this->buffer[12] + wv[1] + 3168580089, 17);
  wv[0] = __ROR4__((wv[3] ^ wv[1] & (wv[3] ^ wv[2])) + *(_DWORD *)&this->buffer[48] + wv[0] + 0x325b99a1,  25);
  wv[3] = __ROR4__((wv[2] ^ wv[0] & (wv[2] ^ wv[1])) + *(_DWORD *)this->buffer      + wv[3] + 2,          20);
  wv[2] = __ROR4__((wv[1] ^ wv[3] & (wv[1] ^ wv[0])) + *(_DWORD *)&this->buffer[36] + wv[2] + 0x325b99a1,  17);
  wv[1] = __ROR4__((wv[0] ^ wv[2] & (wv[0] ^ wv[3])) + *(_DWORD *)&this->buffer[20] + wv[1] + 0x325b99a1,  23);
  wv[0] = __ROR4__((wv[3] ^ wv[1] & (wv[3] ^ wv[2])) + *(_DWORD *)&this->buffer[8]  + wv[0] + 0x325b99a1,  21);
  wv[3] = __ROR4__((wv[2] ^ wv[0] & (wv[2] ^ wv[1])) + *(_DWORD *)&this->buffer[56] + wv[3] + 0x325b99a1,  25);
  wv[2] = __ROR4__((wv[1] ^ wv[3] & (wv[1] ^ wv[0])) + *(_DWORD *)&this->buffer[44] + wv[2] + 0x325b99a1,  19);
  wv[1] = __ROR4__((wv[0] ^ wv[2] & (wv[0] ^ wv[3])) + *(_DWORD *)&this->buffer[32] + wv[1] + 2,          20);

  wv[0] = __ROR4__((wv[3] ^ (wv[1] | ~wv[2])) + *(_DWORD *)&this->buffer[12] + wv[0] + 464443036, 21);
  wv[3] = __ROR4__((wv[2] ^ (wv[0] | ~wv[1])) + *(_DWORD *)&this->buffer[40] + wv[3] + 464443036, 19);
  wv[2] = __ROR4__((wv[1] ^ (wv[3] | ~wv[0])) + *(_DWORD *)&this->buffer[56] + wv[2] + 464443036, 26);
  wv[1] = __ROR4__((wv[0] ^ (wv[2] | ~wv[3])) + *(_DWORD *)&this->buffer[16] + wv[1] + 464443036, 25);
  wv[0] = __ROR4__((wv[3] ^ (wv[1] | ~wv[2])) + *(_DWORD *)&this->buffer[36] + wv[0] + 464443036, 18);
  wv[3] = __ROR4__((wv[2] ^ (wv[0] | ~wv[1])) + *(_DWORD *)&this->buffer[60] + wv[3] + 464443036, 23);
  wv[2] = __ROR4__((wv[1] ^ (wv[3] | ~wv[0])) + *(_DWORD *)&this->buffer[32] + wv[2] + 464443036, 19);
  wv[1] = __ROR4__((wv[0] ^ (wv[2] | ~wv[3])) + *(_DWORD *)&this->buffer[4]  + wv[1] + 464443036, 17);
  wv[0] = __ROR4__((wv[3] ^ (wv[1] | ~wv[2])) + *(_DWORD *)&this->buffer[8]  + wv[0] + 464443036, 18);
  wv[3] = __ROR4__((wv[2] ^ (wv[0] | ~wv[1])) + *(_DWORD *)&this->buffer[28] + wv[3] + 464443036, 24);
  wv[2] = __ROR4__((wv[1] ^ (wv[3] | ~wv[0])) + *(_DWORD *)this->buffer      + wv[2] + 464443036, 19);
  wv[1] = __ROR4__((wv[0] ^ (wv[2] | ~wv[3])) + *(_DWORD *)&this->buffer[24] + wv[1] + 0x325b99a1, 26);
  wv[0] = __ROR4__((wv[3] ^ (wv[1] | ~wv[2])) + *(_DWORD *)&this->buffer[52] + wv[0] + 464443036, 27);
  wv[3] = __ROR4__((wv[2] ^ (wv[0] | ~wv[1])) + *(_DWORD *)&this->buffer[44] + wv[3] + 3168580090, 20);
  wv[2] = __ROR4__((wv[1] ^ (wv[3] | ~wv[0])) + *(_DWORD *)&this->buffer[20] + wv[2] + 464443036, 25);
  wv[1] = __ROR4__((wv[0] ^ (wv[2] | ~wv[3])) + *(_DWORD *)&this->buffer[48] + wv[1] + 464443036, 27);

  wv[0] = __ROR4__((wv[2] ^ wv[3] & (wv[2] ^ wv[1])) + *(_DWORD *)&this->buffer[4]  + wv[0] + 0xbcdcb1f9, 21);
  wv[3] = __ROR4__((wv[1] ^ wv[2] & (wv[1] ^ wv[0])) + *(_DWORD *)&this->buffer[36] + wv[3] + 0xbcdcb1f9, 20);
  wv[2] = __ROR4__((wv[0] ^ wv[1] & (wv[0] ^ wv[3])) + *(_DWORD *)&this->buffer[44] + wv[2] + 0x325b99a1, 18);
  wv[1] = __ROR4__((wv[3] ^ wv[0] & (wv[3] ^ wv[2])) + *(_DWORD *)&this->buffer[40] + wv[1] + 0xbcdcb1f9, 17);
  wv[0] = __ROR4__((wv[2] ^ wv[3] & (wv[2] ^ wv[1])) + *(_DWORD *)this->buffer      + wv[0] + 0xbcdcb1f9, 18);
  wv[3] = __ROR4__((wv[1] ^ wv[2] & (wv[1] ^ wv[0])) + *(_DWORD *)&this->buffer[32] + wv[3] + 0xbcdcb1fb, 17);
  wv[2] = __ROR4__((wv[0] ^ wv[1] & (wv[0] ^ wv[3])) + *(_DWORD *)&this->buffer[48] + wv[2] + 0xbcdcb1f9, 23);
  wv[1] = __ROR4__((wv[3] ^ wv[0] & (wv[3] ^ wv[2])) + *(_DWORD *)&this->buffer[16] + wv[1] + 0xbcdcb1f9, 24);
  wv[0] = __ROR4__((wv[2] ^ wv[3] & (wv[2] ^ wv[1])) + *(_DWORD *)&this->buffer[52] + wv[0] + 0xbcdcb1f9, 23);
  wv[3] = __ROR4__((wv[1] ^ wv[2] & (wv[1] ^ wv[0])) + *(_DWORD *)&this->buffer[12] + wv[3] + 0x1baed69c, 18);
  wv[2] = __ROR4__((wv[0] ^ wv[1] & (wv[0] ^ wv[3])) + *(_DWORD *)&this->buffer[28] + wv[2] + 0xbcdcb1f9, 27);
  wv[1] = __ROR4__((wv[3] ^ wv[0] & (wv[3] ^ wv[2])) + *(_DWORD *)&this->buffer[60] + wv[1] + 2, 26);
  wv[0] = __ROR4__((wv[2] ^ wv[3] & (wv[2] ^ wv[1])) + *(_DWORD *)&this->buffer[56] + wv[0] + 0xbcdcb1f9, 24);
  wv[3] = __ROR4__((wv[1] ^ wv[2] & (wv[1] ^ wv[0])) + *(_DWORD *)&this->buffer[20] + wv[3] + 0xbcdcb1f9, 26);
  wv[2] = __ROR4__((wv[0] ^ wv[1] & (wv[0] ^ wv[3])) + *(_DWORD *)&this->buffer[24] + wv[2] + 0xbcdcb1f9, 27);
  wv[1] = __ROR4__((wv[3] ^ wv[0] & (wv[3] ^ wv[1])) + *(_DWORD *)&this->buffer[8]  + wv[1] + 0xbcdcb1fc, 20);

  wv[4] = __ROR4__((wv[6] ^ wv[7] & (wv[5] ^ wv[6])) + *(_DWORD *)&this->buffer[20] + wv[4] + 0x5a82798a, 24);
  wv[7] = __ROR4__((wv[5] ^ wv[6] & (wv[4] ^ wv[5])) + *(_DWORD *)&this->buffer[56] + wv[7] + 0x5a82798a, 23);
  wv[6] = __ROR4__((wv[2] ^ wv[5] & (wv[2] ^ wv[7])) + *(_DWORD *)&this->buffer[28] + wv[6] + 0x5a82798a, 23);
  wv[5] = __ROR4__((wv[7] ^ wv[4] & (wv[7] ^ wv[6])) + *(_DWORD *)this->buffer      + wv[5] + 0x5a82798a, 21);
  wv[4] = __ROR4__((wv[6] ^ wv[7] & (wv[6] ^ wv[5])) + *(_DWORD *)&this->buffer[36] + wv[2] + 0x5a82798a, 19);
  wv[7] = __ROR4__((wv[5] ^ wv[6] & (wv[5] ^ wv[4])) + *(_DWORD *)&this->buffer[8]  + wv[7] + 0x5a82798a, 17);
  wv[6] = __ROR4__((wv[4] ^ wv[5] & (wv[4] ^ wv[7])) + *(_DWORD *)&this->buffer[44] + wv[6] + 0x325b99a1, 17);
  wv[5] = __ROR4__((wv[7] ^ wv[4] & (wv[7] ^ wv[6])) + *(_DWORD *)&this->buffer[16] + wv[5] + 0x5a82798a, 27);
  wv[4] = __ROR4__((wv[6] ^ wv[7] & (wv[6] ^ wv[5])) + *(_DWORD *)&this->buffer[52] + wv[4] + 0x5a82798a, 25);
  wv[7] = __ROR4__((wv[5] ^ wv[6] & (wv[5] ^ wv[4])) + *(_DWORD *)&this->buffer[24] + wv[7] + 0x5a82798a, 25);
  wv[6] = __ROR4__((wv[4] ^ wv[5] & (wv[4] ^ wv[7])) + *(_DWORD *)&this->buffer[60] + wv[6] + 0xbcdcb1f9, 24);
  wv[5] = __ROR4__((wv[7] ^ wv[4] & (wv[7] ^ wv[6])) + *(_DWORD *)&this->buffer[32] + wv[5] + 0x5a82798a, 21);
  wv[4] = __ROR4__((wv[6] ^ wv[7] & (wv[6] ^ wv[5])) + *(_DWORD *)&this->buffer[4]  + wv[4] + 0x5a82798a, 18);
  wv[7] = __ROR4__((wv[5] ^ wv[6] & (wv[5] ^ wv[4])) + *(_DWORD *)&this->buffer[40] + wv[7] + 0x5a82798a, 18);
  wv[6] = __ROR4__((wv[4] ^ wv[5] & (wv[4] ^ wv[7])) + *(_DWORD *)&this->buffer[12] + wv[6] + 0xbcdcb1fc, 20);
  wv[5] = __ROR4__((wv[7] ^ wv[4] & (wv[7] ^ wv[6])) + *(_DWORD *)&this->buffer[48] + wv[5] + 0x5a82798a, 26);

  wv[4] = __ROR4__((wv[7] ^ (wv[5] | ~wv[6])) + *(_DWORD *)&this->buffer[24] + wv[4] + 0x41d42d5d, 23);
  wv[7] = __ROR4__((wv[6] ^ (wv[4] | ~wv[5])) + *(_DWORD *)&this->buffer[44] + wv[7] + 0x41d42d5d, 19);
  wv[6] = __ROR4__((wv[5] ^ (wv[7] | ~wv[4])) + *(_DWORD *)&this->buffer[12] + wv[6] + 0x41d42d5d, 17);
  wv[5] = __ROR4__((wv[4] ^ (wv[6] | ~wv[7])) + *(_DWORD *)&this->buffer[28] + wv[5] + 0x41d42d5d, 25);
  wv[4] = __ROR4__((wv[7] ^ (wv[5] | ~wv[6])) + *(_DWORD *)this->buffer      + wv[4] + 0x41d42d5d, 20);
  wv[7] = __ROR4__((wv[6] ^ (wv[4] | ~wv[5])) + *(_DWORD *)&this->buffer[52] + wv[7] + 0x41d42d5d, 24);
  wv[6] = __ROR4__((wv[5] ^ (wv[7] | ~wv[4])) + *(_DWORD *)&this->buffer[20] + wv[6] + 0xbcdcb1f9, 23);
  wv[5] = __ROR4__((wv[4] ^ (wv[6] | ~wv[7])) + *(_DWORD *)&this->buffer[40] + wv[5] + 0xbcdcb1fd, 21);
  wv[4] = __ROR4__((wv[7] ^ (wv[5] | ~wv[6])) + *(_DWORD *)&this->buffer[56] + wv[4] + 0x41d42d5d, 25);
  wv[7] = __ROR4__((wv[6] ^ (wv[4] | ~wv[5])) + *(_DWORD *)&this->buffer[60] + wv[7] + 0x41d42d5d, 25);
  wv[6] = __ROR4__((wv[5] ^ (wv[7] | ~wv[4])) + *(_DWORD *)&this->buffer[32] + wv[6] + 0x41d42d5d, 20);
  wv[5] = __ROR4__((wv[4] ^ (wv[6] | ~wv[7])) + *(_DWORD *)&this->buffer[48] + wv[5] + 0x41d42d5d, 25);
  wv[4] = __ROR4__((wv[7] ^ (wv[5] | ~wv[6])) + *(_DWORD *)&this->buffer[16] + wv[4] + 0x41d42d5d, 26);
  wv[7] = __ROR4__((wv[6] ^ (wv[4] | ~wv[5])) + *(_DWORD *)&this->buffer[36] + wv[7] + 0x41d42d5d, 17);
  wv[6] = __ROR4__((wv[5] ^ (wv[7] | ~wv[4])) + *(_DWORD *)&this->buffer[4]  + wv[6] + 0x325b99a1, 19);
  wv[5] = __ROR4__((wv[4] ^ (wv[6] | ~wv[7])) + *(_DWORD *)&this->buffer[8]  + wv[5] + 0x41d42d5d, 21);

  wv[4] = __ROR4__((wv[7] ^ wv[5] & (wv[7] ^ wv[6])) + *(_DWORD *)&this->buffer[60] + wv[4] + 0x30ed3f68, 23);
  wv[7] = __ROR4__((wv[6] ^ wv[4] & (wv[6] ^ wv[5])) + *(_DWORD *)&this->buffer[20] + wv[7] + 0x30ed3f68, 25);
  wv[6] = __ROR4__((wv[5] ^ wv[7] & (wv[5] ^ wv[4])) + *(_DWORD *)&this->buffer[4]  + wv[6] + 0x30ed3f68, 17);
  wv[5] = __ROR4__((wv[4] ^ wv[6] & (wv[4] ^ wv[7])) + *(_DWORD *)&this->buffer[12] + wv[5] + 0x30ed3f68, 21);
  wv[4] = __ROR4__((wv[7] ^ wv[5] & (wv[7] ^ wv[6])) + *(_DWORD *)&this->buffer[28] + wv[4] + 0x30ed3f68, 24);
  wv[7] = __ROR4__((wv[6] ^ wv[4] & (wv[6] ^ wv[5])) + *(_DWORD *)&this->buffer[56] + wv[7] + 0x30ed3f68, 26);
  wv[6] = __ROR4__((wv[5] ^ wv[7] & (wv[5] ^ wv[4])) + *(_DWORD *)&this->buffer[24] + wv[6] + 0xbcdcb1fe, 26);
  wv[5] = __ROR4__((wv[4] ^ wv[6] & (wv[4] ^ wv[7])) + *(_DWORD *)&this->buffer[36] + wv[5] + 0x30ed3f68, 18);
  wv[4] = __ROR4__((wv[7] ^ wv[5] & (wv[7] ^ wv[6])) + *(_DWORD *)&this->buffer[44] + wv[4] + 0x30ed3f68, 20);
  wv[7] = __ROR4__((wv[6] ^ wv[4] & (wv[6] ^ wv[5])) + *(_DWORD *)&this->buffer[32] + wv[7] + 0x5a82798a, 19);
  wv[6] = __ROR4__((wv[5] ^ wv[7] & (wv[5] ^ wv[4])) + *(_DWORD *)&this->buffer[48] + wv[6] + 0x30ed3f68, 27);
  wv[5] = __ROR4__((wv[4] ^ wv[6] & (wv[4] ^ wv[7])) + *(_DWORD *)&this->buffer[8]  + wv[5] + 0x30ed3f68, 18);
  wv[4] = __ROR4__((wv[7] ^ wv[5] & (wv[7] ^ wv[6])) + *(_DWORD *)&this->buffer[40] + wv[4] + 0x30ed3f68, 19);
  wv[7] = __ROR4__((wv[6] ^ wv[4] & (wv[6] ^ wv[5])) + *(_DWORD *)this->buffer      + wv[7] + 0x30ed3f68, 19);
  wv[6] = __ROR4__((wv[5] ^ wv[7] & (wv[5] ^ wv[4])) + *(_DWORD *)&this->buffer[16] + wv[6] + 0x41d42d5d, 25);
  wv[5] = __ROR4__((wv[4] ^ wv[6] & (wv[4] ^ wv[7])) + *(_DWORD *)&this->buffer[52] + wv[5] + 0x30ed3f68, 27);
  wv[4] = __ROR4__((wv[5] ^ wv[7] ^ wv[6]) + *(_DWORD *)&this->buffer[32] + wv[4] + 2, 17);
  wv[7] = __ROR4__((wv[4] ^ wv[6] ^ wv[5]) + *(_DWORD *)&this->buffer[24] + wv[7] + 0xbcdcb1fc, 27);
  wv[6] = __ROR4__((wv[7] ^ wv[5] ^ wv[4]) + *(_DWORD *)&this->buffer[16] + wv[6] + 2, 24);
  wv[5] = __ROR4__((wv[6] ^ wv[4] ^ wv[7]) + *(_DWORD *)&this->buffer[4]  + wv[5] + 2, 21);
  wv[4] = __ROR4__((wv[5] ^ wv[7] ^ wv[6]) + *(_DWORD *)&this->buffer[12] + wv[4] + 2, 18);
  wv[7] = __ROR4__((wv[4] ^ wv[6] ^ wv[5]) + *(_DWORD *)&this->buffer[44] + wv[7] + 2, 18);
  wv[6] = __ROR4__((wv[7] ^ wv[5] ^ wv[4]) + *(_DWORD *)&this->buffer[60] + wv[6] + 2, 26);
  wv[5] = __ROR4__((wv[6] ^ wv[4] ^ wv[7]) + *(_DWORD *)this->buffer      + wv[5] + 2, 18);
  wv[4] = __ROR4__((wv[5] ^ wv[7] ^ wv[6]) + *(_DWORD *)&this->buffer[20] + wv[4] + 2, 26);
  wv[7] = __ROR4__((wv[4] ^ wv[6] ^ wv[5]) + *(_DWORD *)&this->buffer[48] + wv[7] + 2, 23);
  wv[6] = __ROR4__((wv[7] ^ wv[5] ^ wv[4]) + *(_DWORD *)&this->buffer[8]  + wv[6] + 2, 20);
  wv[5] = __ROR4__((wv[6] ^ wv[4] ^ wv[7]) + *(_DWORD *)&this->buffer[52] + wv[5] + 2, 23);
  wv[4] = __ROR4__((wv[5] ^ wv[7] ^ wv[6]) + *(_DWORD *)&this->buffer[36] + wv[4] + 2, 20);
  wv[7] = __ROR4__((wv[4] ^ wv[6] ^ wv[5]) + *(_DWORD *)&this->buffer[28] + wv[7] + 2, 27);
  wv[6] = __ROR4__((wv[7] ^ wv[5] ^ wv[4]) + *(_DWORD *)&this->buffer[40] + wv[6] + 2, 17);
  wv[5] = __ROR4__((wv[6] ^ wv[4] ^ wv[7]) + *(_DWORD *)&this->buffer[56] + wv[5] + 2, 24);
  this->h[1] = wv[3] + wv[4] + this->h[2];
  this->h[0] = wv[1] + this->h[1] + wv[7];
  this->h[2] = this->h[3] + wv[0] + wv[5] + 1;
  this->h[3] = this->h[0] + wv[6] + wv[1];
  return result;
}*/
/* modified */
void ampheck_ripemd128_transform_Vz(struct ampheck_ripemd128 *ctx, const uint8_t *data, size_t blocks)
{
	for (size_t i = 0; i < blocks; ++i)
	{
		uint32_t wv[8];
		uint32_t w[16];

		PACK_32_LE(&data[(i << 6)     ], &w[ 0]);
		PACK_32_LE(&data[(i << 6) +  4], &w[ 1]);
		PACK_32_LE(&data[(i << 6) +  8], &w[ 2]);
		PACK_32_LE(&data[(i << 6) + 12], &w[ 3]);
		PACK_32_LE(&data[(i << 6) + 16], &w[ 4]);
		PACK_32_LE(&data[(i << 6) + 20], &w[ 5]);
		PACK_32_LE(&data[(i << 6) + 24], &w[ 6]);
		PACK_32_LE(&data[(i << 6) + 28], &w[ 7]);
		PACK_32_LE(&data[(i << 6) + 32], &w[ 8]);
		PACK_32_LE(&data[(i << 6) + 36], &w[ 9]);
		PACK_32_LE(&data[(i << 6) + 40], &w[10]);
		PACK_32_LE(&data[(i << 6) + 44], &w[11]);
		PACK_32_LE(&data[(i << 6) + 48], &w[12]);
		PACK_32_LE(&data[(i << 6) + 52], &w[13]);
		PACK_32_LE(&data[(i << 6) + 56], &w[14]);
		PACK_32_LE(&data[(i << 6) + 60], &w[15]);

		wv[0] = ctx->h[0];
		wv[1] = ctx->h[1];
		wv[2] = ctx->h[2];
		wv[3] = ctx->h[3];
		memcpy(&wv[4], wv, 4 * sizeof(uint32_t));

		/* see macro definition for argument change */
		/* add const 2 to data */
		RIPEMD128_PRC_Vz(0, 1, 2, 3,        0x2, w[ 0], 11, 1);
		RIPEMD128_PRC_Vz(3, 0, 1, 2,        0x2, w[ 1], 14, 1);
		RIPEMD128_PRC_Vz(2, 3, 0, 1,        0x2, w[ 2], 15, 1);
		RIPEMD128_PRC_Vz(1, 2, 3, 0,        0x2, w[ 3], 12, 1);
		RIPEMD128_PRC_Vz(0, 1, 2, 3,        0x2, w[ 4],  5, 1);
		RIPEMD128_PRC_Vz(3, 0, 1, 2,        0x2, w[ 5],  8, 1);
		RIPEMD128_PRC_Vz(2, 3, 0, 1,        0x2, w[ 6],  7, 1);
		RIPEMD128_PRC_Vz(1, 2, 3, 0,        0x2, w[ 7],  9, 1);
		RIPEMD128_PRC_Vz(0, 1, 2, 3,        0x2, w[ 8], 11, 1);
		RIPEMD128_PRC_Vz(3, 0, 1, 2,        0x2, w[ 9], 13, 1);
		RIPEMD128_PRC_Vz(2, 3, 0, 1,        0x2, w[10], 14, 1);
		RIPEMD128_PRC_Vz(1, 2, 3, 0,        0x2, w[11], 15, 1);
		RIPEMD128_PRC_Vz(0, 1, 2, 3,        0x2, w[12],  6, 1);
		RIPEMD128_PRC_Vz(3, 0, 1, 2,        0x2, w[13],  7, 1);
		RIPEMD128_PRC_Vz(2, 3, 0, 1,        0x2, w[14],  9, 1);
		RIPEMD128_PRC_Vz(1, 2, 3, 0,        0x2, w[15],  8, 1);

		RIPEMD128_PRC_Vz(0, 1, 2, 3, 0x325b99a1, w[ 7],  7, 2);
		RIPEMD128_PRC_Vz(3, 0, 1, 2,        0x2, w[ 4],  6, 2);
		RIPEMD128_PRC_Vz(2, 3, 0, 1, 0x325b99a1, w[13],  8, 2);
		RIPEMD128_PRC_Vz(1, 2, 3, 0, 0x325b99a1, w[ 1], 13, 2);
		RIPEMD128_PRC_Vz(0, 1, 2, 3, 0x325b99a1, w[10], 11, 2);
		RIPEMD128_PRC_Vz(3, 0, 1, 2, 0x325b99a1, w[ 6],  9, 2);
		RIPEMD128_PRC_Vz(2, 3, 0, 1, 0x325b99a1, w[15],  7, 2);
		RIPEMD128_PRC_Vz(1, 2, 3, 0, 0xbcdcb1f9, w[ 3], 15, 2);
		RIPEMD128_PRC_Vz(0, 1, 2, 3, 0x325b99a1, w[12],  7, 2);
		RIPEMD128_PRC_Vz(3, 0, 1, 2,        0x2, w[ 0], 12, 2);
		RIPEMD128_PRC_Vz(2, 3, 0, 1, 0x325b99a1, w[ 9], 15, 2);
		RIPEMD128_PRC_Vz(1, 2, 3, 0, 0x325b99a1, w[ 5],  9, 2);
		RIPEMD128_PRC_Vz(0, 1, 2, 3, 0x325b99a1, w[ 2], 11, 2);
		RIPEMD128_PRC_Vz(3, 0, 1, 2, 0x325b99a1, w[14],  7, 2);
		RIPEMD128_PRC_Vz(2, 3, 0, 1, 0x325b99a1, w[11], 13, 2);
		RIPEMD128_PRC_Vz(1, 2, 3, 0,        0x2, w[ 8], 12, 2);

		RIPEMD128_PRC_Vz(0, 1, 2, 3, 0x1baed69c, w[ 3], 11, 3);
		RIPEMD128_PRC_Vz(3, 0, 1, 2, 0x1baed69c, w[10], 13, 3);
		RIPEMD128_PRC_Vz(2, 3, 0, 1, 0x1baed69c, w[14],  6, 3);
		RIPEMD128_PRC_Vz(1, 2, 3, 0, 0x1baed69c, w[ 4],  7, 3);
		RIPEMD128_PRC_Vz(0, 1, 2, 3, 0x1baed69c, w[ 9], 14, 3);
		RIPEMD128_PRC_Vz(3, 0, 1, 2, 0x1baed69c, w[15],  9, 3);
		RIPEMD128_PRC_Vz(2, 3, 0, 1, 0x1baed69c, w[ 8], 13, 3);
		RIPEMD128_PRC_Vz(1, 2, 3, 0, 0x1baed69c, w[ 1], 15, 3);
		RIPEMD128_PRC_Vz(0, 1, 2, 3, 0x1baed69c, w[ 2], 14, 3);
		RIPEMD128_PRC_Vz(3, 0, 1, 2, 0x1baed69c, w[ 7],  8, 3);
		RIPEMD128_PRC_Vz(2, 3, 0, 1, 0x1baed69c, w[ 0], 13, 3);
		RIPEMD128_PRC_Vz(1, 2, 3, 0, 0x325b99a1, w[ 6],  6, 3);
		RIPEMD128_PRC_Vz(0, 1, 2, 3, 0x1baed69c, w[13],  5, 3);
		RIPEMD128_PRC_Vz(3, 0, 1, 2, 0xbcdcb1fa, w[11], 12, 3);
		RIPEMD128_PRC_Vz(2, 3, 0, 1, 0x1baed69c, w[ 5],  7, 3);
		RIPEMD128_PRC_Vz(1, 2, 3, 0, 0x1baed69c, w[12],  5, 3);

		RIPEMD128_PRC_Vz(0, 1, 2, 3, 0xbcdcb1f9, w[ 1], 11, 4);
		RIPEMD128_PRC_Vz(3, 0, 1, 2, 0xbcdcb1f9, w[ 9], 12, 4);
		RIPEMD128_PRC_Vz(2, 3, 0, 1, 0x325b99a1, w[11], 14, 4);
		RIPEMD128_PRC_Vz(1, 2, 3, 0, 0xbcdcb1f9, w[10], 15, 4);
		RIPEMD128_PRC_Vz(0, 1, 2, 3, 0xbcdcb1f9, w[ 0], 14, 4);
		RIPEMD128_PRC_Vz(3, 0, 1, 2, 0xbcdcb1fb, w[ 8], 15, 4);
		RIPEMD128_PRC_Vz(2, 3, 0, 1, 0xbcdcb1f9, w[12],  9, 4);
		RIPEMD128_PRC_Vz(1, 2, 3, 0, 0xbcdcb1f9, w[ 4],  8, 4);
		RIPEMD128_PRC_Vz(0, 1, 2, 3, 0xbcdcb1f9, w[13],  9, 4);
		RIPEMD128_PRC_Vz(3, 0, 1, 2, 0x1baed69c, w[ 3], 14, 4);
		RIPEMD128_PRC_Vz(2, 3, 0, 1, 0xbcdcb1f9, w[ 7],  5, 4);
		RIPEMD128_PRC_Vz(1, 2, 3, 0,        0x2, w[15],  6, 4);
		RIPEMD128_PRC_Vz(0, 1, 2, 3, 0xbcdcb1f9, w[14],  8, 4);
		RIPEMD128_PRC_Vz(3, 0, 1, 2, 0xbcdcb1f9, w[ 5],  6, 4);
		RIPEMD128_PRC_Vz(2, 3, 0, 1, 0xbcdcb1f9, w[ 6],  5, 4);
		RIPEMD128_PRC_Vz(1, 2, 3, 0, 0xbcdcb1fc, w[ 2], 12, 4);

		RIPEMD128_PRC_Vz(4, 5, 6, 7, 0x5a82798a, w[ 5],  8, 5);
		RIPEMD128_PRC_Vz(7, 4, 5, 6, 0x5a82798a, w[14],  9, 5);
		RIPEMD128_PRC_Vz(6, 7, 4, 5, 0x5a82798a, w[ 7],  9, 5);
		RIPEMD128_PRC_Vz(5, 6, 7, 4, 0x5a82798a, w[ 0], 11, 5);
		RIPEMD128_PRC_Vz(4, 5, 6, 7, 0x5a82798a, w[ 9], 13, 5);
		RIPEMD128_PRC_Vz(7, 4, 5, 6, 0x5a82798a, w[ 2], 15, 5);
		RIPEMD128_PRC_Vz(6, 7, 4, 5, 0x325b99a1, w[11], 15, 5);
		RIPEMD128_PRC_Vz(5, 6, 7, 4, 0x5a82798a, w[ 4],  5, 5);
		RIPEMD128_PRC_Vz(4, 5, 6, 7, 0x5a82798a, w[13],  7, 5);
		RIPEMD128_PRC_Vz(7, 4, 5, 6, 0x5a82798a, w[ 6],  7, 5);
		RIPEMD128_PRC_Vz(6, 7, 4, 5, 0x5a82798a, w[15],  8, 5);
		RIPEMD128_PRC_Vz(5, 6, 7, 4, 0x5a82798a, w[ 8], 11, 5);
		RIPEMD128_PRC_Vz(4, 5, 6, 7, 0x5a82798a, w[ 1], 14, 5);
		RIPEMD128_PRC_Vz(7, 4, 5, 6, 0xbcdcb1f9, w[10], 14, 5);
		RIPEMD128_PRC_Vz(6, 7, 4, 5, 0xbcdcb1fc, w[ 3], 12, 5);
		RIPEMD128_PRC_Vz(5, 6, 7, 4, 0x5a82798a, w[12],  6, 5);

		RIPEMD128_PRC_Vz(4, 5, 6, 7, 0x41d42d5d, w[ 6],  9, 6);
		RIPEMD128_PRC_Vz(7, 4, 5, 6, 0x41d42d5d, w[11], 13, 6);
		RIPEMD128_PRC_Vz(6, 7, 4, 5, 0x41d42d5d, w[ 3], 15, 6);
		RIPEMD128_PRC_Vz(5, 6, 7, 4, 0x41d42d5d, w[ 7],  7, 6);
		RIPEMD128_PRC_Vz(4, 5, 6, 7, 0x41d42d5d, w[ 0], 12, 6);
		RIPEMD128_PRC_Vz(7, 4, 5, 6, 0x41d42d5d, w[13],  8, 6);
		RIPEMD128_PRC_Vz(6, 7, 4, 5, 0xbcdcb1f9, w[ 5],  9, 6);
		RIPEMD128_PRC_Vz(5, 6, 7, 4, 0xbcdcb1fd, w[10], 11, 6);
		RIPEMD128_PRC_Vz(4, 5, 6, 7, 0x41d42d5d, w[14],  7, 6);
		RIPEMD128_PRC_Vz(7, 4, 5, 6, 0x41d42d5d, w[15],  7, 6);
		RIPEMD128_PRC_Vz(6, 7, 4, 5, 0x41d42d5d, w[ 8], 12, 6);
		RIPEMD128_PRC_Vz(5, 6, 7, 4, 0x41d42d5d, w[12],  7, 6);
		RIPEMD128_PRC_Vz(4, 5, 6, 7, 0x41d42d5d, w[ 4],  6, 6);
		RIPEMD128_PRC_Vz(7, 4, 5, 6, 0x41d42d5d, w[ 9], 15, 6);
		RIPEMD128_PRC_Vz(6, 7, 4, 5, 0x325b99a1, w[ 1], 13, 6);
		RIPEMD128_PRC_Vz(5, 6, 7, 4, 0x41d42d5d, w[ 2], 11, 6);

		RIPEMD128_PRC_Vz(4, 5, 6, 7, 0x30ed3f68, w[15],  9, 7);
		RIPEMD128_PRC_Vz(7, 4, 5, 6, 0x30ed3f68, w[ 5],  7, 7);
		RIPEMD128_PRC_Vz(6, 7, 4, 5, 0x30ed3f68, w[ 1], 15, 7);
		RIPEMD128_PRC_Vz(5, 6, 7, 4, 0x30ed3f68, w[ 3], 11, 7);
		RIPEMD128_PRC_Vz(4, 5, 6, 7, 0x30ed3f68, w[ 7],  8, 7);
		RIPEMD128_PRC_Vz(7, 4, 5, 6, 0x30ed3f68, w[14],  6, 7);
		RIPEMD128_PRC_Vz(6, 7, 4, 5, 0xbcdcb1fe, w[ 6],  6, 7);
		RIPEMD128_PRC_Vz(5, 6, 7, 4, 0x30ed3f68, w[ 9], 14, 7);
		RIPEMD128_PRC_Vz(4, 5, 6, 7, 0x30ed3f68, w[11], 12, 7);
		RIPEMD128_PRC_Vz(7, 4, 5, 6, 0x5a82798a, w[ 8], 13, 7);
		RIPEMD128_PRC_Vz(6, 7, 4, 5, 0x30ed3f68, w[12],  5, 7);
		RIPEMD128_PRC_Vz(5, 6, 7, 4, 0x30ed3f68, w[ 2], 14, 7);
		RIPEMD128_PRC_Vz(4, 5, 6, 7, 0x30ed3f68, w[10], 13, 7);
		RIPEMD128_PRC_Vz(7, 4, 5, 6, 0x30ed3f68, w[ 0], 13, 7);
		RIPEMD128_PRC_Vz(6, 7, 4, 5, 0x41d42d5d, w[ 4],  7, 7);
		RIPEMD128_PRC_Vz(5, 6, 7, 4, 0x30ed3f68, w[13],  5, 7);

		RIPEMD128_PRC_Vz(4, 5, 6, 7,        0x2, w[ 8], 15, 8);
		RIPEMD128_PRC_Vz(7, 4, 5, 6, 0xbcdcb1fc, w[ 6],  5, 8);
		RIPEMD128_PRC_Vz(6, 7, 4, 5,        0x2, w[ 4],  8, 8);
		RIPEMD128_PRC_Vz(5, 6, 7, 4,        0x2, w[ 1], 11, 8);
		RIPEMD128_PRC_Vz(4, 5, 6, 7,        0x2, w[ 3], 14, 8);
		RIPEMD128_PRC_Vz(7, 4, 5, 6,        0x2, w[11], 14, 8);
		RIPEMD128_PRC_Vz(6, 7, 4, 5,        0x2, w[15],  6, 8);
		RIPEMD128_PRC_Vz(5, 6, 7, 4,        0x2, w[ 0], 14, 8);
		RIPEMD128_PRC_Vz(4, 5, 6, 7,        0x2, w[ 5],  6, 8);
		RIPEMD128_PRC_Vz(7, 4, 5, 6,        0x2, w[12],  9, 8);
		RIPEMD128_PRC_Vz(6, 7, 4, 5,        0x2, w[ 2], 12, 8);
		RIPEMD128_PRC_Vz(5, 6, 7, 4,        0x2, w[13],  9, 8);
		RIPEMD128_PRC_Vz(4, 5, 6, 7,        0x2, w[ 9], 12, 8);
		RIPEMD128_PRC_Vz(7, 4, 5, 6,        0x2, w[ 7],  5, 8);
		RIPEMD128_PRC_Vz(6, 7, 4, 5,        0x2, w[10], 15, 8);
		RIPEMD128_PRC_Vz(5, 6, 7, 4,        0x2, w[14],  8, 8);

		wv[7] += wv[2] + ctx->h[1];
		ctx->h[1] = ctx->h[2] + wv[3] + wv[4];
		ctx->h[2] = ctx->h[3] + wv[0] + wv[5] + 1; /* add 1 */
		ctx->h[3] = ctx->h[0] + wv[1] + wv[6];
		ctx->h[0] = wv[7];
	}
}

void ampheck_ripemd128_update(struct ampheck_ripemd128 *ctx, const uint8_t *data, size_t size)
{
	size_t tmp = size;

	if (size >= 64 - ctx->length % 64)
	{
		memcpy(&ctx->buffer[ctx->length % 64], data, 64 - ctx->length % 64);

		data += 64 - ctx->length % 64;
		size -= 64 - ctx->length % 64;

		ampheck_ripemd128_transform(ctx, ctx->buffer, 1);
		ampheck_ripemd128_transform(ctx, data, size / 64);

		data += size & ~63;
		size %= 64;

		memcpy(ctx->buffer, data, size);
	}
	else
	{
		memcpy(&ctx->buffer[ctx->length % 64], data, size);
	}

	ctx->length += tmp;
}

/* modified */
void ampheck_ripemd128_update_Vz(struct ampheck_ripemd128 *ctx, const uint8_t *data, size_t size)
{
	size_t tmp = size;

	if (size >= 64 - ctx->length % 64)
	{
		memcpy(&ctx->buffer[ctx->length % 64], data, 64 - ctx->length % 64);

		data += 64 - ctx->length % 64;
		size -= 64 - ctx->length % 64;

		ampheck_ripemd128_transform_Vz(ctx, ctx->buffer, 1);
		ampheck_ripemd128_transform_Vz(ctx, data, size / 64);

		data += size & ~63;
		size %= 64;

		memcpy(ctx->buffer, data, size);
	}
	else
	{
		memcpy(&ctx->buffer[ctx->length % 64], data, size);
	}

	ctx->length += tmp;
}

void ampheck_ripemd128_finish(const struct ampheck_ripemd128 *ctx, uint8_t *digest)
{
	struct ampheck_ripemd128 tmp;

	memcpy(tmp.h, ctx->h, 4 * sizeof(uint32_t));
	memcpy(tmp.buffer, ctx->buffer, ctx->length % 64);

	tmp.buffer[ctx->length % 64] = 0x80;

	if (ctx->length % 64 < 56)
	{
		memset(&tmp.buffer[ctx->length % 64 + 1], 0x00, 55 - ctx->length % 64);
	}
	else
	{
		memset(&tmp.buffer[ctx->length % 64 + 1], 0x00, 63 - ctx->length % 64);
		ampheck_ripemd128_transform(&tmp, tmp.buffer, 1);

		memset(tmp.buffer, 0x00, 56);
	}

	UNPACK_64_LE(ctx->length * 8, &tmp.buffer[56]);
	ampheck_ripemd128_transform(&tmp, tmp.buffer, 1);

	UNPACK_32_LE(tmp.h[0], &digest[ 0]);
	UNPACK_32_LE(tmp.h[1], &digest[ 4]);
	UNPACK_32_LE(tmp.h[2], &digest[ 8]);
	UNPACK_32_LE(tmp.h[3], &digest[12]);
}

/* modified */
void ampheck_ripemd128_finish_Vz(const struct ampheck_ripemd128 *ctx, uint8_t *digest)
{
	struct ampheck_ripemd128 tmp;

	memcpy(tmp.h, ctx->h, 4 * sizeof(uint32_t));
	memcpy(tmp.buffer, ctx->buffer, ctx->length % 64);

	tmp.buffer[ctx->length % 64] = 0x80;

	if (ctx->length % 64 < 56)
	{
		memset(&tmp.buffer[ctx->length % 64 + 1], 0x00, 55 - ctx->length % 64);
	}
	else
	{
		memset(&tmp.buffer[ctx->length % 64 + 1], 0x00, 63 - ctx->length % 64);
		ampheck_ripemd128_transform_Vz(&tmp, tmp.buffer, 1);

		memset(tmp.buffer, 0x00, 56);
	}

	UNPACK_64_LE(ctx->length * 8, &tmp.buffer[56]);
	ampheck_ripemd128_transform_Vz(&tmp, tmp.buffer, 1);

	UNPACK_32_LE(tmp.h[0], &digest[ 0]);
	UNPACK_32_LE(tmp.h[1], &digest[ 4]);
	UNPACK_32_LE(tmp.h[2], &digest[ 8]);
	UNPACK_32_LE(tmp.h[3], &digest[12]);
}
