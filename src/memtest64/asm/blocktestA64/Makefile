# Makefile.primo

NAME64=memtestA64

all: A64

A64: $(NAME64).asm
	/usr/bin/printf '\x00\xE4\x00\xE4' > $(NAME64).pp
	z80asm $(NAME64).asm
	cat a.bin >> $(NAME64).pp
	pp2ptp -i $(NAME64).pp
	ptp2turbo5 -m 57344 -i $(NAME64).ptp -o $(NAME64).t5.wav
	ptp2wav -f 9000 -i $(NAME64).ptp -o $(NAME64).wav

clean:
	rm -f *.bak out/*.pri *.pp *.ptp *.bin *.hex *.wav
