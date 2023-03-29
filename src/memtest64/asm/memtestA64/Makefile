# Makefile.primo

NAME64=memtestA64
NAME32=memtestA32
UTILS=/home/teszt/workspace/retro/primo/utils
PTPDIR=/home/teszt/workspace/retro/primo/programok/z88dk

all: $(NAME32) $(NAME64)

$(NAME64): $(NAME64).asm
	/usr/bin/printf '\x00\xE4\x00\xE4' > $(NAME64).pp
	z80asm $(NAME64).asm
	cat a.bin >> $(NAME64).pp
	$(UTILS)/pp2ptp -i $(NAME64).pp
	wine $(UTILS)/PTP2WAV.EXE $(NAME64).ptp $(NAME64).wav
	cp $(NAME64).ptp $(PTPDIR)/

$(NAME32): $(NAME32).asm
	/usr/bin/printf '\x00\x64\x00\x64' > $(NAME32).pp
	z80asm $(NAME32).asm
	cat a.bin >> $(NAME32).pp
	$(UTILS)/pp2ptp -i $(NAME32).pp
	wine $(UTILS)/PTP2WAV.EXE $(NAME32).ptp $(NAME32).wav
	cp $(NAME32).ptp $(PTPDIR)/

clean:
	rm -f *.bak out/*.pri *.pp *.ptp *.bin *.hex *.wav $(NAME)
