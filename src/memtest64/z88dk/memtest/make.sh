#!/bin/sh
# Ahhoz, hogy a kezdőcím működjön, a z88dk/z88dk/lib/target/primo/classic/primo_crt0.asm
NAME=memtest
# STARTADDR=0xD000
STARTADDR=0xB23A
rm out/$NAME*
Z88HOME=/home/teszt/workspace/retro/z80/z88dk/z88dk
export PATH=${PATH}:${Z88HOME}/bin
export ZCCCFG=${Z88HOME}/lib/config
zcc +primo -subtype=mamepp -zorg $STARTADDR -create-app $NAME.c -o out/$NAME$STARTADDR.bin
# UTILS=/home/teszt/workspace/retro/primo/utils
# $UTILS/pp2ptp -i out/$NAME.pp
# wine $UTILS/PTP2WAV.EXE out/$NAME.ptp out/$NAME.wav
