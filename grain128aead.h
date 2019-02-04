#ifndef UTILS_H
#define UTILS_H

#define STREAM_BYTES	40
#define MSG_BYTES		0

typedef struct {
	uint8_t lfsr[128];
	uint8_t nfsr[128];
	uint8_t auth_acc[32];
	uint8_t auth_sr[32];
} grain_state;

// TODO: add struct with output: keystream and optionally macstream and tag
typedef struct {
	uint8_t keystream[STREAM_BYTES];
	uint8_t macstream[STREAM_BYTES];
	uint8_t *message;
} grain_data;

void init_grain(grain_state *grain, uint8_t *key, uint8_t *iv);
uint8_t next_lfsr_fb(grain_state *grain);
uint8_t next_nfsr_fb(grain_state *grain);
uint8_t next_h(grain_state *grain);
uint8_t shift(uint8_t fsr[128], uint8_t fb);
void auth_shift(uint8_t sr[32], uint8_t fb);
uint8_t next_z(grain_state *grain);
void generate_keystream(grain_state *grain, grain_data *data);
void print_state(grain_state *grain);

#endif
