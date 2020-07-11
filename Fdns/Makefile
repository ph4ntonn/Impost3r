CC=gcc
CFLAGS=-g -Wall

all: dns

dns: main.c util.c ../encode/base32.c 
	$(CC) $(CFLAGS) -o dns main.c util.c ../encode/base32.c

clean:
	rm *.o dns
