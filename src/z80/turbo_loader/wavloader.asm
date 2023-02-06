;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; https://www.nongnu.org/z80asm/index.html
;;; http://www.ep128.hu/Ep_Konyv/UM0080.pdf
;;; 1 bit = 15.5us
;;; 1 byte = 124us + 16us | 8 bit + 16us ( -1.25 + 2.5 + 4.25 + 8.75 + 1.75 | 5.5 + 10.5 )
;;; ----p--------       ---h---   -------  | l,h = 50/100
;;;              ---l---       -e-         | e = 25   p = 75
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
PORT:                    EQU     $1F             ; 0-63 porttartomány képviselője. Ezen keresztül érhető el a kazettacsatlakozó
PORT_MIRROR:             EQU     $403B           ; Port mirror byte
; SYNC_BYTE:       EQU     $D3             ; A szinkronbájt értéke : 11010011
; LOAD_FROM:       EQU     $E800           ; Load from
; LOAD_COUNTER:    EQU     $1800           ; Loaded bytes counter
DSPHND:                  EQU     $0015
SCREEN_START_POINTER:    EQU     $4039
; STATUSL:                 EQU     $E800           ; A bal felső sarok
; STATUS:                  EQU     $EA99           ; A státusz pozíciója
BASIC:                   EQU     $1A33           ; Fejlesztési teljes romlista 15. oldal
BASIC_PRG_START:         EQU     $43EA
BASIC_PRG_START_POINTER: EQU     $40A4
BASIC_PRG_END_POINTER:   EQU     $40F9
BASIC_RUN_ADDR:          EQU     $1EA3
BASIC_CLEAR:             EQU     $1E7A
PUFFER_START:            EQU     $41E8             ; A billentyűzetpuffer kezdőcíme
DSPMSG:                  EQU     $3D03             ; DOM rutin

                ORG $4400                ; CALL( 58368, DE ) RETRUN HL

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; Előkészületek a BASIC memóriában
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    LD HL, LOADING_MSG
    CALL DSPMSG
    DI                    ; 1    ;
    LD A, (PORT_MIRROR)   ;
    AND 127               ; 1.75 ; Reset NMI bit
    OUT (PORT), A         ; 2.75 ; 
    LD (PORT_MIRROR), A   ;      ;;;;;;;; end DI
;;; A tényleges betöltő modul. Innentől kezdve figyeli a magnójelet, olvassa a fájl elejét, a header-t.
    EXX
    LD HL, (SCREEN_START_POINTER)
    EX DE, HL
    INC DE
    LD HL, (SCREEN_START_POINTER)
    LD (HL), $FF
    EXX
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; A kód START utáni részének átmásolása a billentyűzet pufferbe és a magnó pufferbe
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    LD HL, START          ;      ; 3 ;
    LD DE, PUFFER_START   ;      ; 3 ;
    LD BC, END-START      ;      ; 3 ;
    LDIR                  ;      ; 1 ;
    JP PUFFER_START       ;      ; 3 ; Ugrás a másolt memóriarészbe
START:                    ; 4MHz ;
    LD DE, 1
    LD HL, 0
PRE_HIGH0:                ;      ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;; Drop first high samples
    ADC HL, DE
    JR Z, PRE_HIGH0
    IN A, (PORT)          ; 2.75 ;
    AND 4                 ; 1.75 ;
    JR NZ, PRE_HIGH0      ; 3    ; = 7.5
    IN A, (PORT)          ; 2.75 ; Anti prill
    AND 4                 ; 1.75 ;
    JR NZ, PRE_HIGH0      ; 3    ;;; HIGH sampes skipped

    LD HL, 0
PRE_LOW0:                 ;      ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;; Drop first low samples
    ADC HL, DE
    JR Z, PRE_HIGH0
    IN A, (PORT)          ; 2.75 ;
    AND 4                 ; 1.75 ;
    JR Z, PRE_LOW0        ; 3    ; = 7.5
    IN A, (PORT)          ; 2.75 ; Anti prill
    AND 4                 ; 1.75 ;
    JR Z, PRE_LOW0        ; 3    ;;; LOW samples skipped
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; synchron is ok. Read crc data
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    LD HL, 0              ;      ; Sum checksum bytes in HL
    LD D, 14              ;      ; Read D byte for checksum + 2 bytes checksum
CHECK:
    CALL GET_BYTE         ; 4.25 ; PUSH=2.75 JR=3.00
    LD C, B
    ADD HL, BC            ;      ;
    DEC D
    JR NZ, CHECK
    CALL GET_BYTE         ; 4.25 ; Read the lower control byte
    LD A, B
    CP L                  ;      ;
    JR NZ, ERROR
    CALL GET_BYTE         ; 4.25 ; Read the higher control byte
    LD A, B
    CP H                  ;      ;
    JR NZ, ERROR    
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; CRC OK! Speed and volume is correct!
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
READ_BLOCK:
    CALL GET_BYTE         ; 4.25 ; B = Block type
    DEC B                 ;      ; B == 1 : Screen Else Direct Address
    JR Z, LOAD_SCREEN
    CALL GET_BYTE         ; 4.25 ; Read Load address into HL
    LD L, B               ; 1    ;
    CALL GET_BYTE         ; 4.25 ;
    LD H, B               ; 1    ; First byte address in HL
LOAD_COUNTER:
    CALL GET_BYTE         ; 4.25 ; Read Byte counter into DE
    LD E, B               ; 1    ;
    CALL GET_BYTE         ; 4.25 ;
    LD D, B               ; 1    ; Byte counter in DE

    LD A, B               ;      ;;;;;;;;;;;;;;;;;; Init status line on screen ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    INC A
    EXX
    LD C, A
    LD B, 0
    LDIR
    EXX                   ;      ; Write line end. HL contains last line address

LOAD_DATA:
    CALL GET_BYTE         ; 4.25 ; +6.75-9.5
    LD (HL), B            ; 1.75 ; +1.75
    INC HL                ; 1.5  ;
    DEC DE                ; 1.5  ;

    LD A, D
    CP E
    JR NZ, NO_STATUS      ; 3    ; Decrement status line if D==E
    EXX
    LD (HL), 0
    DEC HL
    EXX
NO_STATUS:
    OR E                  ; 1    ; A OR E, azaz D OR E, vagyis csak akkor nulla, ha DE==0
    JR NZ, LOAD_DATA      ; 3    ; +17.5 While DE != 0
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;; All block data loaded ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    CALL GET_BYTE         ; 4.25 ; Read next block? byte
    EXX                   ;      ; reinit statusline parameters
    LD HL, (SCREEN_START_POINTER)
    EX DE, HL
    INC DE
    LD HL, (SCREEN_START_POINTER)
    LD (HL), $FF
    EXX
    DEC B                 ;      ; 
    JR Z, READ_BLOCK      ;      ; If B==1, read next block
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;; End of load ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    DEC B                 ;      ; B contains next Block information
    JR Z, RET_BASIC       ;      ; If B==2, return to BASIC command line. Last block is BASIC code. HL contanis first free byte address
    DEC B
    JP Z, BASIC_RUN       ;      ; If B==3 BASIC RUN
    CALL GET_BYTE         ;      ; Else read load address
    LD E, B
    CALL GET_BYTE
    LD D, B
    PUSH DE               ;      ; load address in DE
    RET

ERROR:
    LD HL, ERROR_MSG
    CALL DSPMSG
    JR RET_BASIC

LOAD_SCREEN:
    CALL GET_BYTE         ; 4.25 ; Read Byte counter into DE
    LD E, B               ; 1    ;
    CALL GET_BYTE         ; 4.25 ;
    LD D, B               ; 1    ; Byte counter in DE
    LD HL, (SCREEN_START_POINTER)
    ADD HL, DE
    JR LOAD_COUNTER

BASIC_RUN:                ;      ; A Z flag 0
    CALL INIT_BASIC
    LD HL, $1D1E
    PUSH HL
    LD HL, (BASIC_PRG_START_POINTER)         ;      ; HL <- BASIC PROGRAM START ADDRESS
    XOR A
    CP 0                  ;      ; Set Z flag, for BASIC RUN
    JP BASIC_RUN_ADDR

RET_BASIC:
    CALL INIT_BASIC
;    CALL BASIC_CLEAR      ;      ; CALL BASIC CLEAR rutin
    JP $197E
    JP BASIC

INIT_BASIC0:
    LD (BASIC_PRG_END_POINTER), HL
    CALL ENABLED_INTERRUPTS
    RET

ENABLED_INTERRUPTS0:
    EI                    ; 1    ;;;;;;;; bedign EI ;;;;;;;;;;;;;;;;;;;;;
    LD A, (PORT_MIRROR)   ;      ;
    OR 128                ; 1.75 ; Set NMI bit
    OUT (PORT), A         ; 2.75 ;
    LD (PORT_MIRROR), A   ;      ;;;;;;;; end EI ;;;;;;;;;;;;;;;;;;;;;;;;
    RET

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; A küldött bájtok ideje 48kHz-en (76.8kHz) sample = 13,020833333us ~ 13us
;;; - 1 : [ HHHHLLHHHHLL ] = 12 samples = 156us
;;; - 0 : [ HHHHLLLLHHLL ] = 12 samples = 156us
;;; Bájt = 8 bit + [ HHHH ] = 100 samples = 1300us
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; Get byte into B register
;;; A : work register
;;; B : store for bits of byte
;;; C : sample counter
;;; 1 bit olvasásának ideje (4MHz):
;;; - Az első bevezető HIGH jel előtt: 7.75us
;;; - A bevezető HIGH jel feldolgozásának minimális hossza: 21.25us (1 körbefordulás)
;;; - LOW jel minimális hossza: 23.25us + n* 8.5us  Érkezik 26/52 - 10.75
;;; - HIGH jel minimális hossza: 23.25us + n* 8.5us Érkezik 26/52
;;; - Bit értékelés hossza: 7us
;;; - Byte visszatérés: 1.25us
;;; Az utolsó hasznos bit olvasása után 35.5us a következő pre-HIGH olvasásáig
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
GET_BYTE0:
    LD B, 128             ; 1.75  ; Set B.7 to 1. B : Byte
NEXT_BIT:
    LD C, 0               ; 1.75 ; Clear pulse counter
PRE_HIGH:                 ;      ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;; Drop first high samples
    IN A, (PORT)          ; 2.75 ;
    AND 4                 ; 1.75 ;
    JR NZ, PRE_HIGH       ; 3/1.75 ; = 7.5
    IN A, (PORT)          ; 2.75 ;;; Anti prill ;;;
    AND 4                 ; 1.75 ;
    JR NZ, PRE_HIGH       ; 3    ; = 7.5
LOW:                             ; Maximális bithossz 160us, ami 4MHz-en számolva 100us. C kb 10 vagy 11
    IN A, (PORT)          ; 2.75 ;
    INC C                 ; 1    ;
    AND 4                 ; 1.75 ;
    JR Z, LOW             ; 3    ; = 8.5
    IN A, (PORT)          ; 2.75 ;;; Anti prill ;;;
    AND 4                 ; 1.75 ;
    JR Z, LOW             ; 3    ; = 7.5
HIGH:
    IN A, (PORT)          ; 2.75 ;
    DEC C                 ; 1    ;
    AND 4                 ; 1.75 ; -9.5+
    JR NZ, HIGH           ; 3    ; = 8.5
    IN A, (PORT)          ; 2.75 ;;; Anti prill ;;;
    AND 4                 ; 1.75 ;
    JR NZ, HIGH           ; 3    ; = 8.5
                                 ; POST LOW
    RL C                  ; 2    ; CY=1, ha c.7=1, azaz HIGH > LOW
    RR B                  ; 2    ; CY->HL.7->HL.0->CY [ RR (HL) = 3.75 ]
    JR NC, NEXT_BIT       ; 3    ; = 8.75
;;; Byte in B                    ;
    RET                   ; 2.5  ; A == 0, C == *, B == Readed byte

ERROR_MSG:
    db $0C,"Load error! (Volume?)",$0    
LOADING_MSG:
    db $0C,$02,$0D,"                ",$0D,"is turbo loading",$01,$0
END:

GET_BYTE:                EQU     GET_BYTE0 - START + PUFFER_START
INIT_BASIC:              EQU     INIT_BASIC0 - START + PUFFER_START
ENABLED_INTERRUPTS:      EQU     ENABLED_INTERRUPTS0 - START + PUFFER_START
