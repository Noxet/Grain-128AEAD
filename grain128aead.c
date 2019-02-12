#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "grain128aead.h"

/*
 * Define "PRE" to print the pre-output instead of keystream.
 * Define "INIT" to also print the bits during the initialization part.
 * Do this either here or during compilation with -D flag.
 */

uint8_t init_rounds = 0;


void init_grain(grain_state *grain, uint8_t *key, uint8_t *iv)
{
	// expand the packed bytes and place one bit per array cell (like a flip flop in HW)
	for (int i = 0; i < 12; i++) {
		for (int j = 0; j < 8; j++) {
			grain->lfsr[8 * i + j] = (iv[i] & (1 << (7-j))) >> (7-j);

		}
	}

	for (int i = 96; i < 127; i++) {
		grain->lfsr[i] = 1;
	}

	grain->lfsr[127] = 0;

	for (int i = 0; i < 16; i++) {
		for (int j = 0; j < 8; j++) {
			grain->nfsr[8 * i + j] = (key[i] & (1 << (7-j))) >> (7-j);
		}
	}

	for (int i = 0; i < 64; i++) {
		grain->auth_acc[i] = 0;
		grain->auth_sr[i] = 0;
	}
}

void init_data(grain_data *data, uint8_t *msg, uint32_t msg_len)
{
	// allocate enough space for message, including the padding byte, 0x80
	data->message = (uint8_t *) calloc(8 * STREAM_BYTES + 8, 1);
	for (uint32_t i = 0; i < msg_len; i++) {
		data->message[i] = msg[i];
	}

	// always pad data with the byte 0x80
	data->message[msg_len] = 1;
}

uint8_t next_lfsr_fb(grain_state *grain)
{
 	/* f(x) = 1 + x^32 + x^47 + x^58 + x^90 + x^121 + x^128 */
	return grain->lfsr[96] ^ grain->lfsr[81] ^ grain->lfsr[70] ^ grain->lfsr[38] ^ grain->lfsr[7] ^ grain->lfsr[0];
}

uint8_t next_nfsr_fb(grain_state *grain)
{
	return grain->nfsr[96] ^ grain->nfsr[91] ^ grain->nfsr[56] ^ grain->nfsr[26] ^ grain->nfsr[0] ^ (grain->nfsr[84] & grain->nfsr[68]) ^
			(grain->nfsr[67] & grain->nfsr[3]) ^ (grain->nfsr[65] & grain->nfsr[61]) ^ (grain->nfsr[59] & grain->nfsr[27]) ^
			(grain->nfsr[48] & grain->nfsr[40]) ^ (grain->nfsr[18] & grain->nfsr[17]) ^ (grain->nfsr[13] & grain->nfsr[11]) ^
			(grain->nfsr[82] & grain->nfsr[78] & grain->nfsr[70]) ^ (grain->nfsr[25] & grain->nfsr[24] & grain->nfsr[22]) ^
			(grain->nfsr[95] & grain->nfsr[93] & grain->nfsr[92] & grain->nfsr[88]);
}

uint8_t next_h(grain_state *grain)
{
	// h(x) = x0x1 + x2x3 + x4x5 + x6x7 + x0x4x8
	#define x0 grain->nfsr[12]	// bi+12
	#define x1 grain->lfsr[8]		// si+8
	#define x2 grain->lfsr[13]	// si+13
	#define x3 grain->lfsr[20]	// si+20
	#define x4 grain->nfsr[95]	// bi+95
	#define x5 grain->lfsr[42]	// si+42
	#define x6 grain->lfsr[60]	// si+60
	#define x7 grain->lfsr[79]	// si+79
	#define x8 grain->lfsr[94]	// si+94

	uint8_t h_out = (x0 & x1) ^ (x2 & x3) ^ (x4 & x5) ^ (x6 & x7) ^ (x0 & x4 & x8);
	return h_out;
}

uint8_t shift(uint8_t fsr[128], uint8_t fb)
{
	uint8_t out = fsr[0];
	for (int i = 0; i < 127; i++) {
		fsr[i] = fsr[i+1];
	}
	fsr[127] = fb;

	return out;
}

void auth_shift(uint8_t sr[64], uint8_t fb)
{
	for (int i = 0; i < 63; i++) {
		sr[i] = sr[i+1];
	}
	sr[63] = fb;
}

void accumulate(grain_state *grain)
{
	for (int i = 0; i < 64; i++) {
		grain->auth_acc[i] ^= grain->auth_sr[i];
	}
}

uint8_t next_z(grain_state *grain)
{
	uint8_t lfsr_fb = next_lfsr_fb(grain);
	uint8_t nfsr_fb = next_nfsr_fb(grain);
	uint8_t h_out = next_h(grain);

	/* y = h + s_{i+93} + sum(b_{i+j}), j \in A */
	uint8_t A[] = {2, 15, 36, 45, 64, 73, 89};

	uint8_t nfsr_tmp = 0;
	for (int i = 0; i < 7; i++) {
		nfsr_tmp ^= grain->nfsr[A[i]];
	}

	uint8_t y = h_out ^ grain->lfsr[93] ^ nfsr_tmp;
	
	uint8_t lfsr_out;

	/* feedback y if we are in the initialization instance */
	if (init_rounds) {
		lfsr_out = shift(grain->lfsr, lfsr_fb ^ y);
		shift(grain->nfsr, nfsr_fb ^ lfsr_out ^ y);
	} else {
		lfsr_out = shift(grain->lfsr, lfsr_fb);
		shift(grain->nfsr, nfsr_fb ^ lfsr_out);
	}

	return y;
}

void print_state(grain_state *grain)
{
	printf("LFSR: ");
	for (int i = 0; i < 128; i++) {
		printf("%d", grain->lfsr[i]);
	}
	printf("\nNFSR: ");
	for (int i = 0; i < 128; i++) {
		printf("%d", grain->nfsr[i]);
	}
	printf("\n");
}

void print_stream(uint8_t *stream, uint8_t byte_size)
{
	for (int i = 0; i < byte_size; i++) {
		uint8_t yi = 0;
		for (int j = 0; j < 8; j++) {
			yi = (yi << 1) ^ stream[i * 8 + j];
		}
		printf("%02x", yi);
	}
	printf("\n");
}

void generate_keystream(grain_state *grain, grain_data *data)
{
	/* inititalize the accumulator and shift reg. using the first 64 bits */
	for (int i = 0; i < 64; i++) {
		grain->auth_acc[i] = next_z(grain);
	}

	for (int i = 0; i < 64; i++) {
		grain->auth_sr[i] = next_z(grain);
	}

	printf("accumulator: ");
	print_stream(grain->auth_acc, 8);

	printf("shift register: ");
	print_stream(grain->auth_sr, 8);

	uint8_t ks[STREAM_BYTES * 8];		// keystream array
	uint16_t ks_cnt = 0;
	uint8_t ms[STREAM_BYTES * 8];		// macstream array
	uint16_t ms_cnt = 0;
	uint8_t pre[2 * STREAM_BYTES * 8];	// pre-output
	uint16_t pre_cnt = 0;

	/* generate keystream */
	for (int i = 0; i < STREAM_BYTES; i++) {
		/* every second bit is used for keystream, the others for MAC */
		for (int j = 0; j < 16; j++) {
			uint8_t z_next = next_z(grain);
			if (j % 2 == 0) {
				ks[ks_cnt] = z_next;
				ks_cnt++;
			} else {
				ms[ms_cnt] = z_next;
				if (data->message[ms_cnt] == 1) {
					accumulate(grain);
				}
				auth_shift(grain->auth_sr, z_next);
				ms_cnt++;
			}
			pre[pre_cnt++] = z_next;	// pre-output includes all bits
		}
	}

	printf("pre-output: ");
	print_stream(pre, 2 * STREAM_BYTES);

	printf("keystream: ");
	print_stream(ks, STREAM_BYTES);

	printf("macstream: ");
	print_stream(ms, STREAM_BYTES);

	printf("tag: ");
	print_stream(grain->auth_acc, 8);
}


int main()
{
	grain_state grain;
	grain_data data;
	
	//uint8_t key[] = {0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef, 0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0xde, 0xf0};
	//uint8_t iv[] = {0x81, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef, 0x12, 0x34, 0x56, 0x78};

	uint8_t key[16] = { 0 };
	uint8_t iv[12] = { 0 };
	//iv[0] = 0x80;

	uint8_t msg[0];

	init_grain(&grain, key, iv);
	init_data(&data, msg, sizeof(msg));
	
	/* initialize grain and skip output */
	init_rounds = 1;
#ifdef INIT
	printf("init bits: ");
#endif
	for (int i = 0; i < 256; i++) {
#ifdef INIT
		printf("%d", next_z(&grain));
#else
		next_z(&grain);
#endif
	}
#ifdef INIT
	printf("\n");
#endif
	init_rounds = 0;

	generate_keystream(&grain, &data);

	free(data.message);

	return 0;
}
