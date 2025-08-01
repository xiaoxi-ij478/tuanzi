/* torrent.h */
#ifndef TORRENT_H
#define TORRENT_H
#include "ustd.h"
#include "sha1.h"

#ifdef __cplusplus
extern "C" {
#endif

#define btih_hash_size  20

/* vector structure */
typedef struct torrent_vect {
	void **array;     /* array of elements of the vector */
	size_t size;      /* vector size */
	size_t allocated; /* number of allocated elements */
} torrent_vect;

/* BitTorrent algorithm context */
typedef struct torrent_ctx
{
	unsigned char btih[20]; /* resulting BTIH hash sum */
	unsigned options;       /* algorithm options */
	sha1_ctx sha1_context;  /* context for hashing current file piece */
#if defined(USE_OPENSSL) || defined(OPENSSL_RUNTIME)
	unsigned long reserved; /* need more space for OpenSSL SHA1 context */
	void *sha_init, *sha_update, *sha_final;
#endif
	size_t index;             /* byte index in the current piece */
	size_t piece_length;      /* length of a torrent file piece */
	size_t piece_count;       /* the number of pieces processed */
	torrent_vect hash_blocks; /* array of blocks storing SHA1 hashes */
	torrent_vect files;       /* names of files in a torrent batch */
	char* program_name;       /* the name of the program */
	char* announce;           /* announce URL */

	char*  torrent_str;       /* content of generated torrent file */
	size_t torrent_length;    /* length of generated torrent file */
	size_t torrent_allocated; /* bytes allocated for torrent file */
	int error; /* non-zero if error occurred, zero otherwise */
} torrent_ctx;

void bt_init(torrent_ctx *ctx);
void bt_update(torrent_ctx *ctx, const void* msg, size_t size);
void bt_final(torrent_ctx *ctx, unsigned char result[20]);
void bt_cleanup(torrent_ctx *ctx);

size_t bt_get_text(torrent_ctx *ctx, char** pstr);
unsigned char* bt_get_btih(torrent_ctx *ctx);

/* possible options */
#define BT_OPT_PRIVATE 1
#define BT_OPT_INFOHASH_ONLY 2

void bt_set_options(torrent_ctx *ctx, unsigned options);
int  bt_add_file(torrent_ctx *ctx, const char* path, uint64_t filesize);
int  bt_set_announce(torrent_ctx *ctx, const char* announce_url);
int  bt_set_program_name(torrent_ctx *ctx, const char* name);
void bt_set_piece_length(torrent_ctx *ctx, size_t piece_length);
size_t bt_default_piece_length(uint64_t total_size);

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* TORRENT_H */
