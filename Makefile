
CC = gcc
CFLAGS = -std=c99 -Wall -Wextra -Wshadow -fsanitize=address,undefined -O2

all: grain128aead

init: CFLAGS += -DPRINT_INIT
init: grain128aead

pre: CFLAGS += -DPRE
pre: grain128aead

grain128a: grain128a.c

clean:
	rm -f grain128aead
