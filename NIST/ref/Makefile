
CC = gcc
CFLAGS = -std=c99 -Wall -Wextra -Wshadow -fsanitize=address,undefined -O2

all: genkat_aead

genkat_aead: genkat_aead.c grain128aead-v2.c

nist: genkat_aead
	@echo -e "\e[1;34mGenerating test vectors ...\e[0m"
	./genkat_aead
	mkdir -p crypto_aead/grain128aeadv2/ref
	cp LWC_AEAD_KAT_128_96.txt crypto_aead/grain128aeadv2/
	cp api.h grain128aead-v2.c grain128aead-v2.h crypto_aead/grain128aeadv2/ref
	@echo -e "\e[1;34mCreating tar ball ...\e[0m"
	tar -cvjf grain128aead-v2-ref.tar.gz crypto_aead

prof: genkat_aead.c grain128aead-v2.c
	$(CC) -Wall -O2 -pg -o grain_prof $?

clean:
	rm -Rf genkat_aead crypto_aead grain_prof gmon.out
