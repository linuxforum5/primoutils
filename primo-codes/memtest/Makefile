# Simple makefile for utils
NAME=memtest8000
Z88HOME=/home/teszt/workspace/retro/z80/z88dk/z88dk
SRC=src
BIN=bin
export PATH:=${PATH}:${Z88HOME}/bin
export ZCCCFG:=${Z88HOME}/lib/config

all: memtest8000 memtest4000 memtestC000

memtest8000: $(SRC)/memtest8000.c
	zcc +primo -subtype=mamepp -zorg 0x4400 -create-app $(SRC)/memtest8000.c -o $(BIN)/memtest8000-load4400.bin
	pp2ptp -i $(BIN)/memtest8000-load4400.pp -o $(BIN)/memtest8000-load4400.ptp
	ptp2turbo5 -i $(BIN)/memtest8000-load4400.ptp -o $(BIN)/memtest8000-load4400.t5.wav

memtest4000: $(SRC)/memtest4000.c
	zcc +primo -subtype=mamepp -zorg 0x8000 -create-app $(SRC)/memtest4000.c -o $(BIN)/memtest4000-load8000.bin
	pp2ptp -i $(BIN)/memtest4000-load8000.pp -o $(BIN)/memtest4000-load8000.ptp
	ptp2turbo5 -i $(BIN)/memtest4000-load8000.ptp -o $(BIN)/memtest4000-load8000.t5.wav

memtestC000: $(SRC)/memtestC000.c
	zcc +primo -subtype=mamepp -zorg 0x4400 -create-app $(SRC)/memtestC000.c -o $(BIN)/memtestC000-load4400.bin
	pp2ptp -i $(BIN)/memtestC000-load4400.pp -o $(BIN)/memtestC000-load4400.ptp
	ptp2turbo5 -i $(BIN)/memtestC000-load4400.ptp -o $(BIN)/memtestC000-load4400.t5.wav

clean:
	rm -f $(WBIN)/* $(BIN)/* *~ $(SRC)/*~

install:
	cp $(BIN)/* $(INSTALL_DIR)/
