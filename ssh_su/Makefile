CC=gcc
CFLAGS = -Werror -Wall

all: impost3r.so

impost3r.so: main.c ../dns/dns.c
	$(CC) $(CFLAGS) -shared -fPIC -Xlinker -x -o impost3r.so main.c ../dns/dns.c

clean:
	rm *.o impost3r.so