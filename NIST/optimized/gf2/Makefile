
CC = g++
CFLAGS = -std=c++11 -msse3 -mavx512f -mavx512vl -mgfni -mpclmul -Wall -Wextra -Wshadow -fsanitize=address,undefined -O2

all: genkat_aead

genkat_aead: genkat_aead.c grain128aead-v2_opt.cpp

prof: genkat_aead.c grain128aead-v2_opt.cpp
	$(CC) -Wall -O2 -pg -o grain_prof $?

clean:
	rm -Rf genkat_aead crypto_aead grain128aead-v2_opt grain_prof gmon.out
