;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; Microkey Primo clock speed 2.5MHz. The Z80 T = 0.4us
;;; Source wav file contains 62000 sample per seconds. One sample length 16,129032258 ~ 40T
;;; Source wav format:
;;; - Prefix is 20000 silence sample (HIGH value in port)
;;; - blocks
;;; A block format:
;;; - 2 bytes : Relative start address from screen start (lower byte first)
;;; - 2 bytes : The data size. (lower byte first)
;;; - The screen data. Byte after byte.
;;; - 1 byte : The next or end byte. If 0, the slideshow end. If 1, read next block.
;;; A byte format:
;;; - 8 bites
;;; - 3 sample HIGH value working time ( ~ 121T )
;;; A bit format:
;;; - 2/4 LOW sample if the bit is 0/1 ( ~ 81T/161T )
;;; - 3 HIGH sample bit separator and working time ( ~ 121T )
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
PORT:                    EQU     $1F             ; 0-63 porttartomány képviselője. Ezen keresztül érhető el a kazettacsatlakozó
PORT_MIRROR:             EQU     $403B           ; Port mirror byte
SCREEN_START_POINTER:    EQU     $4039

                ORG $4400

START:                               ;  T ;
    DI                               ;  1 ; Primo alatt nincsenek maszkolható megszakítások, így ez felesleges is akár
    LD A, (PORT_MIRROR)              ;  7 ;
    AND 127                          ;  7 ; Reset NMI bit
    OUT (PORT), A                    ; 11 ; Disable NMI
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
READ_BLOCK:                          ;  T ;
                                          ; Read block header
    LD HL, ( SCREEN_START_POINTER )  ;    ; Screen first byte address
    CALL READ_BYTE_INTO_B            ; 17 ;
    LD E, B
    CALL READ_BYTE_INTO_B            ; 17 ;
    LD D, B                               ; DE contains realtive start address. In general: 0.
    ADD HL, DE                            ; HL contains the start address
    CALL READ_BYTE_INTO_B            ; 17 ;
    LD E, B
    CALL READ_BYTE_INTO_B            ; 17 ;
    LD D, B                               ; DE contains byte counter. In general: 0x1800
;;;;;;;;;;;; time of 3 samples is 121T

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; Read a full block
;;; HL : Start address in memory for store
;;; DE : byte counter
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
READ_BLOCK_DATA:                     ;  T ; Egy bájt feldolgozásának ideje: 13.75us + 5.25us - 1.25us a GET_BYTE-on belül = 17.75us
    CALL READ_BYTE_INTO_B            ; 17 ; Read a byte
    LD (HL), B                       ;  7 ; Store a readed byte
    INC HL                           ;  4 ; Increment store address
    DEC DE                           ;  4 ; Decrement byte counter
    LD A, D                          ;  7 ;
    OR E                             ;  4 ;
    JR NZ, READ_BLOCK_DATA           ;12/7; Read next byte if DE is not 0
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; End Read block data
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;; ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    CALL READ_BYTE_INTO_B            ; 17 ; Read flag byte. 1=read next block, 0=End slideshow: wait for ENTER to restart
    DEC B                            ;  4 ;
    JR Z, READ_BLOCK                 ;    ; If Z, then B was 1: read next block.
                                          ; Else end slidshow
    EI                               ;  4 ;
    LD A, (PORT_MIRROR)              ;    ;
    OUT (PORT), A                    ; 11 ; Enabled NMI
WAIT:                                     ; Start keybord listening
    IN A, (55)                       ;    ; Check RETURN port.
    AND 1                            ;    ; Primo füzetek - Szoftver 153. oldal
    JR Z, WAIT                            ; Wait for RETURN key "pressed"
    JP 0                                  ; RETURN pressed, reboot

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; A kiírt minta 62000 s/s sebességgel ( 1 mintavétel hossza = 16,129032258 us : 10.08 )
;;;   1 bit : T*B{4,2}T3 : 1 esetén ~ 112.903 us (4Mhz-re vetítve:  70.5645)
;;;                                                                 B4 = 40.32us
;;;                        0 esetén ~  80.645 us (               :  50.403 )
;;;   8 bit után + T3 :   00 esetén ~ 693.548 us (               : 433.467 )
;;;                       FF esetén ~ 951.613 us (               : 594.758 )
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; Read byte into the B register
;;; C, A and F change
;;; B contains readed byte
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
READ_BYTE_INTO_B:            ;  T ;
    LD B, 128                ;  7 ; The 7. bit set to 1 for bit counting ( RR command. see below )
NEXT_BIT:                         ; 1 bit olvasási ideje: 11.25us (18us) | 1 minta 14.2us | 15.625 | 11.57
    LD C, 0                  ;  7 ; Initialize low length counter
SYNCH:                            ; Várakozás, a HIGH végére. Belső köridő: 7.5us Kilépés nél 6.25us. A bitet lezáró LOW jel kezdetétől ideáig 14.25us telik el Ennyi lehet maximum a LOW jel hossza
    IN A, (PORT)             ; 11 ; Read port data
    AND 4                    ;  7 ;
    JR NZ, SYNCH             ;12/7; 
NOP                          ;  4 ; NOP wait block for bit synchron
NOP                          ;  4 ; If the black screen data (many zero bytes) contains random white pixels, then this NOP block is too short
NOP                          ;  4 ;
NOP                          ;  4 ; Last NOP in wait block
LOWCNT:                           ; LOW length counter loop begin
    IN A, (PORT)             ; 11 ; Read port data
    INC C                    ;  4 ;
    AND 4                    ;  7 ;
    JR Z, LOWCNT             ;12/7; Belső köridő 8.5us, kilépésnél 7.25us. Egy bit 20 vagy 40us, tehát 0 esetén C = 2 vagy 3
    LD A, 3                  ;  7 ; If bit encoding use 4 or 2 samples, then 3 is good value. 0 bit lengt <= 3 loop. If C>3 then bit is 1
    SUB C                    ;  4 ; CY = 1 if C > 3
    RR B                     ;  8 ; CY->(HL).7->(HL).0->CY
    JR NC, NEXT_BIT          ;12/7;
    RET                      ; 10 ;
END_READ_BYTE_INTO_B:
