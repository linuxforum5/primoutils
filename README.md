# primoutils
Primo segédprogramok 2023 linux és windows rendszerekre:
    pp2ptp, ptp2c, ptp2turbo, ptp2txt, ptp2wav, ptp2pri, ptpblocks, ptpcreate, ptpdump, ptpinfo.
A Windows programok a win32 mappában, a linux parancsok a bin mappában találhatók.

## ptp2turbo
Gyorsan betölthető wav fájlt generált a megadott ptp fájlból. A gyors betöltés feltétele a jóminőségű lejátszás, azaz számítógépről/telefonról lejátszott wav esetén működőképes.
Hagyományos szalaggal nem teszteltem.
A program egy Turbo loader-t illeszt a ptp fájl elé, és a loader betöltődése után már az tölti be a primo programot.
A Manic miner 9 perces betöltődése helyett 1 perc alatt betölthető vele. :)


## ptpdump
Egy ptp fájlban található logikai blokkokat listázza vagy menti ki. A szalagon kisebb csomagokba tördelve tárolódnak az adatblokkok. Ez a program az egymás után következő összetartozó
adatblokkok adatait listázza ki.
A -d paraméter segítségével ezek a blokkok egy-egy külön fájlba is kimenthetők, így a ptp fájlban tárolt gép vagy BASIC program is lementhető egy-egy fájlba. A könnyebb felhasználhatósághoz
a képernyő blokkokt a -g paraméter megadása esetén GIF formátumban menti ki. A BASIC programot a -t hatására TXT fájlba menti le, azaz a BASIC programlistát menti le a fájlba. A -u paraméter
pedig utf8 kódolásúvá teszi ezt a fájl.

## ptpcreate
Ptp fájl generálását teszi lehetővé a ptpbump segítségével kiexoirtált blokkokból. BASIC esetén képes TXT formátumban lévő BASIC forráskódot is beilleszteni a PTP fájlba.
Az így generált ptp fájlból később a ptp2turbo segítségével is generálható wav állomány.

## ptp2wav
Egy .ptp típusú fájlt konvertál .wav formátumra. A konverzió alapjául Varga Viktor <a href='https://github.com/vargaviktor/primotools/'>primotools</a> projektben található PTP2WAV szolgált, 
de két további új opció segítségével a betöltési sebesség felgyorsítható.
Például a "ptp2wav -f 13000 -0 7 -i manic_miner.ptp -o manic_miner.wav" parancs segítségével konvertált wav fájl az eredeti 9 perc helyett 5 perc alatt betölthető, a Primo mindenféle módosítása nélkül.

A ptp2wav parancs használata
ptp2wav [opciók] -i <prp_fájlnév> -o <wav_fájlnév>
Opciók:
-f            : Fake baud a betöltéshez. A wav 8000Hz-es mintavételezéssel készül, de például 13000Hz-es mintavételezési értéket megadva sokkal gyorsabban betölthető.
-0            : A 0 bithez tartozó minták száma. Eredetileg 8, de ez 7-re is lecsökkenthető.
-1            : Az 1 bithez tartozó minták száma. Eredetileg 3. Nem ajánlott módosítani.
-i <ptp_fájl> : A konvertálandó ptp fájl neve.
-o <wav_fájl> : A generált wav fájl neve.
-v            : Beszédes kimenet.

## pp2ptp
Egy .pp típusú fájlt konvertál .ptp formátumra. (Jelenleg pp fájlt generál például a z88dk fordító -subtype=mamepp opció hatására.)
A .ptp fájl betölthető emulátorba, vagy tovább konvertálható .wav fájllá a ptp2wav segítségével.

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
A ptp2wav segítségével előállítható a WAV fájl
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
-R            : Show RAM size only.
-b            : Show blocks counter only.
-t            : Show type only (BASIC/SYSTEM).
-T            : Show minimum machine type only (A32/A48/A64).
-f            : Show filename with path only.
-n            : Show program name only.
-a            : Show autostart address only.
-v            : Verbose output.

## ptp2c
Gépi kódú programot ttartalmazó ptp fájl tartalmát c forrskóddá konvertálja
