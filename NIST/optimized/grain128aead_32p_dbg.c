
#include <inttypes.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h> // TODO: remove

#include "grain128aead_32p.h"

u8 swapsb(u8 n);

static const uint32_t mvo0 = 0x22222222;
static const uint32_t mvo1 = 0x18181818;
static const uint32_t mvo2 = 0x07800780;
static const uint32_t mvo3 = 0x007f8000;
static const uint32_t mvo4 = 0x80000000;

static const uint32_t mve0 = 0x44444444;
static const uint32_t mve1 = 0x30303030;
static const uint32_t mve2 = 0x0f000f00;
static const uint32_t mve3 = 0x00ff0000;


void print_state(grain_ctx *grain)
{
	u8 *nfsr = (u8 *) grain->nptr;
	u8 *lfsr = (u8 *) grain->lptr;
	
	/*printf("NFSR lsb: ");
	for (int i = 0; i < 16; i++) {
		u8 n = 0;
		// print with LSB first
		for (int j = 0; j < 8; j++) {
			n |= (((*(nfsr + i)) >> j) & 1) << (7-j);
		}
		printf("%02x", n);
	}
	*/
	printf("\nNFSR msb: ");
	for (int i = 0; i < 16; i++) {
		printf("%02x", *(nfsr + i));
	}

	/*
	printf("\nLFSR lsb: ");
	for (int i = 0; i < 16; i++) {
		u8 l = 0;
		for (int j = 0; j < 8; j++) {
			l |= (((*(lfsr + i)) >> j) & 1) << (7-j);
		}
		printf("%02x", l);
	}
	*/

	printf("\nLFSR msb: ");
	for (int i = 0; i < 16; i++) {
		printf("%02x", *(lfsr + i));
	}

	/*
	printf("\nACC lsb:  ");
	u64 a = 0;
	for (int i = 0; i < 64; i++) {
		a |= ((grain->acc >> i) & 1) << (63-i);
	}
	printf("%016lx", a);
	*/

	printf("\nACC msb: ");
	for (int i = 0; i < 8; i++) {
		printf("%02x", (u8) (grain->acc >> (8 * i)));
	}

	//printf("\nACC msb: %016lx\n", grain->acc);

	/*
	printf("\nREG lsb:  ");
	u64 r = 0;
	for (int i = 0; i < 64; i++) {
		r |= ((grain->reg >> i) & 1) << (63-i);
	}
	printf("%016lx", r);
	*/

	printf("\nREG msb: ");
	for (int i = 0; i < 8; i++) {
		printf("%02x", (u8) (grain->reg >> (8 * i)));
	}

	/*
	for (int i = 0; i < 8; i++) {
		u8 a = 0;
		for (int j = 0; j < 8; j++) {
			a |= (((*(acc + i)) >> j) & 1) << (7-j);
		}
		printf("%02x", a);
	}
	printf("\nREG lsb:  ");
	for (int i = 0; i < 8; i++) {
		u8 r = 0;
		for (int j = 0; j < 8; j++) {
			r |= (((*(reg + i)) >> j) & 1) << (7-j);
		}
		printf("%02x", r);
	}
	*/
	printf("\n\n");
}

void print_ks(u32 ks)
{
	u32 n = 0;
	for (int i = 0; i < 32; i++) {
		n |= ((ks >> i) & 1) << (31-i);
	}
	printf("%08x", n);
}

void print_ks16(u16 ks)
{
	u16 n = 0;
	for (int i = 0; i < 16; i++) {
		n |= ((ks >> i) & 1) << (15-i);
	}
	printf("%04x\n", n);
}

void print_16(u16 ks)
{
	u16 n = 0;
	for (int i = 0; i < 16; i++) {
		n |= ((ks >> i) & 1) << (15-i);
	}
	printf("%04x\n", n);
}

void print_32(u32 ks)
{
	u32 n = 0;
	for (int i = 0; i < 32; i++) {
		n |= ((ks >> i) & 1) << (31-i);
	}
	printf("%08x\n", n);
}

void print_64(u64 ks)
{
	u64 n = 0;
	for (int i = 0; i < 64; i++) {
		n |= ((ks >> i) & 1) << (63-i);
	}
	printf("%016lx\n", n);
}

u32 next_keystream(grain_ctx *grain)
{
	u64 ln0 = (((u64) *(grain->lptr + 1)) << 32) | *(grain->lptr),
	    ln1 = (((u64) *(grain->lptr + 2)) << 32) | *(grain->lptr + 1),
	    ln2 = (((u64) *(grain->lptr + 3)) << 32) | *(grain->lptr + 2),
	    ln3 = (((u64) *(grain->lptr + 3)));
	u64 nn0 = (((u64) *(grain->nptr + 1)) << 32) | *(grain->nptr),
	    nn1 = (((u64) *(grain->nptr + 2)) << 32) | *(grain->nptr + 1),
	    nn2 = (((u64) *(grain->nptr + 3)) << 32) | *(grain->nptr + 2),
	    nn3 = (((u64) *(grain->nptr + 3)));

	// f
	grain->lfsr[grain->count] = (ln0 ^ ln3) ^ ((ln1 ^ ln2) >> 6) ^ (ln0 >> 7) ^ (ln2 >> 17);

	// g                        s0    b0        b26       b96       b56             b91 + b27b59
	grain->nfsr[grain->count] = ln0 ^ nn0 ^ (nn0 >> 26) ^ nn3 ^ (nn1 >> 24) ^ (((nn0 & nn1) ^ nn2) >> 27) ^
				//     b3b67                   b11b13                        b17b18
				((nn0 & nn2) >> 3) ^ ((nn0 >> 11) & (nn0 >> 13)) ^ ((nn0 >> 17) & (nn0 >> 18)) ^
				//       b40b48                        b61b65                      b68b84
				((nn1 >> 8) & (nn1 >> 16)) ^ ((nn1 >> 29) & (nn2 >> 1)) ^ ((nn2 >> 4) & (nn2 >> 20)) ^
				//                   b88b92b93b95
				((nn2 >> 24) & (nn2 >> 28) & (nn2 >> 29) & (nn2 >> 31)) ^
				//              b22b24b25                                  b70b78b82
				((nn0 >> 22) & (nn0 >> 24) & (nn0 >> 25)) ^ ((nn2 >> 6) & (nn2 >> 14) & (nn2 >> 18));
	
	grain->count++;
	grain->lptr++;
	grain->nptr++;

	// move the state to the beginning of the buffers
	if (grain->count >= BUF_SIZE) grain_reinit(grain);

	return (nn0 >> 2) ^ (nn0 >> 15) ^ (nn1 >> 4) ^ (nn1 >> 13) ^ nn2 ^ (nn2 >> 9) ^ (nn2 >> 25) ^ (ln2 >> 29) ^
		((nn0 >> 12) & (ln0 >> 8)) ^ ((ln0 >> 13) & (ln0 >> 20)) ^ ((nn2 >> 31) & (ln1 >> 10)) ^
		((ln1 >> 28) & (ln2 >> 15)) ^ ((nn0 >> 12) & (nn2 >> 31) & (ln2 >> 30));
}

void auth_accumulate(grain_ctx *grain, u16 ms, u16 msg)
{
	/* updates the authentication module using the 
	 * MAC stream (ms) and the plaintext (msg)
	 */
	u16 mstmp = ms;
	u16 acctmp = 0;
	u32 regtmp = (u32) ms << 16;

	for (int i = 0; i < 16; i++) {
		u64 mask = 0x00;
		u32 mask_rem = 0x00;
		if (msg & 0x0001) {
			mask = ~mask; // all ones
			mask_rem = 0x0000ffff;
		}

		grain->acc ^= grain->reg & mask;
		grain->reg >>= 1;

		acctmp ^= regtmp & mask_rem;
		regtmp >>= 1;

		mstmp >>= 1;

		msg >>= 1;
	}

	grain->reg |= ((u64) ms << 48);
	grain->acc ^= ((u64) acctmp << 48);

}

void auth_accumulate8(grain_ctx *grain, u8 ms, u8 msg)
{
	/* updates the authentication module using the 
	 * MAC stream (ms) and the plaintext (msg)
	 */
	u8 mstmp = ms;
	u8 acctmp = 0;
	u16 regtmp = (u16) ms << 8;

	for (int i = 0; i < 8; i++) {
		u64 mask = 0x00;
		u32 mask_rem = 0x00;
		if (msg & 0x01) {
			mask = ~mask; // all ones
			mask_rem = 0x00ff;
		}

		grain->acc ^= grain->reg & mask;
		grain->reg >>= 1;

		acctmp ^= regtmp & mask_rem;
		regtmp >>= 1;

		mstmp >>= 1;

		msg >>= 1;
	}

	grain->reg |= ((u64) ms << 56);
	grain->acc ^= ((u64) acctmp << 56);

}

void grain_init(grain_ctx *grain, const u8 *key, const u8 *iv)
{
	
	// load key, and IV along with padding
	memcpy(grain->nfsr, key, 16);
	memcpy(grain->lfsr, iv, 12);
	*(u32 *) (grain->lfsr + 3) = (u32) 0x7fffffff; // 0xfffffffe in little endian, LSB first


	grain->count = 4;
	grain->nptr = grain->nfsr;
	grain->lptr = grain->lfsr;
	
	printf("PRE INIT\n");
	print_state(grain);

	//printf("pre-init:\n");
	//print_state(grain);

	//printf("init:\n");
	register u32 ks;
	for (int i = 0; i < 8; i++) {
		ks = next_keystream(grain);
		grain->nfsr[i + 4] ^= ks;
		grain->lfsr[i + 4] ^= ks;
		//print_state(grain);
	}

	// add the key in the feedback, "FP(1)" and initialize auth module
	//printf("FP(1)\n");
	grain->acc = 0;
	for (int i = 0; i < 2; i++) {
		// initialize accumulator
		ks = next_keystream(grain);
		// TODO: fix aliasing rules
		grain->acc |= ((u64) ks << (32 * i));
		grain->lfsr[i + 12] ^= *(u32 *) (key + 4 * i);
		//print_state(grain);
	}

	grain->reg = 0;
	for (int i = 0; i < 2; i++) {
		// initialize register
		ks = next_keystream(grain);
		// TODO: fix aliasing rules
		grain->reg |= ((u64) ks << (32 * i));
		grain->lfsr[i + 14] ^= *(u32 *) (key + 8 + 4 * i);
	//	print_state(grain);
	}

	printf("POST INIT\n");
	print_state(grain);
}

void grain_reinit(grain_ctx *grain)
{
	printf("REINIT\n");
	// TODO: fix aliasing rules
	*(u32 *) (grain->lfsr) = *(u32 *) (grain->lptr);
	*(u32 *) (grain->lfsr + 1) = *(u32 *) (grain->lptr + 1);
	*(u32 *) (grain->lfsr + 2) = *(u32 *) (grain->lptr + 2);
	*(u32 *) (grain->lfsr + 3) = *(u32 *) (grain->lptr + 3);

	*(u32 *) (grain->nfsr + 0) = *(u32 *) (grain->nptr + 0);
	*(u32 *) (grain->nfsr + 1) = *(u32 *) (grain->nptr + 1);
	*(u32 *) (grain->nfsr + 2) = *(u32 *) (grain->nptr + 2);
	*(u32 *) (grain->nfsr + 3) = *(u32 *) (grain->nptr + 3);

	grain->lptr = grain->lfsr;
	grain->nptr = grain->nfsr;
	grain->count = 4;
}

uint16_t getmb(u32 num)
{
	// compress x using the mask 0xAAAAAAAA to extract the (odd) MAC bits, LSB first
	register u32 t;
	register u32 x = num & 0xAAAAAAAA;
	t = x & mvo0; x = (x ^ t) | (t >> 1);
	t = x & mvo1; x = (x ^ t) | (t >> 2);
	t = x & mvo2; x = (x ^ t) | (t >> 4);
	t = x & mvo3; x = (x ^ t) | (t >> 8);
	t = x & mvo4; x = (x ^ t) | (t >> 16);

	return (u16) x;
}

uint16_t getkb(u32 num)
{
	// compress x using the mask 0x55555555 to extract the (even) key bits, LSB first
	u32 t;
	u32 x = num & 0x55555555;
	t = x & mve0; x = (x ^ t) | (t >> 1);
	t = x & mve1; x = (x ^ t) | (t >> 2);
	t = x & mve2; x = (x ^ t) | (t >> 4);
	t = x & mve3; x = (x ^ t) | (t >> 8);

	return (u16) x;
}

int encode_der(unsigned long long len, u8 **der)
{
	unsigned long long len_tmp;
	int der_len = 0;

	if (len < 128) {
		*der = malloc(1);
		(*der)[0] = len;
		return 1;
	}

	len_tmp = len;
	do {
		len_tmp >>= 8;
		der_len++;
	} while (len != 0);

	// one extra byte to describe the number of bytes used
	*der = malloc(der_len + 1);
	(*der)[0] = 0x80 | der_len;

	len_tmp = len;
	for (int i = der_len; i > 0; i--) {
		(*der)[i] = len_tmp & 0xff;	// mod 256
		len_tmp >>= 8;
	}

	return der_len + 1;
}


int crypto_aead_encrypt(
	unsigned char *c, unsigned long long *clen,
	const unsigned char *m, unsigned long long mlen,
	const unsigned char *ad, unsigned long long adlen,
	const unsigned char *nsec,
	const unsigned char *npub,
	const unsigned char *k)
{
	grain_ctx grain;
	grain_init(&grain, k, npub);

	*clen = 0;
	unsigned long long mmlen = mlen;

	printf("ENCRYPT\n");
	printf("MLEN: %lld\n", mlen);

	printf("PT: ");
	for (int i = 0; i < mlen; i++) {
		printf("%02x", m[i]);
	}
	printf("\n");

	// authenticate length of AD
	// encode length using DER
	u8 *ader;
	int aderlen = encode_der(adlen, &ader);
	ader = realloc(ader, aderlen + adlen);
	memcpy(ader + aderlen, ad, adlen);

	printf("ADER: ");
	for (int i = 0; i < adlen; i++) {
		printf("%02x", ad[i]);
	}
	printf("\n\n");

	/*
	printf("ks: ");
	for (int i = 0; i < 12; i++) {
		u32 ks = next_keystream(&grain);
		print_ks(ks);
		//print_state(&grain);
	}
	printf("\n");
	*/

	unsigned long long itr = (aderlen + adlen) / 2;
	unsigned long long rem = (aderlen + adlen) % 2;
	printf("ad itr: %lld\nad rem: %lld\n", itr, rem);
	unsigned long long j = 0;
	u32 next;
	u32 rem_word;

	// authenticate AD
	for (unsigned long long i = 0; i < itr; i++) {
		printf("ACC\n");
		next = next_keystream(&grain);
		auth_accumulate(&grain, getmb(next), *(u16 *) (ader + j));
		j += 2;;
	}

	if (rem) {
		printf("ACC REM\n");
		rem_word = next_keystream(&grain);
		auth_accumulate8(&grain, getmb(rem_word), *(ader + j));
	}

	free(ader);

	j = 0;

	// use the last 8 bits in rem_word for the message
	if (rem && mlen) {
		printf("\nFIXFIX\n");
		*(c + j) = ((u8) (getkb(rem_word) >> 8)) ^ *(m + j);
		auth_accumulate8(&grain, getmb(rem_word) >> 8, *(m + j));
		*clen += 1;
		j++;
		mmlen--; // one byte processed
	}


	itr = mmlen / 2;
	rem = mmlen % 2;
	printf("m itr: %lld\nm rem: %lld\n", itr, rem);

	// encrypt and authenticate message
	for (unsigned long long i = 0; i < itr; i++) {
		printf("ENC and ACC\n");
		next = next_keystream(&grain);
		*(u16 *) (c + j) = getkb(next) ^ (*(u16 *) (m + j));
		auth_accumulate(&grain, getmb(next), *(u16 *) (m + j));
		j += 2;
		*clen += 2;
	}

	rem_word = next_keystream(&grain);
	if (rem) {
		printf("ENC and ACC REM\n");
		*(c + j) = ((u8) (getkb(rem_word))) ^ *(m + j);
		// add padding to the last byte of plaintext
		auth_accumulate(&grain, getmb(rem_word), 0x0100 | *(m + j));
		*clen += 1;
	} else {
		auth_accumulate(&grain, getmb(rem_word), 0x01);
	}
	
	// append MAC to ciphertext
	memcpy(c + mlen, &grain.acc, 8);
	//*(u64 *) (c + mlen) = grain.acc;

	*clen += 8;

	print_state(&grain);
	printf("--------------------\n");
	return 0;
}

int crypto_aead_decrypt(
	unsigned char *m, unsigned long long *mlen,
	unsigned char *nsec,
	const unsigned char *c, unsigned long long clen,
	const unsigned char *ad, unsigned long long adlen,
	const unsigned char *npub,
	const unsigned char *k
)
{
	*mlen = clen - 8;
	printf("DECRYPT\n");
	grain_ctx grain;
	grain_init(&grain, k, npub);

	printf("init clen: %lld\n", clen);

	// length of ciphertext, no tag
	unsigned long long cclen = clen - 8;
	*mlen = 0;

	// authenticate length of AD
	// encode length using DER
	u8 *ader;
	int aderlen = encode_der(adlen, &ader);
	ader = realloc(ader, aderlen + adlen);
	memcpy(ader + aderlen, ad, adlen);

	unsigned long long itr = (aderlen + adlen) / 2;
	unsigned long long rem = (aderlen + adlen) % 2;
	unsigned long long j = 0;
	u32 next;
	u32 rem_word;

	// authenticate AD
	for (unsigned long long i = 0; i < itr; i++) {
		printf("ACC\n");
		next = next_keystream(&grain);
		auth_accumulate(&grain, getmb(next), *(u16 *) (ader + j));
		j += 2;
	}

	if (rem) {
		printf("ACC REM\n");
		rem_word = next_keystream(&grain);
		auth_accumulate8(&grain, getmb(rem_word), *(ader + j));
	}

	free(ader);

	j = 0;
	printf("cclen: %d\n", cclen);

	// use the last 8 bits in rem_word for the message
	if (rem && cclen) {
		printf("\nFIXFIX\n");
		*(m + j) = ((u8) (getkb(rem_word) >> 8)) ^ *(c + j);
		auth_accumulate8(&grain, getmb(rem_word) >> 8, *(m + j));
		j++;
		cclen--;
		*mlen += 1;
	}


	itr = cclen / 2;
	rem = cclen % 2;


	// encrypt and authenticate message
	for (unsigned long long i = 0; i < itr; i++) {
		printf("DEC and ACC\n");
		next = next_keystream(&grain);
		*(u16 *) (m + j) = getkb(next) ^ (*(u16 *) (c + j));
		auth_accumulate(&grain, getmb(next), *(u16 *) (m + j));
		j += 2;
		*mlen += 2;
	}

	rem_word = next_keystream(&grain);
	if (rem) {
		printf("DEC and ACC REM\n");
		*(m + j) = ((u8) (getkb(rem_word))) ^ *(c + j);
		// add padding to the last byte of plaintext
		auth_accumulate(&grain, getmb(rem_word), 0x0100 | *(m + j));
		*mlen += 1;
	} else {
		auth_accumulate(&grain, getmb(rem_word), 0x01);
	}
	

	print_state(&grain);
	printf("--------------------\n");
	return 0;
}

u8 swapsb(u8 n)
{
	// swaps significant bit
	u8 val = 0;
	for (int i = 0; i < 8; i++) {
		val |= ((n >> i) & 1) << (7-i);
	}
	return val;
}

/*
int main()
{
	//grain_ctx grain;

	//u8 m[] = {0x00, 0x80};
	u8 m[] = {0, 0x80};
	unsigned long long mlen = sizeof(m);
	//u8 c[sizeof(m) + 8];
	u8 c[sizeof(m) + 8];
	unsigned long long clen;

	u8 ad[10];
	for (int i = 0; i < 10; i++) {
		ad[i] = swapsb(i);
	}
	unsigned long long adlen = sizeof(ad);

	//u8 key[16] = {0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef, 0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0xde, 0xf0};
	//u8 iv[12] = {0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef, 0x12, 0x34, 0x56, 0x78};
	u8 key[16] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
	u8 iv[12] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
	//u8 key[16] = { 0 };
	//u8 iv[12] = { 0 };

	u8 k[16];
	u8 npub[12];

	// change to LSB first
	for (int i = 0; i < 16; i++) {
		k[i] = swapsb(key[i]);
	}

	for (int i = 0; i < 12; i++) {
		npub[i] = swapsb(iv[i]);
	}

	for (int i = 0; i < 10; i++) {
		crypto_aead_encrypt(c, &clen, m, mlen, ad, i, NULL, npub, k);
	}

	//grain_init(&grain, k, npub);
//	crypto_aead_encrypt(c, &clen, m, mlen, ad, adlen, NULL, npub, k);

	printf("msg: ");
	for (int i = 0; i < mlen; i++) {
		printf("%02x", swapsb(m[i]));
	}
	printf("\ncip: ");
	for (int i = 0; i < clen; i++) {
		printf("%02x", swapsb(c[i]));
	}
	printf("\n");

	
	
	//crypto_aead_encrypt(c, &clen, m, mlen, NULL, 0, NULL, npub, k);

}
*/
