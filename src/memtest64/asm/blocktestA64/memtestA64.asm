;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;   Primo A64 ASM memory test for z80asm or pasmo                 ;
;   2022.06.01.                                                   ;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
PORT:                    EQU     $1F             ; Port a magnókimenethez. 0-63 porttartomány képviselője. Ezen keresztül érhető el a kazettacsatlakozó
PORT_MIRROR:             EQU     $403B           ; Port mirror byte
PORT_BEEP:               EQU     $10             ; BEEP Port értéke AND 16
SCREEN_START_POINTER:    EQU     $4039

TEMPSTACK:       EQU     $FFFF
MEMSTART:        EQU     $4100           ; Start of memory to scan
MEMEND:          EQU     $E3FF           ; End of memory to scan

CR_LF:           EQU     0DH
LF:              EQU     0AH
CLS:             EQU     0CH             ; Clear screen

KEY_SPACE:       EQU      $19
KEY_LEFT:        EQU      $39
KEY_O:           EQU      $34
KEY_P:           EQU      $32
KEY_Q:           EQU      $0C
KEY_A:           EQU      $0E
KEY_R:           EQU      $14
KEY_BRK:         EQU      $3F
KEY_X:           EQU      $0A
KEY_B:           EQU      $1A
KEY_I:           EQU      $2E
KEY_K:           EQU      $2A
KEY_V:           EQU      $1E
KEY_C:           EQU      $10
KEY_RETURN:      EQU      55

PRINT_A:                 EQU     $15             ; ROM karakterkiíró rutin
DSPMSG_HL:               EQU     $3D03           ; ROM szövegkiíró rutin
CURSORPOS_DE:            EQU     $80             ; set cursor pos rom address D:sor, E:oszlop

                ORG $E400

    DI                    ; 1    ;
    LD A, (PORT_MIRROR)   ; 1.75 ;
    AND 127               ; 1.75 ; Reset NMI bit
    LD (PORT_MIRROR), A   ; 1.75 ;
    OUT (PORT), A         ; 2.75 ; Disable NMI
RESTART:
    LD HL, MAIN_SCREEN
    CALL DSPMSG_HL           ; print string

    LD HL, $4401
    LD DE, $4FFE             ; E400-4000
WRITE_RAM:
    LD (HL), L
    INC HL
    LD (HL), H
    INC HL
    DEC DE
    LD A, D
    OR E
    JR NZ, WRITE_RAM
    LD HL, $4401
    LD DE, $4FFE             ; E400-4000
    CALL SHOW_STATUS
CHECK_RAM:
    LD A, (HL)
    CP L
    CALL NZ, ERROR
    INC HL
    LD A, (HL)
    CP H
    CALL NZ, ERROR
    INC HL
    DEC DE
    LD A, D
    CP E
    CALL Z, SHOW_STATUS
    LD A, D
    OR E
    JR NZ, CHECK_RAM
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    LD A, (ERROR_COUNTER)
    CP 0
    JR NZ, IS_ERROR
    LD HL, END_SCREEN_OK
    CALL DSPMSG_HL            ; print string
    JR WAIT
IS_ERROR:
    LD HL, END_SCREEN_ERROR
    CALL DSPMSG_HL            ; print string
WAIT:
    IN A, ( KEY_BRK )
    AND 1
    JP NZ, 0
    IN A, ( KEY_RETURN )
    AND 1
    JR Z, WAIT
    JR RESTART

    LD A, 255
    LD (HL), A
    LD A, (HL)

ERROR:
    LD A, (ERROR_COUNTER)
    INC A
    RET Z
    LD (ERROR_COUNTER), A
    RET

CHECK_BYTE: ; HL=A cím, B=A vizsgált érték. Ha hiba van C-t növeli, de max 255-ig
    LD (HL), B
    LD A, (HL)
    CP B
    RET Z
    INC C
    RET NZ
    DEC C
    RET

SHOW_STATUS: ; A tesz állapot megjelenítése
;    PUSH DE
;    PUSH HL
    EXX
    ld de, $0200  ; 5. sor, 0. oszlop
    call CURSORPOS_DE       ; set cursot to (D,E)
    LD HL, STATUS1
    CALL DSPMSG_HL
    EXX
;    POP HL
;    PUSH HL
    LD A, H
    CALL write_hex_A
;    POP HL
;    PUSH HL
    LD A, L
    CALL write_hex_A
    EXX
    LD HL, STATUS2
    CALL DSPMSG_HL
    EXX
    LD A, (ERROR_COUNTER)
    CALL write_hex_A
;    POP HL
;    POP DE
    RET

write_hex_A:
    EXX
    LD B, A
    RRA
    RRA
    RRA
    RRA
    AND $0F
    CALL write_hex_pos_A
    LD A, B
    AND $0F
    CALL write_hex_pos_A
    EXX
    RET

write_hex_pos_A:
    LD HL, HEX
    LD D, 0
    LD E, A
    ADD HL, DE
    LD A,(HL)
    CALL PRINT_A
    RET

ERROR_COUNTER:    db 0
STATUS1:          db  6,"Addr.:",0
STATUS2:          db  " Err.:",0
HEX:              db  "0123456789ABCDEF"
MAIN_SCREEN:      db  CLS,"Primo A64 RAM test: 4400-E400",CR_LF,0
END_SCREEN_OK:    db  CR_LF,"All memories are OK",CR_LF,"Restart: Return",CR_LF,"Exit: BRK",0
END_SCREEN_ERROR: db  CR_LF,"Error(s) found",CR_LF,"Restart: Return",CR_LF,"Exit: BRK",0
