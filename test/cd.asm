; z80dasm 1.1.6
; command line: z80dasm -l -g 0x4500 cd.bin

	org	04500h

	jr l456ah
sub_4502h:
	ld de,(04174h)
	ld hl,00bb4h
	add hl,de	
	ld (l4530h),hl
	ld hl,(040b1h)
	ld (l4532h),hl
	ld hl,00084h
	add hl,de	
	push de	
	ld de,l4534h
	ld bc,00002h
	ldir
	pop de	
	ld hl,0055eh
	add hl,de	
	push de	
	ld de,l4536h
	ld bc,00003h
	ldir
	pop de	
	ret	
l4530h:
	nop	
	nop	
l4532h:
	nop	
	nop	
l4534h:
	nop	
	nop	
l4536h:
	nop	
l4537h:
	nop	
	nop	
l4539h:
	ld a,(de)	
	cp (hl)	
	ret nz	
	inc hl	
	inc de	
	djnz l4539h
	ret	
l4541h:
	push hl	
	ld hl,(040eah)
l4545h:
	call 02337h
l4548h:
	dec c	
	ld c,c	
	ld l,h	
	ld l,h	
	ld h,l	
	ld h,a	
	ld h,c	
	ld l,h	
	jr nz,l4594h
	ld b,c	
	ld d,e	
	ld c,c	
	ld b,e	
	jr nz,l45ceh
	ld h,l	
	ld (hl),d	
	ld (hl),e	
	ld l,c	
	ld l,a	
	ld l,(hl)	
	ld hl,0000dh
l4561h:
	ld hl,l4548h
	call 02b75h
	jp 0465bh
l456ah:
	ld hl,01a01h
	ld de,l4541h
	ld b,004h
	call l4539h
	jr nz,l4561h
	ld hl,02b1ch
	ld de,l4545h
	ld b,003h
	call l4539h
	jr nz,l4561h
	ld hl,(040b1h)
	ld (l4530h),hl
	ld de,0f44bh
	add hl,de	
	ld (l4532h),hl
	ld hl,(04174h)
l4594h:
	ld (l4534h),hl
	ld a,(041a6h)
	ld (l4536h),a
	ld hl,(041a7h)
	ld (l4537h),hl
	ld hl,(04174h)
	ld bc,00015h
	add hl,bc	
	ex de,hl	
	ld hl,l4698h
	add hl,bc	
	ld b,00ch
	call l4539h
	call z,sub_4502h
	ld de,044f5h
	ld hl,(l4530h)
	ld bc,00bb4h
	or a	
	sbc hl,bc
	push hl	
	pop iy
	ld hl,l524ch
	ld bc,00d57h
	dec hl	
l45cdh:
	inc hl	
l45ceh:
	scf	
l45cfh:
	rr (hl)
	jr z,l45cdh
	jr nc,l45e7h
	push bc	
	push hl	
	ld a,(de)	
	inc de	
	ld c,a	
	ld a,(de)	
	ld b,a	
	push iy
	pop hl	
	add hl,bc	
	ld a,h	
	ld (de),a	
	dec de	
	ld a,l	
	ld (de),a	
	pop hl	
	pop bc	
l45e7h:
	inc de	
	dec bc	
	ld a,b	
	or c	
	jr nz,l45cfh
	ld hl,(l4532h)
	ld (040b1h),hl
	ld de,00032h
	jr z,$+87
	ld b,001h
	ld sp,hl	
	ld (bc),a	
	or 045h
	nop	
	call 083cdh
	ld e,021h
	ld c,e	
	ld d,d	
	ld de,(l4530h)
	dec de	
	ld bc,00bb4h
	lddr
	ld hl,(l4534h)
	ld (00084h),hl
	ld a,(l4536h)
	ld (0055eh),a
	ld hl,(l4537h)
	ld (0055fh),hl
	ld hl,00000h
	ld (04174h),hl
	ld hl,0044ch
	ld (04162h),hl
	ld a,0c3h
	ld (041a6h),a
	ld (041c1h),a
	ld (041beh),a
	ld hl,00541h
	ld (041a7h),hl
	ld hl,004b4h
	ld (041c2h),hl
	ld hl,004dah
	ld (041bfh),hl
	ld hl,00b32h
	ld (0400dh),hl
	ld a,008h
	ld (00b86h),a
	xor a	
	ld (00b87h),a
	ld (00b88h),a
	out (080h),a
	ld hl,04661h
	call 02b75h
	call 01b4dh
	jp 01a19h
	inc c	
	ld b,e	
	ld l,a	
	ld l,l	
	ld l,l	
	ld l,a	
	ld h,h	
	ld l,a	
	ld (hl),d	
	ld h,l	
	jr nz,$+85
	ld h,l	
	ld (hl),d	
	ld l,c	
	ld h,c	
	ld l,h	
	jr nz,l46c0h
	ld (hl),l	
	ld (hl),e	
	jr nz,$+74
	ld h,c	
	ld l,(hl)	
	ld h,h	
	ld l,h	
	ld h,l	
	ld (hl),d	
	jr nz,l46bbh
	ld l,030h
	dec c	
	jr z,$+101
	add hl,hl	
	jr nz,$+68
	ld c,l	
	ld b,l	
	dec l	
	ld c,e	
	ld d,e	
	ld e,d	
l4698h:
	ld c,e	
	inc l	
	jr nz,l46cdh
	add hl,sp	
	jr c,$+55
	ld l,00dh
	nop	
	jp z,00083h
	push af	
	xor a	
	ld (0062ch),a
	ld (00630h),a
	ld bc,00b32h
	ld (0400dh),bc
	ld a,(005fah)
	ld c,a	
	ld a,006h
	rst 28h	
l46bbh:
	ld c,000h
	ld a,00ah
	rst 28h	
l46c0h:
	pop af	
	cp 0a7h
	jp z,00086h
	cp 024h
	jp z,000eeh
	cp 0b5h
l46cdh:
	jp z,0015ah
	cp 0afh
	jp z,00471h
	cp 0a9h
	jp z,00197h
	cp 0adh
	jp z,0021bh
	cp 0bbh
	jp z,00271h
	cp 0aah
	jp z,00288h
	cp 09eh
	jp z,001e6h
	cp 083h
	jp z,0029fh
	cp 082h
	jp z,0028eh
	cp 0a2h
	jp z,002d1h
	cp 0abh
	jp z,002d4h
	cp 0b3h
	jp z,002d7h
	cp 0b0h
	jp z,002dah
	cp 0a6h
	jp z,00339h
	cp 0b8h
	jp z,00353h
	cp 0b2h
	jp z,0047bh
	cp 089h
	jp z,003afh
	cp 0a4h
	jp z,00375h
	jp 01997h
	rst 10h	
	cp 0aeh
	call z,00010h
	call 02337h
	push hl	
	ld hl,0ffech
	add hl,sp	
	ld de,004e9h
	ld a,002h
	rst 28h	
	xor a	
	ld (005c9h),a
	call 0051bh
	ld de,(040a4h)
	ld c,01fh
	ld a,010h
	rst 28h	
	jp c,001c6h
	jr z,l476fh
	pop hl	
	cp 0d5h
	ret z	
l4755h:
	ld (040f9h),de
	call 01af8h
	ld hl,(040a2h)
	ld a,h	
	and l	
	inc a	
	ld hl,01d1eh
	push hl	
	jp nz,01b5dh
	call 01b5dh
	jp 01a33h
l476fh:
	cp 063h
	jr nz,l477bh
	ld b,a	
	ld a,(005c9h)
	or a	
	jr nz,l4755h
	ld a,b	
l477bh:
	jp 001cfh
l477eh:
	call 02337h
	push hl	
	call 0050fh
	ld de,00606h
	call 00527h
	call 0051eh
	jr l4797h
	rst 10h	
	jr nz,l477eh
	push hl	
	ld hl,00604h
l4797h:
	push hl	
	ld hl,0ffech
	add hl,sp	
	ld de,004e9h
	ld a,002h
	rst 28h	
	pop hl	
	ld de,(040fdh)
	ld c,000h
	ld a,010h
	rst 28h	
	jp c,001c6h
	cp 063h
	jp nz,001cfh
	ld hl,(040fdh)
	ld bc,0ffffh
	call 00127h
l47bdh:
	ld hl,(040a2h)
	inc hl	
	ld a,h	
	or l	
	pop hl	
	ret nz	
	pop af	
	jp 01a19h
l47c9h:
	ld a,(hl)	
	inc hl	
	or (hl)	
	inc hl	
	ret z	
	ld e,(hl)	
	inc hl	
	ld d,(hl)	
	inc hl	
	ex de,hl	
	sbc hl,bc
	add hl,bc	
	jr z,l47d9h
	ret nc	
l47d9h:
	call 0014dh
	ex de,hl	
	push bc	
	call 02b7eh
	pop bc	
	push hl	
	ld hl,(040a7h)
	call 02b75h
	call 0065fh
	pop hl	
	jr l47c9h
	push de	
	push bc	
	call 00fafh
	ld a,020h
	call 0032ah
	pop bc	
	pop de	
	ret	
	rst 10h	
	call 01b10h
	ex (sp),hl	
	push hl	
	push bc	
	call 0017bh
	jp c,001c6h
	ld (0409ch),a
	pop hl	
	pop bc	
	call 00127h
	ld a,028h
	rst 28h	
	xor a	
	ld (00631h),a
	ld (0409ch),a
	jr l47bdh
	ld a,(005fbh)
	ld c,a	
	ld a,020h
	rst 28h	
	ret c	
	ld c,007h
	ld a,024h
	rst 28h	
	ld a,00ch
	rst 28h	
	add a,a	
	ld a,000h
	ld (00632h),a
	ld a,010h
	ld (00631h),a
	ret	
	rst 10h	
	cp 0aeh
	call z,00010h
	call 02337h
	push hl	
	ld de,004f7h
	ld a,002h
	rst 28h	
	call 0051bh
	ld de,(040a4h)
	ld c,00eh
	ld a,012h
	rst 28h	
	jr c,l4862h
	jr z,l4871h
	ld (00630h),a
	or a	
	jp nz,00447h
	pop hl	
	ret	
l4862h:
	ld a,(0062ch)
	or a	
	jr nz,l486ah
l4868h:
	ld a,064h
l486ah:
	ld e,a	
	call 004dah
	jp 019a2h
l4871h:
	add a,064h
	jr c,l486ah
	ld (0062ch),a
	call 00586h
	jr l486ah
	rst 28h	
	jr c,l4868h
	jr z,l4871h
	cp 001h
	call z,00586h
	ret	
	rst 10h	
	push hl	
	push af	
	ld a,(0062dh)
	or a	
	jr z,l4896h
	call 00591h
	jr l489bh
l4896h:
	ld a,01ah
	rst 28h	
	jr c,l4868h
l489bh:
	pop af	
	jr z,l48b3h
	push hl	
	call 02828h
	pop de	
	ld hl,(040a7h)
l48a6h:
	ld a,(de)	
	inc de	
	ld (hl),a	
	inc hl	
	or a	
	jr nz,l48a6h
	ld (040a9h),a
	jp 021c3h
l48b3h:
	push hl	
	call 020f9h
	pop hl	
	call 028a7h
	pop hl	
	ret	
	ld de,005c9h
	ld (0060fh),de
	rst 10h	
	cp 0aeh
	jr z,l48edh
	cp 0b7h
	jr nz,l48d5h
	rst 10h	
	ld de,005c2h
	ld (0060fh),de
l48d5h:
	push hl	
	ld a,0d1h
	ld (005c9h),a
	ld hl,(040a4h)
	ld (005ceh),hl
	ex de,hl	
	ld hl,(040f9h)
	or a	
	sbc hl,de
	ld (005cch),hl
	jr l4900h
l48edh:
	rst 10h	
	push hl	
	ld a,0d5h
	ld (005c9h),a
	ld hl,01800h
	ld (005cch),hl
	ld h,(ix+008h)
	ld (005ceh),hl
l4900h:
	pop hl	
l4901h:
	call 02337h
	push hl	
	call 0051bh
	ld de,(0060fh)
	ld a,00eh
l490eh:
	call 001dbh
	pop hl	
	ret	
	rst 10h	
	ld de,005fch
l4917h:
	call 0050fh
	push hl	
	call 00527h
	pop hl	
	call 02337h
	push hl	
	call 0051eh
l4926h:
	ld a,018h
	jr l490eh
	rst 10h	
	ld de,005ffh
	jr l4917h
	rst 10h	
	jp nz,01997h
	xor a	
	ld (0062dh),a
	call 00354h
	push hl	
	ld hl,00602h
	jr l4926h
	rst 10h	
	cp 02ch
	jr z,l495ah
	call 02b1ch
	cp 008h
	jp c,01e4ah
	cp 00ch
	jp nc,01e4ah
	ld (005fah),a
	ld a,(hl)	
	cp 02ch
	ret nz	
l495ah:
	rst 10h	
	call 02b1ch
	cp 004h
	jr z,l4967h
	cp 005h
	jp nz,01e4ah
l4967h:
	ld (005fbh),a
	ret	
l496bh:
	ld e,05ch
	ld bc,05a1eh
	jp 019a2h
	ld a,052h
	ld bc,0573eh
	ld bc,0413eh
	ld bc,0443eh
	push af	
	rst 10h	
	call 0059bh
	jr z,l496bh
	ld c,000h
	call 005ach
	jr nz,$-28
	push de	
	rst 8	
	inc l	
	call 02337h
	ld (0060fh),hl
	call 0051bh
	pop de	
	pop af	
	push af	
	cp 044h
	jr z,l49afh
	push de	
	ld de,00609h
	call 00527h
	pop de	
	pop af	
	push af	
	ld bc,(0060dh)
	ld (bc),a	
	inc (hl)	
l49afh:
	ld bc,(005f9h)
	ld a,014h
	call 001dbh
	ld hl,00634h
	ld a,02ch
l49bdh:
	cp (hl)	
	inc hl	
	jr nz,l49bdh
	ld a,(hl)	
	cp 04ch
	jr nz,l49c8h
	pop bc	
	push af	
l49c8h:
	pop af	
	ld (de),a	
	ld a,(005f9h)
	inc de	
	ld (de),a	
	xor a	
	inc de	
	ld (de),a	
	ld a,(005fah)
	inc de	
	ld (de),a	
	ld hl,(0060fh)
	ret	
	rst 10h	
	call 0059bh
	ret nz	
	ex de,hl	
	ld (hl),080h
	inc hl	
	ld (hl),000h
	inc hl	
	inc hl	
	push bc	
	ld c,(hl)	
	ld a,006h
	rst 28h	
	pop bc	
l49eeh:
	ld a,016h
	call 001dbh
	ex de,hl	
	ret	
	rst 10h	
	ex de,hl	
	ld hl,005d1h
	ld b,005h
	ld a,(005fah)
l49ffh:
	inc hl	
	inc hl	
	inc hl	
	cp (hl)	
	call z,0036bh
	inc hl	
	djnz l49ffh
	ld c,00fh
	jr l49eeh
	push hl	
	dec hl	
	dec hl	
	ld (hl),000h
	dec hl	
	ld (hl),080h
	pop hl	
	ret	
	rst 10h	
	cp 023h
	call z,003d3h
	call nz,003e9h
l4a20h:
	call 0260dh
	push hl	
	rst 20h	
	jp nz,00af6h
	push af	
	push de	
	ld de,(040a7h)
	ld (005beh),de
	call 00426h
	ld de,005bdh
	call 02888h
	pop hl	
	pop af	
	call 003abh
	pop hl	
	ld a,(hl)	
	cp 02ch
	jr nz,l4a49h
	rst 10h	
	jr l4a20h
l4a49h:
	ld a,02ah
	rst 28h	
	ret	
	push hl	
	jp 01f33h
	rst 10h	
	call 02828h
	call 003d3h
	push hl	
	ld b,0fah
	ld de,(040a7h)
l4a5fh:
	call 00426h
	ld a,(de)	
	cp 00dh
	jr z,l4a6ah
	inc de	
	djnz l4a5fh
l4a6ah:
	xor a	
	ld (de),a	
	ld (040a9h),a
	ld a,02ah
	rst 28h	
	jp 021c3h
	rst 8	
	inc hl	
	call 0059bh
	jr nz,$+108
	rst 8	
	inc l	
	call 0040fh
	jr nz,$+104
	ld (0060fh),de
	inc de	
	ld a,(de)	
	jr l4a9ch
	ld de,00633h
	ld (0060fh),de
	xor a	
	ld (de),a	
	ld a,00fh
	ld (005f9h),a
	ld a,(005fah)
l4a9ch:
	ld c,a	
	ld a,022h
	rst 28h	
	jp c,001c6h
	ld bc,(005f9h)
	ld a,026h
	rst 28h	
	cp a	
	ret	
	ld a,(de)	
	sub 04ch
	jr z,l4abeh
	ld a,(de)	
	cp 04ch
	jr z,l4abeh
	cp 044h
	jr z,l4abeh
	cp 052h
	jr nz,l4ae3h
l4abeh:
	inc de	
	inc de	
	sub 04ch
	jr nz,l4ac5h
	ld (de),a	
l4ac5h:
	ld a,(de)	
	or a	
	ret	
	ld hl,(0060fh)
	ld a,(hl)	
	or a	
	jr nz,$+28
	ld a,01ch
	push bc	
	rst 28h	
	pop bc	
	jp c,001c6h
	ld (de),a	
	ld a,00ch
	rst 28h	
	and 040h
	ret z	
	ld b,001h
	ld (hl),0ffh
	ret	
l4ae3h:
	ld e,060h
	ld bc,05e1eh
	ld bc,02a1eh
	jp 019a2h
	call 00b37h
	ld hl,(04121h)
	ld a,h	
	or a	
	jp nz,01e4ah
	ld a,l	
	cp 002h
	jp c,01e4ah
	cp 00fh
	jp nc,01e4ah
	ld c,l	
	call 005ach
	jr nz,$-34
	call 0040ah
	ld h,a	
	ld l,a	
	ld (04121h),hl
	ret	
	rst 10h	
	call 0017bh
	jp c,001c6h
	jp 02098h
	rst 10h	
	cp 023h
	jr z,l4b2ch
	ld a,00fh
	ld (005f9h),a
	ld a,(005fah)
	jr l4b41h
l4b2ch:
	rst 10h	
	call 0059bh
	jr nz,$-74
	dec hl	
	rst 10h	
	jr z,l4b38h
	rst 8	
	inc l	
l4b38h:
	ld a,(de)	
	cp 052h
	jr z,l4ae3h
	inc de	
	inc de	
	inc de	
	ld a,(de)	
l4b41h:
	ld c,a	
	ld a,020h
	rst 28h	
	jr c,l4b63h
	ld bc,(005f9h)
	ld a,024h
	rst 28h	
	ld a,090h
	ld (00631h),a
	jp 02098h
	ld a,(00631h)
	or a	
	ret z	
	call p,004c8h
	jr z,l4b66h
	ld a,01eh
	rst 28h	
l4b63h:
	jp c,001c6h
l4b66h:
	ld a,c	
	pop bc	
	pop bc	
	ret	
	ld a,(00632h)
	inc c	
	dec c	
	jr nz,l4b76h
	cpl	
	ld (00632h),a
	ret	
l4b76h:
	or a	
	ret nz	
	ld a,02ch
	rst 28h	
	ret	
	ld a,(00631h)
	or a	
	ret z	
	push bc	
	ld a,028h
	rst 28h	
	pop bc	
	xor a	
	ld (00631h),a
	ret	
	ld a,01ch
	rst 28h	
	ld a,0ddh
	ld (005c9h),a
	ld hl,0ffffh
	ld c,020h
	ret	
	cp 0d9h
	jr z,l4ba4h
l4b9dh:
	ld a,02ah
	ld (0062ch),a
	scf	
	ret	
l4ba4h:
	ld h,007h
l4ba6h:
	ld a,01ch
	rst 28h	
	jr c,l4b9dh
	dec h	
	jr nz,l4ba6h
	ld c,002h
	ret	
	push hl	
	ld hl,00634h
	ld (hl),000h
	inc hl	
	ld (0060dh),hl
	pop hl	
	ret	
	call 0050fh
	call 029d7h
	ld a,(hl)	
	inc hl	
	ld e,(hl)	
	inc hl	
	ld d,(hl)	
	ld bc,0131ah
	or a	
	jr z,l4bddh
	push af	
	ld b,000h
	ld c,a	
	ld hl,(0060dh)
	ex de,hl	
	ldir
	ld (0060dh),de
	pop af	
l4bddh:
	ld hl,00634h
	add a,(hl)	
	ld (hl),a	
	ret	
	push af	
	ld a,e	
	cp 05ah
	jr nc,l4c03h
	cp 02ah
	jr nz,l4bffh
	ld a,(00630h)
	or a	
	jr z,l4bffh
	push hl	
	ld h,000h
	ld l,a	
	call 0014dh
	pop hl	
	xor a	
	ld (00630h),a
l4bffh:
	pop af	
	ret	
	nop	
	nop	
l4c03h:
	cp 064h
	jr z,l4c1bh
	jr c,l4c16h
	pop af	
	pop af	
	ld a,007h
	call 0032ah
	call 00591h
	jp 01a01h
l4c16h:
	ld hl,005b7h
	pop af	
	ret	
l4c1bh:
	pop af	
	pop af	
	ld a,007h
	call 0032ah
	ld hl,00619h
	jp 01a01h
	ld (0062eh),hl
	push af	
	ld a,001h
	ld (0062dh),a
	pop af	
	ret	
	ld hl,(0062eh)
	push af	
	xor a	
	ld (0062dh),a
	pop af	
	ret	
	call 02b1ch
	cp 002h
	jp c,01e4ah
	cp 00fh
	jp nc,01e4ah
	ld c,a	
	ld (005f9h),a
	ld de,005d1h
	ld b,00ah
l4c53h:
	inc de	
	ld a,(de)	
	dec de	
	cp c	
	ret z	
	inc de	
	inc de	
	inc de	
	inc de	
	djnz l4c53h
	ret	
	ld bc,00000h
	nop	
	nop	
	exx	
	and d	
	ld b,b	
	ld (bc),a	
	nop	
	ret nz	
	dec b	
	nop	
	nop	
	nop	
	nop	
	nop	
	nop	
	nop	
	ret	
	add a,b	
	nop	
	nop	
	nop	
	add a,b	
	nop	
	nop	
	nop	
	add a,b	
	nop	
	nop	
	nop	
	add a,b	
	nop	
	nop	
	nop	
	add a,b	
	nop	
	nop	
	nop	
	add a,b	
	nop	
	nop	
	nop	
	add a,b	
	nop	
	nop	
	nop	
	add a,b	
	nop	
	nop	
	nop	
	add a,b	
	nop	
	nop	
	nop	
	add a,b	
	nop	
	nop	
	nop	
	nop	
	ex af,af'	
	inc b	
	ld (bc),a	
	ld c,(hl)	
	ld a,(l5302h)
	ld a,(l4901h)
	ld bc,00224h
	inc h	
	ld a,(02c03h)
	ld d,e	
	inc l	
	nop	
	nop	
	nop	
	nop	
	ld b,d	
	ld c,a	
	ld b,(hl)	
	ld c,a	
	ld c,(hl)	
	ld c,a	
	ld b,d	
	ld b,h	
	ld b,h	
	ld b,l	
	ld d,(hl)	
	ld c,c	
	ld b,e	
	ld b,l	
	jr nz,$+80
	ld c,a	
	ld d,h	
	jr nz,l4d17h
	ld d,d	
	ld b,l	
	ld d,e	
	ld b,l	
	ld c,(hl)	
	ld d,h	
	nop	
	nop	
	nop	
	nop	
	nop	
	nop	
	nop	
	nop	
	nop	
	nop	
	nop	
	nop	
	nop	
	nop	
	nop	
	nop	
	nop	
	nop	
	nop	
	nop	
	nop	
	nop	
	nop	
	nop	
	nop	
	nop	
	nop	
	nop	
	nop	
	nop	
	nop	
	nop	
	nop	
	nop	
	nop	
	nop	
	nop	
	nop	
	nop	
	ld a,010h
	ld hl,00651h
	ret	
	ld (00b8dh),de
	ld (00b8fh),hl
	ret	
	ld hl,(00b8fh)
	ld de,(00b8dh)
	ret	
	ld a,c	
	ld (00b86h),a
	ret	
	ld a,(00b86h)
	ret	
	ld a,c	
	ld (00b87h),a
l4d17h:
	ret	
	ld a,(00b87h)
	ret	
	ei	
	ld a,(0403bh)
	or 080h
	ld (0403bh),a
	out (000h),a
	ret	
	di	
	ld a,(0403bh)
	and 07fh
	ld (0403bh),a
	out (000h),a
	ret	
l4d34h:
	in a,(040h)
	and 010h
	jr z,l4d41h
	in a,(040h)
	and 010h
	ret nz	
	jr l4d34h
l4d41h:
	in a,(040h)
	and 010h
	ret z	
	jr l4d34h
l4d48h:
	in a,(000h)
	and 002h
	call nz,0067ah
	in a,(040h)
	and 010h
	jr z,l4d48h
	in a,(040h)
	and 010h
	scf	
	ret nz	
	jr l4d48h
l4d5dh:
	in a,(040h)
	and 010h
	jr nz,l4d5dh
	in a,(040h)
	and 010h
	ret z	
	jr l4d5dh
l4d6ah:
	in a,(040h)
	and 020h
	jr z,l4d6ah
	in a,(040h)
	bit 5,a
	ret nz	
	jr l4d6ah
	ld a,(00b88h)
	and 0dfh
l4d7ch:
	out (064h),a
	ld (00b88h),a
	ret	
	ld a,(00b88h)
	or 020h
	jr l4d7ch
l4d89h:
	ld a,(00b88h)
	and 0efh
	jr l4d7ch
	ld a,(00b88h)
	or 010h
	jr l4d7ch
	ld a,(00b88h)
	and 0fdh
	jr l4d7ch
	ld a,(00b88h)
	or 002h
	jr l4d7ch
	call 00686h
	call 006e0h
	call 006fch
	ld a,05fh
	ld bc,03f3eh
	call 0072ah
	call 006f5h
	ld b,00ah
l4dbbh:
	djnz l4dbbh
	call 006d5h
	jr l4d89h
	ld a,040h
	jr l4dc8h
	ld a,020h
l4dc8h:
	or c	
	push af	
	jr l4dd5h
	push af	
	ld a,(00b89h)
	or a	
	scf	
	call nz,007b2h
l4dd5h:
	xor a	
	ld (00b89h),a
	pop af	
	ld (00b8ah),a
	call 00686h
	call 006e7h
	call 006fch
	call 00686h
	call 006e0h
	call 006e7h
	ld b,038h
l4df1h:
	ld c,002h
l4df3h:
	in a,(040h)
	and 010h
	jr z,l4dfeh
	djnz l4df1h
	jp 00861h
l4dfeh:
	dec c	
	jr nz,l4df3h
	jr l4e54h
	ld a,c	
	or 060h
	ld (00b8ah),a
	call 00744h
	ret c	
	call 00686h
	call 006eeh
	call 006f5h
	ld b,005h
l4e18h:
	djnz l4e18h
	call 006d5h
l4e1dh:
	in a,(000h)
	and 002h
	call nz,0067ah
	in a,(040h)
	and 020h
	jr nz,l4e1dh
	in a,(040h)
	and 020h
	jr nz,l4e1dh
	call 0067ah
	ret	
	ld a,c	
	or 060h
	ld (00b8ah),a
	call 00744h
	jp 006f5h
	ld a,(00b89h)
	or a	
	push bc	
	call nz,007b2h
	pop bc	
	ret c	
	ld a,0ffh
	ld (00b89h),a
	ld a,c	
	ld (00b8ah),a
	ret	
l4e54h:
	push af	
	call 00686h
	call 006e7h
	call 00692h
	jp nz,00860h
	call 006d5h
	pop af	
	call c,006a6h
	call c,006bbh
	call 006a6h
	call 006e0h
	ld bc,(00b8ah)
	ld b,008h
l4e77h:
	call 00692h
	jp z,00865h
	srl c
	call nc,006eeh
	call c,006e7h
	call 006d5h
	call 006e0h
	call 006e7h
	djnz l4e77h
	ld b,01bh
l4e92h:
	call 00692h
	jr z,l4e9bh
	djnz l4e92h
	jr l4f07h
l4e9bh:
	call 0067ah
	ret	
	call 00686h
	ld c,000h
	call 006d5h
	call 006c8h
l4eaah:
	call 006e7h
	ld b,00fh
l4eafh:
	in a,(040h)
	and 020h
	jr z,l4ed3h
	in a,(040h)
	and 020h
	jr z,l4eafh
	djnz l4eafh
	dec c	
	jp p,00865h
	call 006eeh
	call 006d5h
	ld a,(00b87h)
	or 040h
	ld (00b87h),a
	inc c	
	inc c	
	jr l4eaah
l4ed3h:
	in a,(040h)
	and 020h
	jr nz,l4eafh
	ld b,008h
l4edbh:
	call 006c8h
	add a,a	
	add a,a	
	add a,a	
	add a,a	
	rr c
l4ee4h:
	in a,(040h)
	and 020h
	jr nz,l4ee4h
	in a,(040h)
	and 020h
	jr nz,l4ee4h
	djnz l4edbh
	call 006eeh
	ld a,(00b87h)
	bit 6,a
	call nz,00717h
	call 0067ah
	ld a,c	
	ret	
	pop af	
	ld a,080h
	jr l4f09h
l4f07h:
	ld a,003h
l4f09h:
	push hl	
	ld hl,00b87h
	or (hl)	
	ld (hl),a	
	call 0067ah
	call 00714h
	ld a,(hl)	
	pop hl	
	scf	
	ret	
	ld a,c	
	and 0efh
	or 0e0h
	ld c,a	
	push bc	
	ld bc,(00b86h)
	ld a,020h
	rst 28h	
	pop bc	
	ld a,024h
	rst 28h	
	ld a,028h
	rst 28h	
	ld a,(00b87h)
	ld b,a	
	and 0bfh
	ld a,b	
	jr l4f5ch
	ld a,c	
	or 0f0h
	ld c,a	
	push bc	
	xor a	
	ld (00b87h),a
	ld bc,(00b86h)
	ld a,020h
	rst 28h	
	pop bc	
	ld a,024h
	rst 28h	
	ld a,(00b87h)
	or a	
	scf	
	ret nz	
	call 008bfh
	ret c	
	ld a,028h
	rst 28h	
	ld a,(00b87h)
	or a	
l4f5ch:
	scf	
	ret nz	
	jp 00aa1h
	ld a,(hl)	
	inc hl	
	or a	
	ret z	
	ld b,a	
l4f66h:
	ld c,(hl)	
	inc hl	
	ld a,01eh
	rst 28h	
	ret c	
	djnz l4f66h
	ret	
	xor a	
	ld (00b8bh),a
	ld a,c	
	ld (00b93h),a
	ld (00b91h),de
	ld c,000h
	ld a,014h
	rst 28h	
	ret c	
	ret z	
	ld bc,(00b86h)
	ld a,022h
	rst 28h	
	ret c	
	ld c,000h
	ld a,026h
	rst 28h	
	call 00937h
	jr nc,l4f99h
	ld a,01ah
	rst 28h	
	ret c	
	ret z	
l4f99h:
	ld a,02ah
	rst 28h	
	ld c,000h
	ld a,016h
	rst 28h	
	ret c	
	ret z	
	ld a,(00b94h)
	ret	
l4fa7h:
	bit 4,c
	jr z,l5005h
	call 009e8h
	ret c	
	ld de,(00b91h)
	ld bc,00932h
	push bc	
l4fb7h:
	ld a,(00b8bh)
	or a	
	ret nz	
	jp (hl)	
l4fbdh:
	bit 3,c
	jr z,l5005h
	call 009e8h
	ret c	
	push hl	
	call 008f7h
	jr c,l4fd0h
	jr z,l4fd0h
	pop hl	
	jr l4fb7h
l4fd0h:
	pop bc	
	pop bc	
	ret	
l4fd3h:
	ret	
	ld (00b91h),de
l4fd8h:
	ex de,hl	
	ld a,01ch
	rst 28h	
	ret c	
	ld bc,(00b93h)
l4fe1h:
	cp 0cdh
	jr z,l4fa7h
	cp 0c3h
	jr z,l4fbdh
	cp 0c9h
	jr z,l4fd3h
	ld (00b94h),a
	cp 0d9h
	jr z,l501eh
	cp 0d5h
	jr z,$+46
	cp 0d1h
	jr z,$+39
	cp 0ddh
	jr nz,l5005h
	bit 5,c
	ex de,hl	
	jr nz,l502fh
l5005h:
	ld hl,0096fh
	push hl	
	ld hl,(00b8dh)
	ld de,(00b91h)
	jp (hl)	
	ld (00b91h),de
	jr nc,l4fe1h
	pop bc	
	call 008f7h
	ret z	
	scf	
	ret	
l501eh:
	bit 0,c
	ld hl,049cbh
	ld hl,l51cbh
	jr z,l5005h
	call 009e8h
	call nc,009deh
	ret c	
l502fh:
	ld a,01ch
	rst 28h	
	ret c	
	ld b,a	
	ld a,(00b8bh)
	or a	
	jr nz,l5071h
	ld a,h	
	cp (ix+008h)
	ld a,b	
	jr nc,l504bh
	ld bc,(00b8fh)
	or a	
	sbc hl,bc
	add hl,bc	
	jr nc,l5067h
l504bh:
	ld (hl),a	
l504ch:
	inc hl	
	dec de	
	ld a,d	
	or e	
	jr z,l4fd8h
	ld a,(00b87h)
	bit 6,a
	jr z,l502fh
	pop bc	
	ex de,hl	
	call 008f7h
	ret c	
	ret z	
	ld hl,00b71h
	ld a,063h
	cp a	
	ret	
l5067h:
	pop bc	
	call 008f7h
	ret c	
	ret z	
	ld a,0a8h
	cp a	
	ret	
l5071h:
	ld a,b	
	cp (hl)	
	jr z,l504ch
	ld a,(00b8ch)
	inc a	
	jr z,l504ch
	ld (00b8ch),a
	jr l504ch
	ld a,01ch
	rst 28h	
	ret c	
	ld e,a	
	ld a,01ch
	rst 28h	
	ld d,a	
	ret	
	call 009deh
	ret c	
	ld a,(00b94h)
	cp 0d9h
	ld h,d	
	ld l,e	
	ret z	
	ld hl,(00b91h)
	add hl,de	
	cp 0d1h
	ret z	
	ld h,(ix+008h)
	ld l,000h
	add hl,de	
	and a	
	ret	
	xor a	
	ld (00b8ch),a
	inc a	
	call 008ceh
	ret c	
	ret z	
	ld a,(00b8ch)
	ret	
	ld (00b8bh),de
	ld c,001h
	ld a,014h
	rst 28h	
	ret c	
	ret z	
	ld bc,(00b86h)
	ld a,020h
	rst 28h	
	ret c	
	ld c,001h
	ld a,024h
	rst 28h	
	call 00a4ah
	jr nc,l50d5h
	ld a,01ah
	rst 28h	
	ret c	
	ret z	
l50d5h:
	ld a,028h
	rst 28h	
	ld c,001h
	ld a,016h
	rst 28h	
	ret	
l50deh:
	ld a,01eh
	rst 28h	
	ld c,(hl)	
	inc hl	
	call nc,0079eh
	ret c	
	ld c,(hl)	
l50e8h:
	ld a,01eh
	rst 28h	
	ret	
l50ech:
	ld hl,(00b8bh)
	ld b,004h
	ld c,(hl)	
	inc hl	
	ld a,c	
	cp 0c9h
	jr z,l50e8h
	cp 0c3h
	jr z,l50deh
	ld a,01eh
	rst 28h	
	ret c	
l5100h:
	ld c,(hl)	
	inc hl	
	ld a,01eh
	rst 28h	
	ret c	
	djnz l5100h
	ld d,h	
	ld e,l	
	dec hl	
	ld a,(hl)	
	dec hl	
	ld l,(hl)	
	ld h,a	
	ex de,hl	
	ld c,(hl)	
	inc hl	
	ld a,(hl)	
	inc hl	
	ld (00b8bh),hl
	ld h,a	
	ld l,c	
l5119h:
	ld a,d	
	or e	
	jr z,l50ech
	ld c,(hl)	
	inc hl	
	ld a,01eh
	rst 28h	
	ret c	
	dec de	
	jr l5119h
	xor a	
	ld (00b87h),a
	ld bc,(00b86h)
	ld a,020h
	rst 28h	
	ld c,00fh
	ld a,024h
	rst 28h	
	ld a,(00b87h)
	or a	
	scf	
	ret nz	
	call 008bfh
	ret c	
	ld a,028h
	rst 28h	
	ld a,(00b87h)
	push af	
	xor a	
	ld (00b87h),a
	ld bc,(00b86h)
	ld a,022h
	rst 28h	
	jr c,l518ah
	ld c,00fh
	ld a,026h
	rst 28h	
	ld hl,00b96h
l515ch:
	ld a,01ch
	rst 28h	
	jr c,l518ah
	ld (hl),a	
	inc hl	
	ld a,(00b87h)
	bit 6,a
	jr z,l515ch
	dec hl	
	ld (hl),000h
	ld a,02ah
	rst 28h	
	pop af	
	ld (00b87h),a
	ld hl,00b96h
	ld a,(hl)	
	sub 030h
	add a,a	
	ld b,a	
	add a,a	
	add a,a	
	add a,b	
	inc hl	
	add a,(hl)	
	sub 030h
	dec hl	
	cp 002h
	ccf	
	ret nc	
	cp a	
	ret	
l518ah:
	pop bc	
	ret	
	inc c	
	dec c	
	ret z	
	ld a,c	
	cp 041h
	jr c,l51a4h
	cp 05bh
	jr c,l51a0h
	cp 061h
	jr c,l51a4h
	cp 07bh
	jr nc,l51a4h
l51a0h:
	xor 020h
	ld c,a	
	ret	
l51a4h:
	push hl	
	ld hl,00b15h
l51a8h:
	ld a,(hl)	
	inc hl	
	inc hl	
	or a	
	jr z,l51b3h
	cp c	
	jr nz,l51a8h
	dec hl	
	ld c,(hl)	
l51b3h:
	pop hl	
	or 001h
	ret	
	ld e,049h
	rra	
	ld e,(hl)	
	ld b,b	
	ld h,l	
	ld e,e	
	ld c,a	
	ld e,h	
	ld l,a	
	ld e,l	
	ld h,c	
	ld e,(hl)	
	ld (hl),l	
	ld e,a	
	ld d,l	
	ld h,b	
	ld b,l	
	ld a,e	
	ld c,a	
l51cbh:
	ld a,h	
	ld c,a	
	ld a,l	
	ld b,c	
	ld a,(hl)	
	ld d,l	
	ld a,a	
	ld d,l	
	nop	
	push hl	
	push bc	
	ld hl,00b42h
	ld b,000h
	ld c,a	
	add hl,bc	
	ld c,(hl)	
	inc hl	
	ld h,(hl)	
	ld l,c	
	pop bc	
	ex (sp),hl	
	ret	
	ld d,d	
	ld b,058h
	ld b,060h
	ld b,068h
	ld b,06dh
	ld b,071h
	ld b,076h
	ld b,011h
	ld a,(bc)	
	call 00308h
	ld a,(bc)	
	sub l	
	ex af,af'	
	ld (hl),a	
	ex af,af'	
	add a,h	
	ld a,(bc)	
	and c	
	ld a,(bc)	
	defb 0fdh,007h,09eh	;illegal sequence
	rlca	
	inc h	
	rlca	
	jr nz,l520fh
	sub d	
	rlca	
	ld h,c	
	rlca	
	rrca	
	rlca	
	inc bc	
l520fh:
	rlca	
	jp pe,02c0ah
	add hl,sp	
l5214h:
	add hl,sp	
	inc l	
	ld b,l	
	ld c,(hl)	
	ld b,h	
	jr nz,l526ah
	ld b,(hl)	
	jr nz,l5264h
l521eh:
	ld c,c	
	ld c,h	
	ld b,l	
	inc l	
	jr nc,l5254h
	inc l	
	jr nc,l5257h
	nop	
	nop	
	nop	
	nop	
	nop	
	nop	
	nop	
	nop	
	nop	
	nop	
	nop	
	nop	
	nop	
	nop	
	nop	
	nop	
l5237h:
	nop	
	nop	
	nop	
	nop	
	nop	
	nop	
	nop	
	nop	
	nop	
	nop	
	nop	
	nop	
	nop	
	nop	
	nop	
	nop	
	nop	
	nop	
	nop	
	nop	
	nop	
l524ch:
	nop	
	nop	
	nop	
	nop	
	nop	
	nop	
	nop	
	nop	
l5254h:
	nop	
	nop	
	nop	
l5257h:
	nop	
	nop	
	nop	
	nop	
	nop	
	nop	
	nop	
	nop	
	nop	
	nop	
	nop	
	nop	
	nop	
l5264h:
	nop	
	nop	
	nop	
	nop	
	nop	
	nop	
l526ah:
	nop	
	nop	
	nop	
	nop	
	nop	
	nop	
	nop	
l5271h:
	nop	
	nop	
	nop	
	nop	
	nop	
	nop	
	nop	
	jr nz,l5282h
	ld (de),a	
	inc b	
	nop	
	ex af,af'	
	add a,d	
	jr nz,l52a1h
	ld (de),a	
l5282h:
	nop	
	nop	
	nop	
	nop	
	nop	
	nop	
	nop	
	nop	
	djnz l521eh
	ld b,b	
	nop	
	jr nz,l5214h
	djnz l52d4h
	ex af,af'	
	ld hl,01084h
	ld b,d	
	ex af,af'	
	ld hl,01084h
	nop	
	nop	
	add a,c	
	inc b	
	ld b,b	
	nop	
l52a1h:
	nop	
	nop	
	nop	
	ld b,b	
	jr nz,l5237h
	inc h	
	ld b,b	
	ld b,b	
	nop	
	ld b,b	
	ex af,af'	
	djnz l52afh
l52afh:
	nop	
	nop	
	ex af,af'	
l52b2h:
	nop	
	nop	
	nop	
	nop	
	jr nz,l52b9h
	add a,c	
l52b9h:
	add a,b	
l52bah:
	nop	
	nop	
	add a,h	
	nop	
	jr nz,$+10
	nop	
	adc a,b	
	djnz l52e4h
	add a,b	
	inc b	
	ld b,b	
	jr nz,$+10
	nop	
	nop	
	nop	
	nop	
	add a,b	
	ex af,af'	
	nop	
	ld de,00104h
	inc b	
l52d4h:
	ex af,af'	
	add a,d	
	add a,b	
	ex af,af'	
	ld b,c	
	ld (02020h),hl
	add a,b	
	ld b,h	
	nop	
	nop	
	jr nz,l52e2h
l52e2h:
	nop	
	ld (bc),a	
l52e4h:
	nop	
	nop	
	inc b	
	ld (bc),a	
	sub b	
	nop	
	inc h	
	djnz l5271h
	inc b	
	nop	
l52efh:
	djnz l5301h
	ld b,d	
	nop	
	nop	
	inc b	
	ld b,d	
	jr nz,l52f8h
l52f8h:
	nop	
	sub b	
	nop	
	nop	
	sub d	
	add a,b	
	nop	
	nop	
	add a,b	
l5301h:
	nop	
l5302h:
	ld (bc),a	
l5303h:
	nop	
	nop	
	ld (bc),a	
	ld b,c	
	jr nz,l530bh
	add hl,bc	
	ld b,h	
l530bh:
	nop	
	nop	
	nop	
	nop	
	inc b	
	ld b,b	
	nop	
	nop	
	nop	
	nop	
	nop	
	add a,b	
	djnz l5359h
	ld (bc),a	
	ld b,b	
	add a,d	
	nop	
	nop	
	nop	
	inc b	
	inc b	
	ld hl,01020h
	djnz l5366h
	nop	
	ld (bc),a	
	inc b	
	nop	
	ld (bc),a	
	nop	
	djnz l52b2h
	nop	
	nop	
	djnz l52bah
	nop	
	add a,b	
	nop	
	ld b,d	
	nop	
	nop	
	inc b	
	ld bc,00410h
	ld hl,00004h
	jr nz,l5342h
	nop	
l5342h:
	nop	
	inc b	
	nop	
	nop	
	nop	
	nop	
	nop	
	nop	
	nop	
	nop	
	nop	
	nop	
	nop	
	nop	
	nop	
	nop	
	nop	
	nop	
	nop	
	ld hl,02111h
	ld b,d	
l5359h:
	inc b	
	nop	
	nop	
	nop	
	nop	
	nop	
	ld b,b	
	nop	
	nop	
	nop	
	nop	
	nop	
	ld (bc),a	
l5366h:
	ld de,00408h
	ld (bc),a	
	add a,c	
	inc h	
	jr nz,l52efh
	nop	
	add a,b	
	djnz l5303h
	inc h	
	ld c,c	
	nop	
	jr nz,l5377h
l5377h:
	adc a,c	
	inc h	
	djnz l5383h
	nop	
	inc b	
	sub d	
	add a,h	
	ld b,b	
	add a,h	
	inc h	
	adc a,c	
l5383h:
	inc h	
	ld de,09212h
	inc h	
	djnz l53aah
	ld b,d	
	ld (de),a	
	nop	
	nop	
	ld c,c	
	ld b,d	
	nop	
	jr nz,l5393h
l5393h:
	nop	
	sub b	
	sub b	
	nop	
	djnz $+20
	nop	
	ex af,af'	
	nop	
	ld bc,00880h
	jr nz,l53a9h
	inc b	
	ld bc,04400h
	inc b	
	jr nz,l53a8h
l53a8h:
	ld (bc),a	
l53a9h:
	nop	
l53aah:
	jr nz,l53cch
	inc h	
	ld (bc),a	
	ex af,af'	
	ld bc,00080h
	ld bc,00080h
	nop	
	add a,b	
	adc a,b	
	djnz l53beh
	nop	
	inc h	
	ld b,b	
	nop	
l53beh:
	djnz l53c0h
l53c0h:
	djnz l53e2h
	inc b	
	inc b	
	add a,b	
	jr nz,l53c7h
l53c7h:
	djnz $+3
	ld bc,01100h
l53cch:
	ld b,d	
	nop	
	ld (bc),a	
	jr nz,l53d1h
l53d1h:
	nop	
	jr nz,$+66
	nop	
	nop	
	nop	
	nop	
	ld b,b	
	nop	
	nop	
	ld (04100h),hl
	jr nz,$+70
	nop	
	ex af,af'	
l53e2h:
	jr nz,l53e4h
l53e4h:
	ld c,b	
	nop	
	nop	
	nop	
	nop	
	nop	
	add a,b	
	nop	
	nop	
	nop	
	nop	
	nop	
	nop	
	ld bc,0aaa0h
	xor d	
	xor d	
	xor d	
	xor d	
	ld (bc),a	
	nop	
	nop	
	nop	
	nop	
	nop	
	nop	
	nop	
