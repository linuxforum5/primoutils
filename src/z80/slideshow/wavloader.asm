;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; 1 bit = 15.5us
;;; 1 byte = 124us + 16us | 8 bit + 16us ( -1.25 + 2.5 + 4.25 + 8.75 + 1.75 | 5.5 + 10.5 )
;;; ----p--------       ---p---   -------  | l,h = 50/100
;;;              ---t---       -e-         | e = 25   p = 75
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
PORT:                    EQU     $1F             ; 0-63 porttartomány képviselője. Ezen keresztül érhető el a kazettacsatlakozó
PORT_MIRROR:             EQU     $403B           ; Port mirror byte
; SYNC_BYTE:       EQU     $D3             ; A szinkronbájt értéke : 11010011
; LOAD_FROM:       EQU     $E800           ; Load from
; LOAD_COUNTER:    EQU     $1800           ; Loaded bytes counter
;DSPHND:                  EQU     $0015
SCREEN_START_POINTER:    EQU     $4039
; STATUSL:                 EQU     $E800           ; A bal felső sarok
; STATUS:                  EQU     $EA99           ; A státusz pozíciója
; BASIC:                   EQU     $1A33           ; Fejlesztési teljes romlista 15. oldal
; BASIC_PRG_START:         EQU     $43EA
BASIC_PRG_START_POINTER: EQU     $40A4
BASIC_PRG_END_POINTER:   EQU     $40F9
BASIC_RUN_ADDR:          EQU     $1EA3
BASIC_CMD_LINE_ADDR:     EQU     $197E
; BASIC_CLEAR:             EQU     $1E7A
; GET_BYTE:                EQU     $411C   ; $411C-$4151 = $35. 53 Bájt használható 49 bájtra van szükség. A legfelső elfoglalt bájt címe: $414D
DSPMSG:                  EQU     $3D03   ; ROM szövegkiíró rutin

                ORG $4400

START:
    LD HL, LOADING_MSG
    CALL DSPMSG

    DI                    ; 1    ;
    LD A, (PORT_MIRROR)   ; 1.75 ;
    AND 127               ; 1.75 ; Reset NMI bit
    OUT (PORT), A         ; 2.75 ; Disable NMI
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; Egy érték beolvasáaa 2.75us
;;; Egy bájton belül két bit olvasása között eltelő idő: 14us.
;;; A bájt uolsó bitje, és a következő bájt első bitje közötti idő: 14-3+1.75+2.5+7.75 = 23us
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;    LD HL, $E800             ; 2.5  ;
;    LD DE, $1800             ; 2.5  ;

READ_BLOCK:
    LD HL, ( SCREEN_START_POINTER )
    CALL GET_BYTE_INTO_A     ; 4.25 ;
    LD E, B
    CALL GET_BYTE_INTO_A     ; 4.25 ;
    LD D, B
    ADD HL, DE
    CALL GET_BYTE_INTO_A     ; 4.25 ;
    LD E, B
    CALL GET_BYTE_INTO_A     ; 4.25 ;
    LD D, B
;    LD DE, $1800             ; 2.5  ;
;;;;;;;;;;;; 3 bitnyi idő 33.75 - 17.74 = 16
READ_BLOCK_DATA:                    ; Egy bájt feldolgozásának ideje: 13.75us + 5.25us - 1.25us a GET_BYTE-on belül = 17.75us
    CALL GET_BYTE_INTO_A     ; 4.25 ;
    LD (HL), B               ; 1.75 ;
    INC HL                   ; 1    ;
    DEC DE                   ; 1    ;
    LD A, D                  ; 1.75 ;
    OR E                     ; 1    ;
    JR NZ, READ_BLOCK_DATA   ;3/1.75;

    CALL GET_BYTE_INTO_A     ; 4.25 ;
    DEC B
    JR Z, READ_BLOCK

    EI                       ; 1    ;;;;;;;; bedign EI ;;;;;;;;;;;;;;;;;;;;;
    LD A, (PORT_MIRROR)      ;      ;
    OUT (PORT), A            ; 2.75 ;
WAIT:
    IN A, (55)               ; Check RETURN.
    AND 1                    ; Primo füzetek - Szoftver 153. oldal
    JR Z, WAIT
    JP START

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; Read byte int A register
;;; BC és F elromlik
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
GET_BYTE_INTO_A:
    LD B, 128                ; 1.75 ;
NEXT_BIT:                           ; 1 bit olvasási ideje: 11.25us (18us) | 1 minta 14.2us | 15.625 | 11.57
    LD C, 0
SYNCH:                       ; Várakozás, a HIGH végére. Belső köridő: 6.75us Kilépés nél 5.5us. A bitet lezáró LOW jel kezdetétől ideáig 14.25us telik el Ennyi lehet maximum a LOW jel hossza
    IN A, (PORT)             ; 2.75 ; Read port data
    AND 4                    ; 1.75 ;
    JR NZ, SYNCH             ;3/1.75;
    IN A, (PORT)             ; 2.75 ; Read port data
    AND 4                    ; 1.75 ;
    JR NZ, SYNCH             ;3/1.75;
LOWCNT:
    IN A, (PORT)             ; 2.75 ; Read port data
    INC C
    AND 4
    JR Z, LOWCNT
    IN A, (PORT)             ; 2.75 ; Read port data
    AND 4                    ; 1.75 ;
    JR Z, LOWCNT
    LD A, 3                  ; 1.75 ; 4/2 esetén 3
    SUB C
    RR B                     ; 2    ; CY->(HL).7->(HL).0->CY
    JR NC, NEXT_BIT          ;3/1.75;
;    LD A, B                  ; 1    ; There is a time, because port is dummy low
    RET                      ; 2.5  ;
END_GET_BYTE_INTO_A:

LOADING_MSG:
    db $0C,$02,$0D,"Screen",$0D,"is turbo loading",$01,$0
END:

GET_BYTE_SIZE: EQU END_GET_BYTE_INTO_A-GET_BYTE_INTO_A
