
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -g

all: grain128aead

init: CFLAGS += -DINIT
init: grain128aead

pre: CFLAGS += -DPRE
pre: grain128aead

grain128a: grain128a.c

clean:
	rm -f grain128aead
