
CC = gcc
CFLAGS = -std=c99 -Wall -Wextra -Wshadow -fsanitize=address,undefined -O2

all: genkat_aead

genkat_aead: genkat_aead.c grain128aead_32p.c

nist: genkat_aead
	@echo -e "\e[1;34mGenerating test vectors ...\e[0m"
	./genkat_aead
	mkdir -p crypto_aead/grain128aead/opt32
	cp LWC_AEAD_KAT_128_96.txt crypto_aead/grain128aead/
	cp api.h grain128aead_32p.c grain128aead_32p.h crypto_aead/grain128aead/opt32
	@echo -e "\e[1;34mCreating tar ball ...\e[0m"
	tar -cvjf grain128aead-opt32.tar.gz crypto_aead

prof: genkat_aead.c grain128aead_32p.c
	$(CC) -Wall -O2 -pg -o grain_prof32 $?

clean:
	rm -Rf genkat_aead crypto_aead grain128aead_32p grain_prof32 gmon.out
