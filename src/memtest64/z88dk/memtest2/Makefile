# Makefile.primo

Z88HOME=/home/teszt/workspace/retro/z80/z88dk/z88dk
export PATH := ${PATH}:${Z88HOME}/bin
export ZCCCFG := ${Z88HOME}/lib/config
STARTADDR=0xD000

CFLAGS = +primo -c -o $@
CC = zcc

objects = src/addresstest.o

addresstest:	$(objects)
	$(CC) +primo -subtype=mamepp -zorg $(STARTADDR) -create-app $(objects) -o out/addresstest.bin

src/addresstest.o: src/addresstest.c

clean:
	rm -f $(objects) *.bak out/*.pri out/*.pp out/*.bin out/*.hex addresstest
