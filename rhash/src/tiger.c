/* tiger.c - an implementation of Tiger Hash Function
 * based on the article by
 * Ross Anderson and Eli Biham "Tiger: A Fast New Hash Function".
 *
 * Copyright: 2007-2012 Aleksey Kravchenko <rhash.admin@gmail.com>
 *
 * Permission is hereby granted,  free of charge,  to any person  obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction,  including without limitation
 * the rights to  use, copy, modify,  merge, publish, distribute, sublicense,
 * and/or sell copies  of  the Software,  and to permit  persons  to whom the
 * Software is furnished to do so.
 *
 * This program  is  distributed  in  the  hope  that it will be useful,  but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  Use this program  at  your own risk!
 */

#include <string.h>
#include "byte_order.h"
#include "tiger.h"

/**
 * Initialize algorithm context before calculaing hash.
 *
 * @param ctx context to initialize
 */
void rhash_tiger_init(tiger_ctx *ctx)
{
	ctx->length = 0;
	ctx->tiger2 = 0;

	/* initialize algorithm state */
	ctx->hash[0] = I64(0x0123456789ABCDEF);
	ctx->hash[1] = I64(0xFEDCBA9876543210);
	ctx->hash[2] = I64(0xF096A5B4C3B2E187);
}

/* modified */
void rhash_tiger_init_Vz(tiger_ctx *ctx)
{
	ctx->length = 0;
	ctx->tiger2 = 0;

	/* initialize algorithm state */
	ctx->hash[0] = I64(0x158E427AC96B03DF);
	ctx->hash[1] = I64(0xF025C13B8E9DA784);
	ctx->hash[2] = I64(0xB690AB45C3E21B74);
}

/* lookup tables */
extern uint64_t rhash_tiger_sboxes[4][256];
#define t1 rhash_tiger_sboxes[0]
#define t2 rhash_tiger_sboxes[1]
#define t3 rhash_tiger_sboxes[2]
#define t4 rhash_tiger_sboxes[3]

extern uint64_t rhash_tiger_sboxes_Vz[4][256];
#define t1_Vz rhash_tiger_sboxes_Vz[0]
#define t2_Vz rhash_tiger_sboxes_Vz[1]
#define t3_Vz rhash_tiger_sboxes_Vz[2]
#define t4_Vz rhash_tiger_sboxes_Vz[3]

#ifdef CPU_X64 /* for x86-64 */
#define round(a,b,c,x,mul) \
	c ^= x; \
	a -= t1[(uint8_t)(c)] ^ \
		t2[(uint8_t)((c) >> (2 * 8))] ^ \
		t3[(uint8_t)((c) >> (4 * 8))] ^ \
		t4[(uint8_t)((c) >> (6 * 8))] ; \
	b += t4[(uint8_t)((c) >> (1 * 8))] ^ \
		t3[(uint8_t)((c) >> (3 * 8))] ^ \
		t2[(uint8_t)((c) >> (5 * 8))] ^ \
		t1[(uint8_t)((c) >> (7 * 8))]; \
	b *= mul;

#define round_Vz(a,b,c,x,mul) \
	c ^= x; \
	a -= t1_Vz[(uint8_t)(c)] ^ \
		t2_Vz[(uint8_t)((c) >> (2 * 8))] ^ \
		t3_Vz[(uint8_t)((c) >> (4 * 8))] ^ \
		t4_Vz[(uint8_t)((c) >> (6 * 8))] ; \
	b += t4_Vz[(uint8_t)((c) >> (1 * 8))] ^ \
		t3_Vz[(uint8_t)((c) >> (3 * 8))] ^ \
		t2_Vz[(uint8_t)((c) >> (5 * 8))] ^ \
		t1_Vz[(uint8_t)((c) >> (7 * 8))]; \
	b *= mul;

#else /* for IA32 */

#define round(a,b,c,x,mul) \
	c ^= x; \
	a -= t1[(uint8_t)(c)] ^ \
		t2[(uint8_t)(((uint32_t)(c)) >> (2 * 8))] ^ \
		t3[(uint8_t)((c) >> (4 * 8))] ^ \
		t4[(uint8_t)(((uint32_t)((c) >> (4 * 8))) >> (2 * 8))] ; \
	b += t4[(uint8_t)(((uint32_t)(c)) >> (1 * 8))] ^ \
		t3[(uint8_t)(((uint32_t)(c)) >> (3 * 8))] ^ \
		t2[(uint8_t)(((uint32_t)((c) >> (4 * 8))) >> (1 * 8))] ^ \
		t1[(uint8_t)(((uint32_t)((c) >> (4 * 8))) >> (3 * 8))]; \
	b *= mul;

#define round_Vz(a,b,c,x,mul) \
	c ^= x; \
	a -= t1_Vz[(uint8_t)(c)] ^ \
		t2_Vz[(uint8_t)(((uint32_t)(c)) >> (2 * 8))] ^ \
		t3_Vz[(uint8_t)((c) >> (4 * 8))] ^ \
		t4_Vz[(uint8_t)(((uint32_t)((c) >> (4 * 8))) >> (2 * 8))] ; \
	b += t4_Vz[(uint8_t)(((uint32_t)(c)) >> (1 * 8))] ^ \
		t3_Vz[(uint8_t)(((uint32_t)(c)) >> (3 * 8))] ^ \
		t2_Vz[(uint8_t)(((uint32_t)((c) >> (4 * 8))) >> (1 * 8))] ^ \
		t1_Vz[(uint8_t)(((uint32_t)((c) >> (4 * 8))) >> (3 * 8))]; \
	b *= mul;
#endif /* CPU_X64 */

#define pass(a,b,c,mul) \
	round(a,b,c,x0,mul) \
	round(b,c,a,x1,mul) \
	round(c,a,b,x2,mul) \
	round(a,b,c,x3,mul) \
	round(b,c,a,x4,mul) \
	round(c,a,b,x5,mul) \
	round(a,b,c,x6,mul) \
	round(b,c,a,x7,mul)

#define pass_Vz(a,b,c,mul) \
	round_Vz(a,b,c,x0,mul) \
	round_Vz(b,c,a,x1,mul) \
	round_Vz(c,a,b,x2,mul) \
	round_Vz(a,b,c,x3,mul) \
	round_Vz(b,c,a,x4,mul) \
	round_Vz(c,a,b,x5,mul) \
	round_Vz(a,b,c,x6,mul) \
	round_Vz(b,c,a,x7,mul)

#define key_schedule { \
	x0 -= x7 ^ I64(0xA5A5A5A5A5A5A5A5); \
	x1 ^= x0; \
	x2 += x1; \
	x3 -= x2 ^ ((~x1)<<19); \
	x4 ^= x3; \
	x5 += x4; \
	x6 -= x5 ^ ((~x4)>>23); \
	x7 ^= x6; \
	x0 += x7; \
	x1 -= x0 ^ ((~x7)<<19); \
	x2 ^= x1; \
	x3 += x2; \
	x4 -= x3 ^ ((~x2)>>23); \
	x5 ^= x4; \
	x6 += x5; \
	x7 -= x6 ^ I64(0x0123456789ABCDEF); \
}

/* const */
#define key_schedule_Vz { \
	x0 -= x7 ^ I64(0xA5A5B5A5A5A5A7A5); \
	x1 ^= x0; \
	x2 += x1; \
	x3 -= x2 ^ ((~x1)<<19); \
	x4 ^= x3; \
	x5 += x4; \
	x6 -= x5 ^ ((~x4)>>23); \
	x7 ^= x6; \
	x0 += x7; \
	x1 -= x0 ^ ((~x7)<<19); \
	x2 ^= x1; \
	x3 += x2; \
	x4 -= x3 ^ ((~x2)>>23); \
	x5 ^= x4; \
	x6 += x5; \
	x7 -= x6 ^ I64(0x0123456789ABCDEF); \
}

/**
 * The core transformation. Process a 512-bit block.
 *
 * @param state the algorithm state
 * @param block the message block to process
 */
static void rhash_tiger_process_block(uint64_t state[3], uint64_t* block)
{
	/* Optimized for GCC IA32.
	   The order of declarations is important for compiler. */
	uint64_t a, b, c;
	uint64_t x0, x1, x2, x3, x4, x5, x6, x7;
#ifndef CPU_X64
	uint64_t tmp;
	char i;
#endif

	x0 = le2me_64(block[0]); x1 = le2me_64(block[1]);
	x2 = le2me_64(block[2]); x3 = le2me_64(block[3]);
	x4 = le2me_64(block[4]); x5 = le2me_64(block[5]);
	x6 = le2me_64(block[6]); x7 = le2me_64(block[7]);

	a = state[0];
	b = state[1];
	c = state[2];

	/* passes and key shedules */
#ifndef CPU_X64
	for(i = 0; i < 3; i++) {
		if(i != 0) key_schedule;
		pass(a, b, c, (i == 0 ? 5 : i == 1 ? 7 : 9));
		tmp = a;
		a = c;
		c = b;
		b = tmp;
	}
#else
	pass(a, b, c, 5);
	key_schedule;
	pass(c, a, b, 7);
	key_schedule;
	pass(b, c, a, 9);
#endif

	/* feedforward operation */
	state[0] = a ^ state[0];
	state[1] = b - state[1];
	state[2] = c + state[2];
}

/* modified */
static void rhash_tiger_process_block_Vz(uint64_t state[3], uint64_t* block)
{
	/* Optimized for GCC IA32.
	   The order of declarations is important for compiler. */
	uint64_t a, b, c;
	uint64_t x0, x1, x2, x3, x4, x5, x6, x7;
#ifndef CPU_X64
	uint64_t tmp;
	char i;
#endif

	x0 = le2me_64(block[0]); x1 = le2me_64(block[1]);
	x2 = le2me_64(block[2]); x3 = le2me_64(block[3]);
	x4 = le2me_64(block[4]); x5 = le2me_64(block[5]);
	x6 = le2me_64(block[6]); x7 = le2me_64(block[7]);

	a = state[0];
	b = state[1];
	c = state[2];

	/* passes and key shedules */
#ifndef CPU_X64
	for(i = 0; i < 3; i++) {
		if(i != 0) key_schedule_Vz;
		pass_Vz(a, b, c, (i == 0 ? 5 : i == 1 ? 7 : 9));
		tmp = a;
		a = c;
		c = b;
		b = tmp;
	}
#else
	pass_Vz(a, b, c, 5);
	key_schedule_Vz;
	pass_Vz(c, a, b, 7);
	key_schedule_Vz;
	pass_Vz(b, c, a, 9);
#endif

	/* feedforward operation */
	state[0] = a ^ state[0];
	state[1] = b - state[1];
	state[2] = c + state[2];
}

/**
 * Calculate message hash.
 * Can be called repeatedly with chunks of the message to be hashed.
 *
 * @param ctx the algorithm context containing current hashing state
 * @param msg message chunk
 * @param size length of the message chunk
 */
void rhash_tiger_update(tiger_ctx *ctx, const unsigned char* msg, size_t size)
{
	size_t index = (size_t)ctx->length & 63;
	ctx->length += size;

	/* fill partial block */
	if(index) {
		size_t left = tiger_block_size - index;
		if(size < left) {
			memcpy(ctx->message + index, msg, size);
			return;
		} else {
			memcpy(ctx->message + index, msg, left);
			rhash_tiger_process_block(ctx->hash, (uint64_t*)ctx->message);
			msg += left;
			size -= left;
		}
	}
	while(size >= tiger_block_size) {
		if(IS_ALIGNED_64(msg)) {
			/* the most common case is processing of an already aligned message
			without copying it */
			rhash_tiger_process_block(ctx->hash, (uint64_t*)msg);
		} else {
			memcpy(ctx->message, msg, tiger_block_size);
			rhash_tiger_process_block(ctx->hash, (uint64_t*)ctx->message);
		}

		msg += tiger_block_size;
		size -= tiger_block_size;
	}
	if(size) {
		/* save leftovers */
		memcpy(ctx->message, msg, size);
	}
}

/* modified */
void rhash_tiger_update_Vz(tiger_ctx *ctx, const unsigned char* msg, size_t size)
{
	size_t index = (size_t)ctx->length & 63;
	ctx->length += size;

	/* fill partial block */
	if(index) {
		size_t left = tiger_block_size - index;
		if(size < left) {
			memcpy(ctx->message + index, msg, size);
			return;
		} else {
			memcpy(ctx->message + index, msg, left);
			rhash_tiger_process_block_Vz(ctx->hash, (uint64_t*)ctx->message);
			msg += left;
			size -= left;
		}
	}
	while(size >= tiger_block_size) {
		if(IS_ALIGNED_64(msg)) {
			/* the most common case is processing of an already aligned message
			without copying it */
			rhash_tiger_process_block_Vz(ctx->hash, (uint64_t*)msg);
		} else {
			memcpy(ctx->message, msg, tiger_block_size);
			rhash_tiger_process_block_Vz(ctx->hash, (uint64_t*)ctx->message);
		}

		msg += tiger_block_size;
		size -= tiger_block_size;
	}
	if(size) {
		/* save leftovers */
		memcpy(ctx->message, msg, size);
	}
}

/**
 * Store calculated hash into the given array.
 *
 * @param ctx the algorithm context containing current hashing state
 * @param result calculated hash in binary form
 */
void rhash_tiger_final(tiger_ctx *ctx, unsigned char result[24])
{
	unsigned index = (unsigned)ctx->length & 63;
	uint64_t* msg64 = (uint64_t*)ctx->message;

	/* pad message and run for last block */

	/* append the byte 0x01 to the message */
	ctx->message[index++] = (ctx->tiger2 ? 0x80 : 0x01);

	/* if no room left in the message to store 64-bit message length */
	if(index > 56) {
		/* then fill the rest with zeros and process it */
		while(index < 64) {
			ctx->message[index++] = 0;
		}
		rhash_tiger_process_block(ctx->hash, msg64);
		index = 0;
	}
	while(index < 56) {
		ctx->message[index++] = 0;
	}
	msg64[7] = le2me_64(ctx->length << 3);
	rhash_tiger_process_block(ctx->hash, msg64);

	/* save result hash */
	le64_copy(result, 0, &ctx->hash, 24);
}

/* modified */
void rhash_tiger_final_Vz(tiger_ctx *ctx, unsigned char result[24])
{
	unsigned index = (unsigned)ctx->length & 63;
	uint64_t* msg64 = (uint64_t*)ctx->message;

	/* pad message and run for last block */

	/* append the byte 0x01 to the message */
	ctx->message[index++] = (ctx->tiger2 ? 0x80 : 0x01);

	/* if no room left in the message to store 64-bit message length */
	if(index > 56) {
		/* then fill the rest with zeros and process it */
		while(index < 64) {
			ctx->message[index++] = 0;
		}
		rhash_tiger_process_block_Vz(ctx->hash, msg64);
		index = 0;
	}
	while(index < 56) {
		ctx->message[index++] = 0;
	}
	msg64[7] = le2me_64(ctx->length << 3);
	rhash_tiger_process_block_Vz(ctx->hash, msg64);

	/* save result hash */
	le64_copy(result, 0, &ctx->hash, 24);
}
