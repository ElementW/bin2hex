STRIP ?= strip

all: bin2hex

bin2hex: bin2hex.c
	$(CC) -std=c99 -O3 -Os -o bin2hex bin2hex.c
	#$(STRIP) bin2hex