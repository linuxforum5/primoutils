# primoutils
Utilities for Hungarian Primo computer

## pp2ptp
Convert the .pp file into .ptp format.
The z88dk can compile a C program to Primo, but the generated .pp file can use only in emulator. The real hardware can't use it.
This utility convert it to .ptp format. The .ptp format can convert to .wav with ptp2wav from <a href='https://github.com/vargaviktor/primotools/'>primotools</a>.
The .wav can load the original Primo computer.
### Example
#### Compile a new Primo C progam
Standard compilation. The program stored in memory from 0x4100 (or in a newest zcc version 0x4400) address.
    zcc +primo -create-app helloworld.c -o helloworld.bin

The latest z88dk can use the -zorg option for the primo target:
    zcc +primo -create-app -zorg 0xD000 helloworld.c -o helloworld.bin
This program loaded into memory from 0xD000 address.

This zcc command creates a helloworld.pp and helloworld.bin files. 
The pp2ptp can convert it to .ptp:
    ./pp2ptp -i helloworld.pp
This command creates a helloworld.ptp file.
<a href='https://github.com/vargaviktor/primotools/'>PTP2WAV.EXE</a> can convert it to WAV file.
Then the PRIMO BASIC LOAD command can load it into PRIMO memory.
