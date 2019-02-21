
CC = gcc
CFLAGS = -std=c99 -Wall -Wextra -Wshadow -fsanitize=address,undefined -O2

all: genkat_aead

test: grain128aead

genkat_aead: genkat_aead.c grain128aead.c

grain128a: grain128a.c

clean:
	rm -f grain128aead
