
#include <inttypes.h>
#include <stddef.h>

#include "grain128aead_32p.h"

void print_state(grain_ctx *grain)
{
	u8 *nfsr = (u8 *) grain->nfsr;
	u8 *lfsr = (u8 *) grain->lfsr;

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

void grain_init(grain_ctx *grain, const u8 *key, const u8 *iv)
{
	*(u64 *) (grain->nfsr) = *(u64 *) (key);
	// lfsr and nfsr are 32 bit wide, add 2 to skip the first 64 bits
	*(u64 *) (grain->nfsr + 2) = *(u64 *) (key + 8);
	
	// load IV along with padding
	*(u64 *) (grain->lfsr) = *(u64 *) (iv);
	*(u32 *) (grain->lfsr + 2) = *(u32 *) (iv + 8);
	*(u32 *) (grain->lfsr + 3) = (u32) 0xfeffffff;
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

	u8 m[2] = {0, 0};
	unsigned long long mlen = 2;
	u8 c[sizeof(m) + 8];
	unsigned long long clen;
	u8 k[16] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
	u8 npub[12] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
	
	grain_init(&grain, k, npub);
	print_state(&grain);
	
	
	//crypto_aead_encrypt(c, &clen, m, mlen, NULL, 0, NULL, npub, k);

}
