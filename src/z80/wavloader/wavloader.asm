;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; 1 bit = 15.5us
;;; 1 byte = 124us + 16us | 8 bit + 16us ( -1.25 + 2.5 + 4.25 + 8.75 + 1.75 | 5.5 + 10.5 )
;;; ----p--------       ---h---   -------  | l,h = 50/100
;;;              ---l---       -e-         | e = 25   p = 75
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
PORT:            EQU     $1F             ; 0-63 porttartomány képviselője. Ezen keresztül érhető el a kazettacsatlakozó
PORT_MIRROR:     EQU     $403B           ; Port mirror byte
; SYNC_BYTE:       EQU     $D3             ; A szinkronbájt értéke : 11010011
; LOAD_FROM:       EQU     $E800           ; Load from
; LOAD_COUNTER:    EQU     $1800           ; Loaded bytes counter
DSPHND:          EQU     $0015
STATUS:          EQU     $EA99           ; A státusz pozíciója

                ORG $E400                ; CALL( 58368, DE ) RETRUN HL

START:                    ; 4MHz ;
    LD HL, LOADING_MSG
WRTMSG:
    LD A,(HL)
    OR A
    JR Z, RUN
    CALL DSPHND
    INC HL
    JR WRTMSG

;    LD BC, LOAD_COUNTER-1 ;      ; CLS eleje
;    LD HL, LOAD_FROM      ;      ;
;    LD DE, LOAD_FROM+1    ;      ;
;    LD (HL), $AA          ;      ; Clear current byte on screen
;    LDIR                  ;      ; CLS vége

;    LD BC, LOAD_COUNTER-1 ;      ; CLS eleje
;    LD HL, LOAD_FROM      ;      ;
;    LD DE, LOAD_FROM+1    ;      ;
;    LD (HL), 0            ;      ; Clear current byte on screen
;    LDIR                  ;      ; CLS vége
RUN:
    DI                    ; 1    ;
    LD A, (PORT_MIRROR)   ;
    AND 127               ; 1.75 ; Reset NMI bit
    OUT (PORT), A         ; 2.75 ; 
    LD (PORT_MIRROR), A   ;      ;;;;;;;; end DI

;;; A tényleges betöltő modul. Innentől kezdve figyeli a magnójelet, olvassa a fájl elejét, a header-t.
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

;    CALL GET_BYTE         ; 4.25 ; Block type in B: 0=start block, 1=data block
;    LD A, B               ;      ;
;    CP 0
;    JR NZ, DATA_BLOCK     ;      ; If type == 0, then it is the start block. Start address in HL
;    PUSH HL
;    RET
;DATA_BLOCK:
    CALL GET_BYTE         ; 4.25 ;
    LD L, B               ; 1    ;
;    NOP
    CALL GET_BYTE         ; 4.25 ;
    LD H, B               ; 1    ; Run address after load
    PUSH HL               ; 2.75 ; For start: RET

    CALL GET_BYTE         ; 4.25 ;
    LD L, B               ; 1    ;
;    NOP
    CALL GET_BYTE         ; 4.25 ;
    LD H, B               ; 1    ; First byte address in HL
;    NOP

    CALL GET_BYTE         ; 4.25 ;
    LD E, B               ; 1    ;
;    NOP
    CALL GET_BYTE         ; 4.25 ;
    LD D, B               ; 1    ; Byte counter in DE
;    NOP

LOAD_DATA:
    CALL GET_BYTE         ; 4.25 ; +6.75-9.5
    LD (HL), B            ; 1.75 ; +1.75
    INC HL                ; 1.5  ;
    DEC DE                ; 1.5  ;
    LD A, D               ; 1    ;
    LD (STATUS), A        ; 3.25 ;
    OR E                  ; 1    ;
    JR NZ, LOAD_DATA      ; 3    ; +17.5 While DE != 0
                                 ; All data loaded

    EI                    ; 1    ;
    LD A, (PORT_MIRROR)   ;
    OR 128                ; 1.75 ; Set NMI bit
    OUT (PORT), A         ; 2.75 ; 
    LD (PORT_MIRROR), A   ;      ;;;;;;;; end EI

;WAIT:
;    IN A, (55)            ; Check RETURN key
;    AND 1                 ; Primo füzetek - Szoftver 153. oldal
;    JR Z, WAIT

;;;;;;;;;;;;;;;;;;;;;; Teljes betöltés utáni várakozás
;;    LD DE, 2000
;;POSTWAIT:
;;    DEC DE
;;    LD A, D
;;    OR E
;;    JR NZ, POSTWAIT

    RET	                  ; 2.5  ; Run start address
;    POP HL
;    JP START

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
GET_BYTE:
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
    RET                   ; 2.5  ;

LOADING_MSG:
    db $0C,$02,"                ",$0D,"is turbo loading",$0
