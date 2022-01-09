all: p1 p2 chan
p1: p1.c
	gcc p1.c -o p1 -lcrypto

p2: p2.c
	gcc p2.c -o p2 -lcrypto

chan: chan.c
	gcc chan.c -o chan

clean:
	rm -f p1 p2 chan

