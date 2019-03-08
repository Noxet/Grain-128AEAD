
#include <inttypes.h>
#include <stddef.h>

#include "grain128aead_32p.h"

void print_state(grain_ctx *grain)
{
	u8 *nfsr = (u8 *) grain->nptr;
	u8 *lfsr = (u8 *) grain->lptr;

	printf("NFSR: ");
	for (int i = 0; i < 16; i++) {
		printf("%02x", *(nfsr + i));
	}
	printf("\nLFSR: ");
	for (int i = 0; i < 16; i++) {
		printf("%02x", *(lfsr + i));
	}
	printf("\n");
}


u32 next_keystream(grain_ctx *grain)
{
	u64 ln0 = *(u64 *) (grain->lptr),
	    ln1 = *(u64 *) (grain->lptr + 1),
	    ln2 = *(u64 *) (grain->lptr + 2),
	    ln3 = *(u64 *) (grain->lptr + 3);
	u64 nn0 = *(u64 *) (grain->nptr),
	    nn1 = *(u64 *) (grain->nptr + 1),
	    nn2 = *(u64 *) (grain->nptr + 2),
	    nn3 = *(u64 *) (grain->nptr + 1);

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

	return (nn0 >> 2) ^ (nn0 >> 15) ^ (nn1 >> 4) ^ (nn1 >> 13) ^ nn2 ^ (nn2 >> 9) ^ (nn2 >> 25) ^ (ln2 >> 29) ^
		((nn0 >> 12) & (ln0 >> 8)) ^ ((ln0 >> 13) & (ln0 >> 20)) ^ ((nn2 >> 31) & (ln1 >> 10)) ^
		((ln1 >> 28) & (ln2 >> 15)) ^ ((nn0 >> 12) & (nn2 >> 31) & (ln2 >> 30));
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
	*(u32 *) (grain->lfsr + 3) = (u32) 0xfeffffff;

	grain->count = 4;
	grain->nptr = grain->nfsr;
	grain->lptr = grain->lfsr;

	register u32 ks;
	for (int i = 0; i < 8; i++) {
		ks = next_keystream(grain);
		grain->nfsr[i + 4] ^= ks;
		grain->lfsr[i + 4] ^= ks;
		//print_state(grain);
	}
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

int main()
{
	grain_ctx grain;

	/*
	u8 m[2] = {0, 0};
	unsigned long long mlen = 2;
	u8 c[sizeof(m) + 8];
	unsigned long long clen;
	*/
	u8 k[16] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
	u8 npub[12] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
	
	grain_init(&grain, k, npub);
	print_state(&grain);

	next_keystream(&grain);
	print_state(&grain);
	
	
	//crypto_aead_encrypt(c, &clen, m, mlen, NULL, 0, NULL, npub, k);

}
