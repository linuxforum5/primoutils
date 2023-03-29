;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;   Primo A64 ASM memory test for z80asm or pasmo                 ;
;   2022.06.01.                                                   ;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
TEMPSTACK:       EQU     $FFFF
MEMSTART:        EQU     $4100           ; Start of memory to scan
MEMEND:          EQU     $E3FF           ; End of memory to scan
CURSORPOS_DE:    EQU     $80             ; set cursor pos rom address

CR:              EQU     0DH
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

                ORG $E400

                DI                       ; Disable interrupts
MAINMENU:    ;  LD HL,TEMPSTACK          ; Temp stack
             ;  LD SP,HL                 ; Set up a temporary stack
                LD HL, MENU              ; Sign-on message
                CALL PRINT_HL            ; print string
select:         IN A, ( KEY_B )          ; Check KEY B
                AND 1
                jp nz, BitTest
                IN A, ( KEY_C )          ; Check KEY C
                AND 1
                jp nz, AddressTest
                IN A, ( KEY_K )          ; check KEY K
                AND 1
                jr z, select
                JP 0                     ; EXIT

PRINT_HL:       LD       A,(HL)          ; Get character
                OR       A               ; Is it $00 ?
                RET      Z               ; Then RETurn on terminator
                CALL     15H             ; Print it
                INC      HL              ; Next Character
                JR       PRINT_HL        ; Continue until $00
                RET

output_pres:    INC A
                LD HL,HEX
next1:          INC HL
                DEC A
                JP NZ, next1
                LD A, (HL)
                CALL 15H
                RET

parse_hex_BC:   LD A, 6
                CALL 15H                ; Set write mode
                LD A, B
                RRA
                RRA
                RRA
                RRA
                AND 0fh
                call output_pres
                LD A, B
                AND 0fh
                call output_pres
                LD A, C
                RRA
                RRA
                RRA
                RRA
                AND 0fh
                call output_pres
                LD A, C
                AND 0fh
                call output_pres
                RET

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; Address test
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
AddrTest1:      LD H, B
                LD L, C
                LD (HL), 0ffh
                LD A, (HL)
                SUB 0ffh
                JP NZ, ADDR_ERROR
                LD (HL), 00h
                LD A, (HL)
                SUB 00h
                JP NZ, ADDR_ERROR
                LD (HL), 0aah
                LD A, (HL)
                SUB 0aah
                JP NZ, ADDR_ERROR
                LD (HL), 055h
                LD A, (HL)
                SUB 055h
                JP NZ, ADDR_ERROR
                LD A, 0                ; no error
                RET

AddrTest2:      LD H, B
                LD L, C
                LD A, (HL)
                SUB 055h
                JP NZ, ADDR_ERROR
                LD (HL), 0aah
                LD A, (HL)
                SUB 0aah
                JP NZ, ADDR_ERROR
                LD A, 0                ; no error
                RET

ADDR_ERROR:     LD HL, ADDR_ERR_COUNTER
                LD A, (HL)
                LD D, A
                SUB 96                  ; max errors
                ret z
                LD A, (HL)
                LD E, A
                SRA A
                SRA A
                SRA A
                ADD 3                  ; first row
                LD D, A
                LD A, E
                LD E, 0
                AND 7
                jr z, colok
tovabb:         INC E
                INC E
                INC E
                INC E
                INC E
                DEC A
                jr nz, tovabb
colok:          INC (HL)
                call CURSORPOS_DE       ; set cursot to (D,E)
                call parse_hex_BC
                RET

AddressTest:    LD HL, AddressScreen
                call PRINT_HL
                LD HL, ADDR_ERR_COUNTER
                LD (HL), 0

                ld de, $000F
                call CURSORPOS_DE       ; set cursot to (D,E)
                ld bc, MEMEND
                call parse_hex_BC

                ld de, $000A
                call CURSORPOS_DE       ; set cursot to (D,E)
                ld bc, MEMSTART
                call parse_hex_BC

                LD HL, NEWLINE
                call PRINT_HL
AddressLoop1:   ld hl, MEMEND
                sbc hl, bc
                jp z, AddressEnd1
                ld DE, 010AH
                ld a,255
                sub c
                call z, CurrentAddressPrintingBCDE
                call AddrTest1
                inc bc
                jp AddressLoop1
AddressEnd1:    ld bc, MEMSTART
AddressLoop2:   ld hl, MEMEND
                sbc hl, bc
                jp z, AddressEnd2
                ld DE, 0119H
                ld a,255
                sub c
                call z, CurrentAddressPrintingBCDE
                call AddrTest2
                inc bc
                jp AddressLoop2
AddressEnd2:    ld de, $0F00
                call CURSORPOS_DE       ; set cursot to (D,E)
                ld hl, ENDMSG
                ld a, (hl)
                call PRINT_HL

wait:           IN A, ( KEY_V )
                AND 1
                jp nz, MAINMENU
                jr wait

CurrentAddressPrintingBCDE:               ; print cureent memory address
                call CURSORPOS_DE       ; set cursot to (D,E)
                call parse_hex_BC
                RET
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; Bit test
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
BitTest:        LD HL, BitScreen
                call PRINT_HL

                ld de, $0010
                call CURSORPOS_DE       ; set cursot to (D,E)
                ld bc, MEMEND
                call parse_hex_BC

                ld de, $000B
                call CURSORPOS_DE       ; set cursot to (D,E)
                ld bc, MEMSTART
                call parse_hex_BC

                LD HL, NEWLINE
                call PRINT_HL
                LD HL, MEMORYBANK_ROW
                LD (HL), 3              ; Az első memóriabank sora a 3.
BitLoop:        ld hl, MEMEND
                sbc hl, bc
                jp z, BitEnd
                ld DE, 010EH
                ld a,255
                sub c
                call z, CurrentAddressPrintingBCDE

BitAddrSkip:    LD HL, BIT
                ld (HL), 1
                ld e, 3                 ; col
BitACheck:      LD A, (HL)
                call BitTestBCA
                LD HL, MEMORYBANK_ROW
                ld d, (HL)              ; row
                xor 0
                jr z, Bit_ok
                call CURSORPOS_DE       ; set cursot to (D,E)
                ld HL, HibasBit
                call PRINT_HL
Bit_ok:         LD HL, BIT
                inc e
                inc e
                inc e
                inc e
                sla (HL)
                jr nc, BitACheck
                INC BC
                LD A, $FF
                AND C
                JR nz, SKIP_RAM_BANK_INCREMENT	; Ha a C nem nulla, akkor biztos nem kell memóriabank indexet növelni, nem léptünk át új 16KB-os szegmensbe
                LD A,$3F
                AND B
                JR nz, SKIP_RAM_BANK_INCREMENT	; Ha a B alsó 4 bitje nem üres, akkor biztos nem kell memóriabank indexet növelni, nem léptünk át új 16KB-os szegmensbe
                LD HL, MEMORYBANK_ROW           ; Különben új memóriabankba léptünk
                INC (HL)                        ; Növeljük a megjelenítendő sort
SKIP_RAM_BANK_INCREMENT:
                IN A, ( KEY_K )
                AND 1
                jp z, BitLoop
BitEnd:         ld de, $0600
                call CURSORPOS_DE       ; set cursot to (D,E)
                ld hl, ENDMSG
                ld a, (hl)
                call PRINT_HL
                jp wait

BitTestBCA:     LD H, B                ; Parameters: BC=address, A=Bit value
                LD L, C
                LD D, A
                LD (HL), D             ; D-ben marad a jó bit
                LD A, (HL)             ; A-ban a kérdéses bit
                AND D                  ; A == 0 if bit is bad
                XOR D                  ; A != 0 if bit is bad
                RET NZ                 ; return if error
                LD A,D
                XOR $FF
                LD (HL), A
                LD A, (HL)
                AND D
                RET NZ                 ; return if error
                LD A,0                 ; set ok
                RET

BIT:            db     0
MEMORYBANK_ROW: db     0
MENU:           db     CLS,"Primo A64 memoriateszt",CR,LF,"B - Bitteszt",CR,LF,"C - Cimtesz (elso 10 hibaig)",CR,LF,"K - Kilepes",CR,LF,0
AddressScreen:  db     CLS,"Cimteszt (    -    )",CR,LF,"1. menet:      2. menet: ",CR,LF,"Hibas cimek (elso 96):",0
BitScreen:      db     CLS,"Bit teszt (    -    )",CR,LF,"Aktualis cim: ",CR,LF,"   0.  1.  2.  3.  4.  5.  6.  7.",CR,LF,"1.",CR,LF,"2.",CR,LF,"3.",0
HEX:            db    "_0123456789abcdef"
NEWLINE:        db    CR,LF,0
SPACE:          db    32,0
ENDMSG:         db    "Teszt vege: V - Vissza a fomenube.",0
HibasBit:       db    "E",0
ADDR_ERR_COUNTER: db 0
; 4 = inverz előtörlés
; 6 = normal előtörlés
