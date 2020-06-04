CC=gcc
CFLAGS=-g -Wall

all: .impost3r

.impost3r: main.c ../dns/dns.c
	$(CC) $(CFLAGS) -o .impost3r main.c ../dns/dns.c

clean:
	rm *.o .impost3r
