
CC = gcc
CFLAGS = -std=c99 -Wall -Wextra -Wshadow -fsanitize=address,undefined -O2

all: genkat_aead

genkat_aead: genkat_aead.c grain128aead.c

clean:
	rm -f genkat_aead
