
#include <inttypes.h>
#include <stddef.h>
#include <stdio.h> // TODO: remove

#include "grain128aead_32p.h"

void print_state(grain_ctx *grain)
{
	u8 *nfsr = (u8 *) grain->nptr;
	u8 *lfsr = (u8 *) grain->lptr;
	u8 *acc = (u8 *) grain->acc;
	u8 *reg = (u8 *) grain->reg;

	printf("NFSR lsb: ");
	for (int i = 0; i < 16; i++) {
		u8 n = 0;
		// print with LSB first
		for (int j = 0; j < 8; j++) {
			n |= (((*(nfsr + i)) >> j) & 1) << (7-j);
		}
		printf("%02x", n);
	}
	//printf("\tNFSR msb: ");
	for (int i = 0; i < 16; i++) {
	//	printf("%02x", *(nfsr + i));
	}
	printf("\nLFSR lsb: ");
	for (int i = 0; i < 16; i++) {
		u8 l = 0;
		for (int j = 0; j < 8; j++) {
			l |= (((*(lfsr + i)) >> j) & 1) << (7-j);
		}
		printf("%02x", l);
	}
	//printf("\tLFSR msb: ");
	for (int i = 0; i < 16; i++) {
	//	printf("%02x", *(lfsr + i));
	}
	printf("\nACC lsb:  ");
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

void print_ks_word(u32 num)
{
	u32 swapped = ((num>>24)&0xff) | // move byte 3 to byte 0
                    ((num<<8)&0x00ff0000) | // move byte 1 to byte 2
                    ((num>>8)&0x0000ff00) | // move byte 2 to byte 1
                    ((num<<24)&0xff000000); // byte 0 to byte 3
	printf("%08x", swapped);
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
		((ln1 >> 28) & (ln2 >> 15)) ^ ((nn0 >> 12) & (nn2 >> 31) & (ln2 >> 30)); // ln2 >> 30
}

void grain_init(grain_ctx *grain, const u8 *key, const u8 *iv)
{
	// load key 32 bits at a time
	*(u32 *) (grain->nfsr) = *(u32 *) (key);
	*(u32 *) (grain->nfsr + 1) = *(u32 *) (key + 4);
	*(u32 *) (grain->nfsr + 2) = *(u32 *) (key + 8);
	*(u32 *) (grain->nfsr + 3) = *(u32 *) (key + 12);
	
	// load IV along with padding
	*(u32 *) (grain->lfsr) = *(u32 *) (iv);
	*(u32 *) (grain->lfsr + 1) = *(u32 *) (iv + 4);
	*(u32 *) (grain->lfsr + 2) = *(u32 *) (iv + 8);
	*(u32 *) (grain->lfsr + 3) = (u32) 0x7fffffff;

	grain->count = 4;
	grain->nptr = grain->nfsr;
	grain->lptr = grain->lfsr;

	printf("pre-init:\n");
	print_state(grain);

	printf("init:\n");
	register u32 ks;
	for (int i = 0; i < 8; i++) {
		ks = next_keystream(grain);
		grain->nfsr[i + 4] ^= ks;
		grain->lfsr[i + 4] ^= ks;
		print_state(grain);
	}

	// add the key in the feedback, "FP(1)"
	printf("FP(1)\n");
	for (int i = 0; i < 2; i++) {
		// initialize accumulator
		ks = next_keystream(grain);
		*((u32 *) (grain->acc) + i) = ks;
		grain->lfsr[i + 12] ^= *(u32 *) (key + 4 * i);
		print_state(grain);
	}
	
	for (int i = 0; i < 2; i++) {
		// initialize register
		ks = next_keystream(grain);
		*((u32 *) (grain->reg) + i) = ks;
		grain->lfsr[i + 14] ^= *(u32 *) (key + 8 + 4 * i);
		print_state(grain);
	}
}

void grain_reinit(grain_ctx *grain)
{
	printf("REINIT\n");
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

int crypto_aead_encrypt(
	unsigned char *c, unsigned long long *clen,
	const unsigned char *m, unsigned long long mlen,
	const unsigned char *ad, unsigned long long adlen,
	const unsigned char *nsec,
	const unsigned char *npub,
	const unsigned char *k);

int crypto_aead_decrypt(
	unsigned char *m, unsigned long long *mlen,
	unsigned char *nsec,
	const unsigned char *c, unsigned long long clen,
	const unsigned char *ad, unsigned long long adlen,
	const unsigned char *npub,
	const unsigned char *k
);

u8 swapsb(u8 n)
{
	// swaps significant bit
	u8 val = 0;
	for (int i = 0; i < 8; i++) {
		val |= ((n >> i) & 1) << (7-i);
	}
	return val;
}

int main()
{
	grain_ctx grain;

	/*
	u8 m[2] = {0, 0};
	unsigned long long mlen = 2;
	u8 c[sizeof(m) + 8];
	unsigned long long clen;
	*/
	u8 key[16] = {0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef, 0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0xde, 0xf0};
	u8 iv[12] = {0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef, 0x12, 0x34, 0x56, 0x78};
	//u8 key[16] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
	//u8 iv[12] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
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

	grain_init(&grain, k, npub);

	printf("ks: ");
	for (int i = 0; i < 8; i++) {
		u32 ks = next_keystream(&grain);
		print_ks(ks);
		//print_state(&grain);
	}
	printf("\n");
	
	
	//crypto_aead_encrypt(c, &clen, m, mlen, NULL, 0, NULL, npub, k);

}
