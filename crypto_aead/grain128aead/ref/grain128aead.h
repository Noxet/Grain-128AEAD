#ifndef UTILS_H
#define UTILS_H

#define STREAM_BYTES	16
#define MSG_BYTES		0

enum GRAIN_ROUND {INIT, FP1, NORMAL};

typedef struct {
	uint8_t lfsr[128];
	uint8_t nfsr[128];
	uint8_t auth_acc[64];
	uint8_t auth_sr[64];
} grain_state;

// TODO: add struct with output: keystream and optionally macstream and tag
typedef struct {
	unsigned char *message;
	unsigned long long msg_len;
} grain_data;

void init_grain(grain_state *grain, const unsigned char *key, const unsigned char *iv);
uint8_t next_lfsr_fb(grain_state *grain);
uint8_t next_nfsr_fb(grain_state *grain);
uint8_t next_h(grain_state *grain);
uint8_t shift(uint8_t fsr[128], uint8_t fb);
void auth_shift(uint8_t sr[32], uint8_t fb);
uint8_t next_z(grain_state *grain, uint8_t);
void generate_keystream(grain_state *grain, grain_data *data, uint8_t *);
void print_state(grain_state *grain);

#endif
