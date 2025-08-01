#include "all.h"
#include "compressor.h"

//static const char *in_pos  = nullptr;       /* orig name: pInPos     */
//static const char *in_mem  = nullptr;       /* orig name: pInputMem  */
//static       char *out_pos = nullptr;       /* orig name: pOutPos    */
//static       char *out_mem = nullptr;       /* orig name: pOutputMem */
static char pc_table[32768] = {}; /* orig name: pcTable    */

//static char mgetc()
//{
//    return *in_pos++;
//}
//
//static void mputc(char c)
//{
//    *out_pos++ = c;
//}
/*
 * the compressed block is 1 byte (master byte) declaring which byte is in the compress table
 * and 8 or less bytes of the uncompressed data
 * every bit of master byte means if the following byte is uncompressed
 * if it is 0, then read the next char, and store into the pc_table with index of
 * the previous char and the previous previous char shift left by 7, then XOR
 * if it is 1, then read the char from the pc_table with index of
 * the previous char and the previous previous char shift left by 7, then XOR
 */

unsigned Decompress(
    const char *in,
    char *out,
    unsigned in_len,
    unsigned out_len
)
{
    unsigned in_pos = 0, out_pos = 0;
    unsigned uncompressed_size = 0;
    char prev_prev_char = 0, prev_char = 0, cur_char = 0;
    char master_byte = 0;
    memset(pc_table, ' ', sizeof(pc_table));

    while (in_pos < in_len) {
        master_byte = in[in_pos++];

        for (unsigned j = 0; j < 8; j++) {
            if (master_byte & 1)
                cur_char = pc_table[prev_char ^ (prev_prev_char << 7)];

            else {
                if (in_pos >= in_len)
                    return uncompressed_size;

                pc_table[prev_char ^ (prev_prev_char << 7)] = cur_char = in[in_pos++];
            }

            uncompressed_size++;

            if (out_len) {
                out[out_pos++] = cur_char;

                if (out_pos >= out_len)
                    return uncompressed_size;
            }

            master_byte >>= 1;
            prev_prev_char = prev_char;
            prev_char = cur_char;
        }
    }

    return uncompressed_size;
}

unsigned Compress(
    const char *in,
    char *out,
    unsigned in_len,
    unsigned out_len
)
{
    unsigned in_pos = 0, out_pos = 0;
    unsigned buffer_index = 0;
    char prev_prev_char = 0, prev_char = 0, cur_char = 0;
    char master_byte = 0;
    char buffer[8] = {};
    unsigned compressed_size = 0;
    memset(pc_table, ' ', sizeof(pc_table));

    while (in_pos < in_len) {
        for (unsigned j = 0; j < 8; j++) {
            cur_char = in[in_pos++];

            if (pc_table[prev_char ^ (prev_prev_char << 7)] == cur_char)
                master_byte |= 1 << j;

            else
                buffer[buffer_index++] =
                    pc_table[prev_char ^ (prev_prev_char << 7)] = cur_char;

            prev_prev_char = prev_char;
            prev_char = cur_char;

            if (in_pos >= in_len)
                break;
        }

        compressed_size += 1 + buffer_index;

        if (out_len) {
            out[out_pos++] = master_byte;

            if (out_pos >= out_len)
                return out_pos;

            for (unsigned j = 0; j < buffer_index; j++) {
                out[out_pos++] = buffer[j];

                if (out_pos >= out_len)
                    return out_pos;
            }
        }

        buffer_index = 0;
        master_byte = 0;
    }

    return compressed_size;
}
