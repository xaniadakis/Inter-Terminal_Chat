CC=gcc
DEPS=struct.h

all:	p1	enc1	chan	enc2	p2

p1:	process1.c
	gcc	process1.c	-o	p1	-lpthread
enc1:	enc1.c
	gcc	enc1.c	-o	enc1	-lpthread	-lcrypto
chan:	chan.c
	gcc	chan.c	-o	chan	-lpthread
enc2:	enc2.c
	gcc	enc2.c	-o	enc2	-lpthread	-lcrypto
p2:	process2.c
	gcc process2.c	-o	p2	-lpthread

clean:
		$(RM)	p1	enc1	chan	enc2	p2

