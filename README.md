# primoutils
Primo segédprogramok 2022 linux és windows rendszerekre:
pp2ptp, ptp2txt, ptpinfo, ptpdump.
A Windows programok a win32 mappában, a linux parancsok a bin mappában találhatók.

## pp2ptp
Egy .pp típusú fájlt konvertál .ptp formátumra. (Jelenleg pp fájlt generál például a z88dk fordító -subtype=mamepp opció hatására.)
A .ptp fájl betölthető emulátorba, vagy tovább konvertálható .wav fájllá a <a href='https://github.com/vargaviktor/primotools/'>primotools</a> projektben található PTP2WAV.EXE segítségével.

A pp2ptp parancs használata
pp2ptp [opciók] -i <pp_fájlnév> [ -o <ptp_fájlnév> ]
Opciók:
-o <ptp_fájl> : A generált output fájl neve. Ha nincs megadva, akkor az input fájl neve, de .ptpt kiterjesztéssel.
-n <név>      : A program betöltési neve. Ha nincs megadva, akkor a megadott input fájlnév kiterjesztés nélkül.
-v            : Beszédes kimenet.

### Példa
#### A helloworld.c program futtatása Primón
Normál fordítás. A program a 0x4400 címre kerül betöltésre.
    zcc +primo -subtype=mamepp -create-app helloworld.c -o helloworld.bin
A legfrissebb zcc már lehetővé teszi a -zorg paraméter használatát is Primo esetében, így tetszőleges címre is tölthetjük a programot. Pl.:
    zcc +primo -subtype=mamepp -create-app -zorg 0xD000 helloworld.c -o helloworld.bin
Ezzel elkészül a helloworld.pp állomány.
A pp2ptp segítségével ez már konvertálható ptp formátumra:
    ./pp2ptp -i helloworld.pp
Ez létrehozza a helloworld.ptp fájlt.
A <a href='https://github.com/vargaviktor/primotools/'>PTP2WAV.EXE</a> segítségével előállítható a WAV fájl
A Primo BASIC LOAD parancsa ezt már képes betölteni és lefuttatni.

##ptp2txt
Egy .ptp fájlban található BASIC program kilistázása. Erőssége az átsorszámozási lehetőség, és az UTF-8 konverzió!
ptpinfo [opciók] -i <ptp_filename> [ -o <txt_filename> ]
Opciók:
-v            : Beszédes mód.
-u            : A Primó ékezetes karaktereit UTF-8 karakterekre cseréli.
-r            : Átsorszámozza a BASIC programot, és úgy listázza ki. Hibás sorszámhivatkozás esetén a hibás sorszámhivatkozás változatlan marad.

##ptpinfo
Egy ptp fájlról szolgáltat információkat. Ha nincsenek megadva egyedi opciók, akkor minden információt megjelenít. Teszteléshez hasznos lehet.
ptpinfo [opciók] -i <ptp_fájlnév>
Pociók:
-R            : RAM size only.
-b            : Blocks counter only.
-t            : Type only (BASIC/SYSTEM).
-T            : Machine type only (A32/A48/A64).
-f            : Filename with path only.
-n            : Program name only.
-v            : Verbose output.

##ptpdump
Egy gépikódú programot tartalmazó .ptp fájlt konvertál bináris fájllá, ami a betöltés utáni memóratartalmat jelenti.
