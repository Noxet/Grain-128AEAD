/*
 * The reference implementation of Grain128-AEAD.
 * 
 * Note that this implementation is *not* meant to be run in software.
 * It is written to be as close to a hardware implementation as possible,
 * hence it should not be used in benchmarks due to bad performance.
 *
 * Jonathan Sönnerup
 * 2019
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "grain128aead.h"

uint8_t grain_round;

void init_grain(grain_state *grain, const unsigned char *key, const unsigned char *iv)
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

void init_data(grain_data *data, const unsigned char *msg, unsigned long long msg_len)
{
	// allocate enough space for message, including the padding bit 1 (byte 0x80)
	data->message = (uint8_t *) calloc(8 * msg_len + 1, 1);
	data->msg_len = 8 * msg_len + 1;
	for (unsigned long long i = 0; i < msg_len; i++) {
		for (int j = 0; j < 8; j++) {
			data->message[8 * i + j] = (msg[i] & (1 << (7-j))) >> (7-j);
		}
	}

	// always pad data with the bit 1 (byte 0x80)
	data->message[data->msg_len - 1] = 1;
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

uint8_t next_z(grain_state *grain, uint8_t keybit)
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
	if (grain_round == INIT) {
		lfsr_out = shift(grain->lfsr, lfsr_fb ^ y);
		shift(grain->nfsr, nfsr_fb ^ lfsr_out ^ y);
	} else if (grain_round == FP1) {
		lfsr_out = shift(grain->lfsr, lfsr_fb ^ keybit);
		shift(grain->nfsr, nfsr_fb ^ lfsr_out);
	} else if (grain_round == NORMAL) {
		lfsr_out = shift(grain->lfsr, lfsr_fb);
		shift(grain->nfsr, nfsr_fb ^ lfsr_out);
	}

	return y;
}

int crypto_aead_encrypt(unsigned char *c, unsigned long long *clen,
	const unsigned char *m, unsigned long long mlen,
	const unsigned char *ad, unsigned long long adlen,
	const unsigned char *nsec,
	const unsigned char *npub,
	const unsigned char *k
	)
{
	grain_state grain;
	grain_data data;

	init_grain(&grain, k, npub);
	init_data(&data, m, mlen);

	*clen = 0;

	/* initialize grain and skip output */
	grain_round = INIT;
	for (int i = 0; i < 256; i++) {
		next_z(&grain, 0);
	}
	
	grain_round = FP1;

	uint8_t key_idx = 0;
	/* inititalize the accumulator and shift reg. using the first 64 bits */
	for (int i = 0; i < 8; i++) {
		for (int j = 0; j < 8; j++) {
			uint8_t fp1_fb = (k[key_idx] & (1 << (7-j))) >> (7-j);
			grain.auth_acc[8 * i + j] = next_z(&grain, fp1_fb);
		}
		key_idx++;
	}

	for (int i = 0; i < 8; i++) {
		for (int j = 0; j < 8; j++) {
			uint8_t fp1_fb = (k[key_idx] & (1 << (7-j))) >> (7-j);
			grain.auth_sr[8 * i + j] = next_z(&grain, fp1_fb);
		}
		key_idx++;
	}

	grain_round = NORMAL;

	unsigned long long ad_cnt = 0;
	unsigned char adval = 0;

	/* accumulate tag for associated data only */
	for (unsigned long long i = 0; i < adlen; i++) {
		/* every second bit is used for keystream, the others for MAC */
		for (int j = 0; j < 16; j++) {
			uint8_t z_next = next_z(&grain, 0);
			if (j % 2 == 0) {
				// do not encrypt
			} else {
				adval = ad[ad_cnt / 8] & (1 << (7 - (ad_cnt % 8)));
				if (adval == 1) {
					accumulate(&grain);
				}
				auth_shift(grain.auth_sr, z_next);
				ad_cnt++;
			}
		}
	}

	unsigned long long ac_cnt = 0;
	unsigned long long m_cnt = 0;
	unsigned long long c_cnt = 0;
	unsigned char cc = 0;

	// generate keystream for message
	for (unsigned long long i = 0; i < mlen; i++) {
		// every second bit is used for keystream, the others for MAC
		cc = 0;
		for (int j = 0; j < 16; j++) {
			uint8_t z_next = next_z(&grain, 0);
			if (j % 2 == 0) {
				// transform it back to 8 bits per byte
				cc |= (data.message[m_cnt++] ^ z_next) << (7 - (c_cnt % 8));
				c_cnt++;
			} else {
				if (data.message[ac_cnt++] == 1) {
					accumulate(&grain);
				}
				auth_shift(grain.auth_sr, z_next);
			}
		}
		c[i] = cc;
		*clen += 1;
	}
	
	// generate unused keystream bit
	next_z(&grain, 0);
	// the 1 in the padding means accumulation
	accumulate(&grain);

	/* append MAC to ciphertext */
	unsigned long long acc_idx = 0;
	for (unsigned long long i = mlen; i < mlen + 8; i++) {
		unsigned char acc = 0;
		// transform back to 8 bits per byte
		for (int j = 0; j < 8; j++) {
			acc |= grain.auth_acc[8 * acc_idx + j] << (7 - j);
		}
		c[i] = acc;
		acc_idx++;
		*clen += 1;
	}

	free(data.message);

	return 0;
}

int crypto_aead_decrypt(
       unsigned char *m,unsigned long long *mlen,
       unsigned char *nsec,
       const unsigned char *c,unsigned long long clen,
       const unsigned char *ad,unsigned long long adlen,
       const unsigned char *npub,
       const unsigned char *k
     )
{
	// mtmp will contain unwanted tag for ciphertext
	unsigned char *mtmp = (unsigned char *) malloc(clen);
	crypto_aead_encrypt(mtmp, mlen, c, clen - 8, ad, adlen, nsec, npub, k);

	*mlen -= 8; // remove length of tag

	// copy only plaintext
	for (unsigned long long i = 0; i < *mlen; i++) {
		m[i] = mtmp[i];
	}

	free(mtmp);
	return 0;
}
