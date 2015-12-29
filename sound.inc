;---------------------------------------------------
; SOUND A,X,Y, produce tone
;
; A = Duration (1-255) * 0,005 usec
; X = Pitch (9-19)
; Y = Octave (0-7)
;---------------------------------------------------

dur_val   equ $99
pit_val   equ $9a
oct_val   equ $9b

sound:
	sta dur_val
	stx pit_val
	sty oct_val

	lda #>5000     ; 2
	sta $b809      ; 4
	lda #<5000     ; 2
	sta $b808      ; 4
sloop3:                       ;  
	  jsr toggle         ;22 , t3=t2+25

	  ldy oct_val        ; 3 , t2=Y*(t1+5)+3
sloop2:	   ldx pit_val       ; 3 , t1=X*34+3
sloop1:	      lda $b809      ; 4
	      cmp #$ff       ; 2
	      beq chk_timer  ; 2
	      jsr wait       ; 6+15
sloop5:	      dex            ; 2
	    bne sloop1        ; 3
	    dey              ; 2
	  bne sloop2          ; 3
	jmp sloop3            ; 3

end_sound:
	rts             

;--------------------------------------
; Toggle speaker
;--------------------------------------
toggle:
	lda $b002      ; 4
	eor #4         ; 2
	sta $b002      ; 4
	rts            ; 6

;--------------------------------------
; Check if time is over (21 usec)
;--------------------------------------
chk_timer:
	dec dur_val    ; 3
	beq end_sound  ; 3
settimer:
	lda #>5000     ; 2
	sta $b809      ; 4
	lda #<5000     ; 2
	sta $b808      ; 4
	jmp sloop5      ; 3

;--------------------------------------
; Wait routine 21-6=15 usec
;--------------------------------------
wait:
	eor $0         ; 3
	nop            ; 2
	nop            ; 2
	nop            ; 2
	rts            ; 6
sounds:
eat_sound:		; 2,-15,0,1
	lda #10
	ldx #14
	ldy #8
	jmp sound
eat_flash_sound1:	; 2,-15,120,1
	lda #10
	ldx #10
	ldy #2
	jmp sound
eat_flash_sound2:	; 2,-15,150,1
	lda #10
	ldx #6
	ldy #2
	jmp sound
die_sound:		; 0,-15,6,10
	lda #100
	ldx #27
	ldy #4
	jmp sound
end_level_sound:	; &12,-15,140,1
	lda #10
	ldx #7
	ldy #2
	jmp sound

