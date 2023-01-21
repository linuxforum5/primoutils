# Simple makefile for utils
CC=gcc
WCC=i686-w64-mingw32-gcc
SRC=src
WBIN=win32
BIN=bin
INSTALL_DIR=~/.local/bin

all: pp2ptp ptpinfo ptp2txt ptpdump ptp2wav

pp2ptp: $(SRC)/pp2ptp.c
	$(CC) -o $(BIN)/pp2ptp $(SRC)/pp2ptp.c
	$(WCC) -o $(WBIN)/pp2ptp $(SRC)/pp2ptp.c

ptp2wav: $(SRC)/ptp2wav.c
	$(CC) -o $(BIN)/ptp2wav $(SRC)/ptp2wav.c
	$(WCC) -o $(WBIN)/ptp2wav $(SRC)/ptp2wav.c

ptpinfo: $(SRC)/ptpinfo.c
	$(CC) -Isrc/lib -o $(BIN)/ptpinfo $(SRC)/ptpinfo.c $(SRC)/lib/fs.c
	$(WCC) -Isrc/lib -o $(WBIN)/ptpinfo $(SRC)/ptpinfo.c $(SRC)/lib/fs.c

ptp2txt: $(SRC)/ptp2txt.c
	$(CC) -Isrc/lib -o $(BIN)/ptp2txt $(SRC)/ptp2txt.c $(SRC)/lib/fs.c
	$(WCC) -Isrc/lib -o $(WBIN)/ptp2txt $(SRC)/ptp2txt.c $(SRC)/lib/fs.c

ptpdump: $(SRC)/ptpdump.c
	$(CC) -Isrc/lib -o $(BIN)/ptpdump $(SRC)/ptpdump.c $(SRC)/lib/fs.c
	$(WCC) -Isrc/lib -o $(WBIN)/ptpdump $(SRC)/ptpdump.c $(SRC)/lib/fs.c

clean:
	rm -f $(WBIN)/* $(BIN)/* *~ $(SRC)/*~ 

install:
	cp $(BIN)/* $(INSTALL_DIR)/
