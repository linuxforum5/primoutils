# Makefile.primo

NAME64=memtestA64
NAME32=memtestA32

all: A64

A64: $(NAME64).asm
	/usr/bin/printf '\x00\xE4\x00\xE4' > $(NAME64).pp
	z80asm $(NAME64).asm
	cat a.bin >> $(NAME64).pp
	pp2ptp -i $(NAME64).pp
	ptp2turbo5 -m 57344 -i $(NAME64).ptp -o $(NAME64).t5.wav

A32: $(NAME32).asm
	/usr/bin/printf '\x00\xE4\x00\xE4' > $(NAME32).pp
	z80asm $(NAME32).asm
	cat a.bin >> $(NAME32).pp
	pp2ptp -i $(NAME32).pp
	ptp2turbo5 -i $(NAME32).ptp -o $(NAME32).t5.wav

clean:
	rm -f *.bak out/*.pri *.pp *.ptp *.bin *.hex *.wav
