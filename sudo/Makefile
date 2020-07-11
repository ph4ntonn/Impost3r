CC=gcc
CFLAGS=-g -Wall

all: .impost3r

.impost3r: main.c ../dns/dns.c ../encode/base32.c 
	$(CC) $(CFLAGS) -o .impost3r main.c ../dns/dns.c ../encode/base32.c

clean:
	rm *.o .impost3r
