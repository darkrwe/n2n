CC=gcc
CFLAGS=-Wall -g -O2

opentapdevice: test.c libancillary.a
	$(CC) -o $@ $(CFLAGS) -L. test.c -lancillary 
