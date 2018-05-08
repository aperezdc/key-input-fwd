
all: key-input-fwd

key-input-fwd: main.c
	$(CC) -Wall -std=c11 -o key-input-fwd main.c
