# Simple makefile for utils

CC=gcc
SRC=src
BIN=bin
INSTALL_DIR=~/.local/bin

all: pp2ptp

pp2ptp: $(SRC)/pp2ptp.c
	$(CC) -o $(BIN)/pp2ptp $(SRC)/pp2ptp.c

clean:
	rm -f $(BIN)/* *~ $(SRC)/*~ 

install:
	cp $(BIN)/* $(INSTALL_DIR)/
