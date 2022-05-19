#!/bin/sh
# Ahhoz, hogy a kezdőcím működjön, a z88dk/z88dk/lib/target/primo/classic/primo_crt0.asm
NAME=helloworld
# STARTADDR=0x4100
# STARTADDR=0xE800
# STARTADDR=0xD000
STARTADDR=0xB000
rm out/$NAME.*
Z88HOME=/home/teszt/workspace/retro/z80/z88dk/z88dk
UTILS=/home/teszt/workspace/retro/primo/utils
export PATH=${PATH}:${Z88HOME}/bin
export ZCCCFG=${Z88HOME}/lib/config
zcc +primo -zorg $STARTADDR -create-app $NAME.c -o out/$NAME.bin
# zcc +primo -create-app $NAME.c -o $NAME.bin
$UTILS/pp2ptp -i out/$NAME.pp
wine $UTILS/PTP2WAV.EXE out/$NAME.ptp out/$NAME.wav
