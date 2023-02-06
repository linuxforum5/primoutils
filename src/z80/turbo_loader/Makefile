# Makefile.primo

BASENAME=wavloader
TEMP_BIN_NAME=~z80.bin
LOAD_ADDRESS='\x00\x44'
START_ADDRESS='\x00\x44'

all: $(BASENAME)

$(BASENAME): $(BASENAME).asm
	/usr/bin/printf $(LOAD_ADDRESS)$(START_ADDRESS) > $(BASENAME).pp
	rm -f $(BASENAME).wav $(BASENAME).c
	z80asm -o $(TEMP_BIN_NAME)  $(BASENAME).asm
	cat $(TEMP_BIN_NAME) >> $(BASENAME).pp
	rm $(TEMP_BIN_NAME)
	pp2ptp -i $(BASENAME).pp
	ptp2c -N -n turbo_loader -i $(BASENAME).ptp -o $(BASENAME).c
	cp *.c /home/teszt/workspace/retro/primo/utils/github.com/primoutils/src/ptp2turbo/

clean:
	rm -f *.bak out/*.pri *.pp *.ptp *.bin *.hex *.wav $(BASENAME)
