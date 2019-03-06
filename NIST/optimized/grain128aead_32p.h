
#ifndef GRAIN128AEAD_32P_H
#define GRAIN128AEAD_32P_H

typedef uint8_t		u8;
typedef uint16_t	u16;
typedef uint32_t	u32;
typedef uint64_t	u64;

typedef struct {
	u32 lfsr[1024];
	u32 nfsr[1024];
	u32 *lptr;
	u32 *nptr;
	u32 count;
} grain_ctx;

#endif
