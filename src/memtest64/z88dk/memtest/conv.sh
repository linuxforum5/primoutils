#!/bin/sh
# NAME=memtest0xD000
NAME=memtest0xB23A
UTILS=/home/teszt/workspace/retro/primo/utils
$UTILS/pp2ptp -i out/$NAME.pp
wine $UTILS/PTP2WAV.EXE out/$NAME.ptp out/$NAME.wav
cp out/$NAME.ptp /home/teszt/workspace/retro/primo/programok/z88dk/
