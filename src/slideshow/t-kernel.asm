;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; Microkey Primo clock speed 2.5MHz. The Z80 T = 0.4us
;;; Turbo kernel
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

