;--------------------------------------------------------------------------
; HYPER VIPER
;
; Written by Kenton Price in 1986
; Enhancements for release by Paul Davis
; Joystick support by Pitfall Jones
; Atom version by Kees van Oss
;
; Version 1.1
; Released by Retro Software, January 2011
; http://www.retrosoftware.co.uk/hyperviper
;--------------------------------------------------------------------------
; This work is licensed by Kenton Price (retrosoftware@kentonprice.com)
; under the Creative Commons Attribution-NonCommercial-ShareAlike
; 3.0 Unported License.
;
; To view a copy of this licence, visit
; http://creativecommons.org/licenses/by-nc-sa/3.0/
; or send a letter to Creative Commons, 171 Second Street,
; Suite 300, San Francisco, California 94105, USA.
;--------------------------------------------------------------------------

tmp      equ $70
text_ptr equ $60

; os call entry points
osrdch                       equ $fe71
osrdkey                      equ $fe94
osrnd                        equ $c986
oswaitsync                   equ $fe66
;oswrch = &FFEE
;osword = &FFF1
;osbyte = &FFF4
;oscli = &FFF7

; sprite/score data
snake_segment_pointers       equ sprites
rock_sprite_data             equ sprites+$70
score                        equ sprites+$80
mushroom_sprite_data         equ sprites+$90
brick_wall_sprite_data       equ sprites+$A0
snake_body_sprite_data       equ sprites+$B0
snake_head_sprite_data       equ sprites+$c0
snake_head_right_sprite_data equ sprites+$D0

; game constants
max_lives_count             equ 13
max_screen_number           equ 5
initial_snake_segment_count equ 5
initial_lives_count         equ 5
initial_flash_duration      equ 30
;initial_sound_flag          equ 0
game_speed                  equ 5
mushroom_count              equ 20
safe_zone_size              equ 3
wait_time                   equ 50

; game variables
snake_head_ptr              equ $c0
ptr                         equ $c2
safe_zone_left_ptr          equ $c4
safe_zone_right_ptr         equ $c6
high_score_entry_ptr        equ $c8
high_score_copy_ptr         equ $ca

snake_segment_count         equ $300
snake_direction             equ $301
pause_flag                  equ $302
collision_object            equ $304
last_screen_number          equ $309
timer0                      equ $30a
timer1                      equ $30b
timer2                      equ $30c
;sound_flag                  equ $30D
;tune_ptr                    equ $30E
flash_counter               equ $312
flash_duration              equ $314
high_score_crsr_pos         equ $316
high_score_entry_pos        equ $317
last_rock_ptr               equ $318
counter                     equ $31D
current_screen_number       equ $31E
lives_counter               equ $31F

x_tmp    equ $240
y_tmp    equ $241
store0   equ $242
store1   equ $243
store2   equ $244
store3   equ $245

; screen constants
bytes_per_character          equ 8
character_per_line           equ 32
;bytes_per_line               equ bytes_per_character*character_per_line

lives_screen_location        equ $8000+25+0*$100
flashing_mushroom_location   equ $820F ;??????????????
snake_start_location         equ $8000+12+14*$100

; sprite image constants
mushroom_image               equ $ff
rock_image                   equ $78
wall_image                   equ $f7

; movement direction constants
dir_left                     equ 1
dir_right                    equ 2
dir_up                       equ 3
dir_down                     equ 4


;-------------------------------------------------------------
;MAIN
;-------------------------------------------------------------

dwnld:
	; entry point to the game
	; initialise screen display

	LDA #1
	STA last_screen_number
;	JMP title_screen

;-------------------------------------------------------------
title_screen:
	; swap between title screen and high score screen
	; until space bar is pressed
	LDA #0
show_screen:
	STA timer0
	STA timer1
	LDA screen_duration
	STA timer2
tsloop:
	LDA timer0
	AND #1
	BNE high
	JSR display_title_screen
	JMP waitkey
high:
	JSR display_high_scores
waitkey:
	JSR display_press_space
	LDA #1
	STA current_screen_number
walp:
;joy:
;	LDA #1
;	STA use_joystick
;	BEQ nofire
;	JSR check_fire
;	BCS play
;	LDA #0
;	STA use_joystick
	; check for space bar
nofire:
	jsr osrdch
	cpy #0
	BEQ play

	DEC timer1
	BNE walp
	DEC timer2
	BNE walp
	LDA screen_duration
	STA timer2
        INC timer0
	JMP tsloop
;-------------------------------------------------------------
play:
	; check which screen the player got up to last time
	; if >1 offer the choice of screen to play
	LDA last_screen_number
	CMP #1
	BEQ le1
	LDY #0
levv:
	jsr display_which_screen
	CLC
	LDA last_screen_number
	ADC #'0'
        ldx #24
        ldy #18
        jsr PUTCHAR

	CLC
	LDA last_screen_number
	ADC #'1'
	STA last_screen_number
log:
	JSR osrdch
	tya
        clc
        adc #$20
        CMP #'1'
	BMI log
	CMP last_screen_number
	BPL log
	SEC
	SBC #'0'
	STA current_screen_number
	SEC
	LDA last_screen_number
	SBC #'1'
	STA last_screen_number
	; display status panel
le1:
	jsr display_status_panel
	; reset game variables
	LDA #0
	STA score
	STA score+1
	STA score+2
	STA score+3
	LDA #initial_flash_duration
	STA flash_duration
	LDA #initial_lives_count
	STA lives_counter
prsn:
;	JMP go

;-------------------------------------------------------------
go:
	LDA #0
	STA pause_flag

    ; Draw snake head in playfield  
	LDA #<snake_head_sprite_data    
	STA ptr
	LDA #>snake_head_sprite_data
	STA ptr+1
	LDA #<snake_start_location
	STA snake_head_ptr
	LDA #>snake_start_location
	STA snake_head_ptr+1
	JSR draw_snake_head
	LDA #initial_snake_segment_count
	STA snake_segment_count
	LDA #dir_left
	STA snake_direction
    ; Draw snake body segments in playfield
	LDX #0
wcr:
	CLC
        txa
        lsr a
	ADC #<(snake_start_location+1)
	STA snake_segment_pointers+2,X
	STA ptr
	LDA #>snake_start_location
	STA snake_segment_pointers+3,X
	STA ptr+1
	JSR draw_snake_segment
        INX
	INX
	CPX #8
	BNE wcr

    ; Draw brick wall around edge of screen
	LDA #<($8000+0+3*$100)
	STA ptr
	LDA #>($8000+0+3*$100)
	STA ptr+1
	LDA #32
	STA counter
	JSR draw_horizontal_wall    ;Top
	lda #0
        sta ptr
	LDA #20
	STA counter
	JSR draw_vertical_wall      ;Left
	LDA #32
	STA counter
	JSR draw_horizontal_wall    ;Right
	LDA #<($8000+31+3*$100)
	STA ptr
	LDA #>($8000+31+3*$100)
	STA ptr+1
	LDA #21
	STA counter
	JSR draw_vertical_wall      ;Bottom

    ; Check screen number to start
	LDA current_screen_number
	CMP #1
	BNE kk
	JMP mushy
kk:
	CMP #2
	BEQ scr2
	CMP #3
	BEQ scr3
	CMP #4
	BEQ scr4
	CMP #5
	BPL scr5
scr2:
	LDA #<($8000+16+3*$100)     ;Draw screen 2
	STA ptr
	LDA #>($8000+16+3*$100)
	STA ptr+1
	LDA #7
	STA counter
	JSR draw_vertical_wall
	LDA #>($8000+16+16*$100)
	STA ptr+1
	LDA #7
	STA counter
	JSR draw_vertical_wall
	JMP mushy
scr3:
	LDA #<($8000+10+3*$100)     ;Draw screen 3
	STA ptr
	LDA #>($8000+10+3*$100)
	STA ptr+1
	LDA #7
	STA counter
	JSR draw_vertical_wall
	LDA #>($8000+10+16*$100)
	STA ptr+1
	JSR draw_vertical_wall
	LDA #<($8000+21+3*$100)
	STA ptr
	LDA #>($8000+21+3*$100)
	STA ptr+1
	JSR draw_vertical_wall
	LDA #>($8000+21+16*$100)
	STA ptr+1
	JSR draw_vertical_wall
	JMP mushy
scr4:
	LDA #<($8000+1+13*$100)     ;Draw screen 4
	STA ptr
	LDA #>($8000+1+13*$100)
	STA ptr+1
	LDA #10
	STA counter
	JSR draw_horizontal_wall
	LDA #<($8000+21+13*$100)
	STA ptr
	JSR draw_horizontal_wall
	LDA #<($8000+16+3*$100)
	STA ptr
	LDA #>($8000+16+3*$100)
	STA ptr+1
	LDA #7
	STA counter
	JSR draw_vertical_wall
	LDA #>($8000+16+15*$100)
	STA ptr+1
        inc counter
	JSR draw_vertical_wall
	JMP mushy
scr5:
	LDA #<($8000+7+3*$100)     ;Draw screen 5
	STA ptr
	LDA #>($8000+7+3*$100)
	STA ptr+1
	LDA #17
	STA counter
	JSR draw_vertical_wall
	LDA #<($8000+24+7*$100)
	STA ptr
	LDA #>($8000+24+7*$100)
	STA ptr+1
	JSR draw_vertical_wall
	LDA #<($8000+12+7*$100)
	STA ptr
	LDA #>($8000+12+7*$100)
	STA ptr+1
	LDA #12
	STA counter
	JSR draw_horizontal_wall
	LDA #<($8000+8+11*$100)
	STA ptr
	LDA #>($8000+8+11*$100)
	STA ptr+1
	JSR draw_horizontal_wall
	LDA #<($8000+12+15*$100)
	STA ptr
	LDA #>($8000+12+15*$100)
	STA ptr+1
	JSR draw_horizontal_wall
	LDA #<($8000+8+19*$100)
	STA ptr
	LDA #>($8000+8+19*$100)
	STA ptr+1
	JSR draw_horizontal_wall

;-------------------------------------------------------------
mushy
    ; Draw snake with number of lives left
	LDX lives_counter
mushy2:
        stx ptr
        sec
        LDA #<lives_screen_location
	sbc ptr
        STA ptr
	LDA #>lives_screen_location
	STA ptr+1
	JSR draw_snake_segment
	DEX
	BNE mushy2
	LDA #<snake_head_right_sprite_data
	STA ptr
	LDA #>snake_head_right_sprite_data
	STA ptr+1
	LDA #<lives_screen_location
	STA snake_head_ptr
	LDA #>lives_screen_location
	STA snake_head_ptr+1
	JSR draw_snake_head

    ; Set snake head position
	LDA #<snake_start_location
	STA snake_head_ptr
	STA snake_segment_pointers
	LDA #>snake_start_location
	STA snake_head_ptr+1
	STA snake_segment_pointers+1

    ; Place mushrooms at random locations on screen
	LDX #mushroom_count
mush:
	JSR get_random_position
	; Don't place mushroom if something already there
	LDY #$40
	LDA (ptr),Y
	BNE mush
	; Prevent mushrooms from being placed next to each other
	; mushroom in valid position
	JSR display_mushroom
	DEX
	BNE mush

	LDA flash_duration
	STA flash_counter

    ; Place rocks at random locations on screen
	CLC
	LDA #4
	ADC current_screen_number
	ADC current_screen_number
	TAX
rloop:
	JSR place_rock
	DEX
	BNE rloop

    ; Save last rock position
	LDA ptr
	STA last_rock_ptr
	LDA ptr+1
	STA last_rock_ptr+1

    ; 1 sec delay
	LDA #wait_time
	STA ptr
wawaw:
	JSR oswaitsync
	DEC ptr
	BNE wawaw

;        jsr $ffe3

;-------------------------------------------------------------
; Game loop if not dead 
;-------------------------------------------------------------
eachgo:        
	lda $b002
	eor #4
	sta $b002

    ; Turn the last placed rock into a flashing mushroom
	LDA last_rock_ptr
	STA ptr
	LDA last_rock_ptr+1
	STA ptr+1
;	JSR oswaitsync
	JSR display_mushroom
	JSR oswaitsync
	JSR delete_snake_segment

    ; Start keyscan
	JSR osrdch
check_left:
	cpy #$3a        ; Z-key
	BNE check_right
	LDA snake_direction ; ignore attempt to move the snake back on itself
	CMP #dir_right
	BEQ check_right
	LDA #dir_left
	STA snake_direction
	BNE move
check_right:
	cpy #$38        ; X-key
	BNE check_up
	LDA snake_direction ; ignore attempt to move the snake back on itself
	CMP #dir_left
	BEQ check_up
	LDA #dir_right
	STA snake_direction
	BNE move
check_up:
	cpy #$1b        ; ;-key
	BNE check_down
	LDA snake_direction ; ignore attempt to move the snake back on itself
	CMP #dir_down
	BEQ check_down
	LDA #dir_up
	STA snake_direction
	BNE move
check_down:
	cpy #$1e        ; .-key
	BNE move
	LDA snake_direction ; ignore attempt to move the snake back on itself
	CMP #dir_up
	BEQ move
	LDA #dir_down
	STA snake_direction

    ; Move snake in correct direction
move:
	JSR move_snake_forwards
	LDA snake_direction
	CMP #dir_left
	BEQ left
	CMP #dir_right
	BEQ right
	CMP #dir_up
	BEQ up
	JMP down
left:
	dec snake_head_ptr
	JMP goon
right:
	inc snake_head_ptr
	JMP goon
up:
	dec snake_head_ptr+1
	JMP goon
down:
	inc snake_head_ptr+1
goon:
	LDA snake_head_ptr
	STA snake_segment_pointers
	LDA snake_head_ptr+1
	STA snake_segment_pointers+1

	; check if snake has collided with anything
	LDY #$40
	LDA (snake_head_ptr),Y
	STA collision_object

	; draw snake head in current direction
	; calculate sprite adres snake head left/right/up/down
	LDA snake_direction
	CLC
	ASL A
	ASL A
	ASL A
	ASL A
	ADC #<snake_body_sprite_data
	STA ptr
	LDA #>snake_body_sprite_data
	STA ptr+1
;	LDY #0
	JSR draw_snake_head

	; draw snake segment over the head in previous location
	LDA snake_segment_pointers+2
	STA ptr
	LDA snake_segment_pointers+3
	STA ptr+1
	JSR draw_snake_segment

	; delete the last snake segment at the tail
	CLC
	LDA snake_segment_count
	ASL A
	TAY
	LDA snake_segment_pointers,Y
	STA ptr
	LDA snake_segment_pointers+1,Y
	STA ptr+1
	JSR delete_snake_segment

    ; Check collision status
	LDA collision_object
	BEQ ok1
	CMP #mushroom_image ; mushroom - eat
	BEQ eat_mushroom
	LDA collision_object
	CMP #rock_image ; rock - check if flashing first
	BNE deda ; anything else - die
	LDA last_rock_ptr+1
	CMP #>flashing_mushroom_location
	BEQ eat_mushroom ; flashing mushroom - eat
deda:
	JMP dead
;-------------------------------------------------------------
eat_mushroom:
	jsr eat_sound

	JSR place_rock
	; set this new rock to be the potential flashing mushroom
	; unless there is still one on the screen
	LDA last_rock_ptr+1
	CMP #>flashing_mushroom_location
	BNE keke
	LDA ptr
	STA last_rock_ptr
	LDA ptr+1
	STA last_rock_ptr+1
	LDA flash_duration
	STA flash_counter
keke:
	LDA collision_object
	CMP #rock_image ; don't get points for eating a flashing mushroom
	BEQ ok1
	JSR increment_score
	INC snake_segment_count
ok1:
	; level ends after eating all the mushrooms
	LDA snake_segment_count
	CMP #mushroom_count+initial_snake_segment_count
	BNE gone
	JMP newlev
gone:
	LDA #game_speed
	BEQ fl
	STA counter
gloop:
	JSR oswaitsync
	DEC counter
	BNE gloop
	; check if snake has just eaten a flashing mushroom
fl:
	LDA last_rock_ptr
	CMP snake_head_ptr
	BNE nofl
	LDA last_rock_ptr+1
	CMP snake_head_ptr+1
	BNE nofl
	LDA #<flashing_mushroom_location
	STA last_rock_ptr
	LDA #>flashing_mushroom_location
	STA last_rock_ptr+1
	LDA flash_duration
	STA flash_counter

	JSR eat_flash_sound1
	JSR eat_flash_sound2
nofl:
pau:
	lda $b001
	and #$40
	BNE nopau
	LDA pause_flag
	BNE nopau
	LDA #1
	STA pause_flag
	LDY #0
	jsr display_pause_on_screen
pauwa:
	jsr osrdch
	cpy #$f   ; DELETE 
	BNE pauwa
	jsr display_pause_off_screen

nopau:	; check to see if flashing mushroom timer has expired
	DEC flash_counter
	LDA flash_counter
	BNE nopau2
	LDA flash_duration
	STA flash_counter
	LDA last_rock_ptr
	STA ptr
	LDA last_rock_ptr+1
	STA ptr+1
	CMP #>flashing_mushroom_location
	BEQ nprock
	JSR display_rock ; turn flasher back into a rock
nprock:
	JSR place_rock ; make a new flasher at a random position
	LDA ptr
	STA last_rock_ptr
	LDA ptr+1
	STA last_rock_ptr+1
nopau2:
	JMP eachgo

newlev:
	; 1 sec delay
	LDA #wait_time
	STA ptr
nl2:
	JSR oswaitsync
	DEC ptr
	BNE nl2
	; add 200 bonus points to score
	LDA #20
	STA ptr+1
nl3:
	LDA #10
	STA ptr
nl4:
	JSR oswaitsync
	DEC ptr
	LDA ptr
	BNE nl4

	JSR end_level_sound

	JSR increment_score
	DEC ptr+1
	LDA ptr+1
	BNE nl3
	JSR clear
	; move to next level + get an extra life
	INC current_screen_number
	LDA lives_counter
	CMP #max_lives_count
	BEQ gg
	INC lives_counter
gg:
	LDA current_screen_number
	CMP #max_screen_number+1
	BMI ggg
	LDA #max_screen_number
	STA current_screen_number
ggg:
	JMP go
dead:	; clear a life marker from the status panel
	LDA lives_counter
	STA ptr
	SEC
	LDA #<lives_screen_location
	SBC ptr
	STA ptr
	LDA #>lives_screen_location
	STA ptr+1
	JSR delete_snake_segment
	; play death sound effect

	JSR die_sound

	; 1 sec delay
	LDA #wait_time
	STA ptr
wawa:
	jsr oswaitsync
	DEC ptr
	LDA ptr
	BNE wawa
	; decrement lives, check if no lives left
	DEC lives_counter
	LDA lives_counter
	BEQ dd2
	JSR clear
	JMP go
dd2:
	; print Game Over message
	jsr display_game_over_screen

	LDA #wait_time*2
	STA ptr
	; update highest screen number reached
	LDA current_screen_number
	CMP last_screen_number
	BMI wawa3
	STA last_screen_number
	; 1 sec delay
wawa3:
	JSR oswaitsync
	DEC ptr
	LDA ptr
	BNE wawa3
	JSR clear
	; see if score is in high score table
	JSR check_high_score
	BCC restart ; if not, just show title screen
	JSR edit_high_score ; else enter name
	LDA #1 ; then show the high score screen first
	JMP show_screen
restart:
	JMP title_screen

;-------------------------------------------------------------
find_safe_zone:	; find the safe zone around the snake's head
	LDY #$40
	; find the left + right limits of the safe zone
	LDA snake_head_ptr
	STA safe_zone_left_ptr
	LDA snake_head_ptr+1
	STA safe_zone_left_ptr+1
	LDA snake_direction
	CMP #dir_right	; if moving right, ignore columns to left of head
	BEQ find_right_zone
	LDX #safe_zone_size
left_loop:
	dec safe_zone_left_ptr
	LDA (safe_zone_left_ptr),y
	CMP #wall_image
	BNE next_left
	inc safe_zone_left_ptr	; hit the wall
	BNE find_right_zone
next_left:
	DEX
	BNE left_loop
find_right_zone:
	LDA snake_head_ptr
	STA safe_zone_right_ptr
	LDA snake_head_ptr+1
	STA safe_zone_right_ptr+1
	LDA snake_direction
	CMP #dir_left	; if moving left, ignore columns to right of head
	BNE right1
	LDX #1
	BNE right_loop
right1:
	LDX #safe_zone_size+1	; +1 because the snake head occupies the first slot
right_loop
	inc safe_zone_right_ptr
	LDA (safe_zone_right_ptr),y
	CMP #wall_image
	BEQ find_top_zone
	DEX
	BNE right_loop
find_top_zone:
	; move the pointers to the top of the safe zone
	LDA snake_direction
	CMP #dir_down	; if moving down, ignore rows above head
	BEQ done
	sec
	lda safe_zone_left_ptr+1
	sbc #safe_zone_size
	sta safe_zone_left_ptr+1
	lda safe_zone_right_ptr+1
	sbc #safe_zone_size
	sta safe_zone_right_ptr+1
done:
	RTS

;-------------------------------------------------------------
place_rock:	; place a rock at a random screen location
	STX counter
	JSR find_safe_zone
again:	; make a pointer to a random position on the screen
	JSR get_random_position
	LDY #$40 ; don't place the rock if something is already there
	LDA (ptr),Y
	BNE again
	; don't place the rock in the safe zone around the snake's head
	LDA safe_zone_left_ptr
	STA store0
	LDA safe_zone_left_ptr+1
	STA store1
	LDA safe_zone_right_ptr
	STA store2
	LDA safe_zone_right_ptr+1
	STA store3
	LDA snake_direction
	CMP #dir_up	; if moving up/down, can ignore the other half of the rows
	BCC start
	LDX #safe_zone_size+1
	BNE loop5
start:
	LDX #safe_zone_size*2+1
loop5:	; if rock before left of zone, that's ok
	LDA ptr
	CMP store0
	LDA ptr+1
	SBC store1
	BCC ok
	; if rock inside safe zone, try again
	LDA ptr
	CMP store2
	LDA ptr+1
	SBC store3
	BCC again
	; otherwise, try the next line in the safe zone
	inc store1
	inc store3
	DEX
	BNE loop5
	; if rock not in safe zone, it's ok to place it
ok:
	LDX counter
display_rock:
	stx x_tmp
	sty y_tmp
	LDX #0
loop6:
	LDA rock_sprite_data,x
	ldy scr_ptr,x
	STA (ptr),Y
	INx
	CPx #bytes_per_character
	BNE loop6
	ldx x_tmp
	ldy y_tmp
	RTS
;-------------------------------------------------------------
move_snake_forwards:
	LDA snake_segment_count
	CLC
	ASL A
	TAY
loop8:
	LDA snake_segment_pointers-2,Y
	STA snake_segment_pointers,Y
	LDA snake_segment_pointers-1,Y
	STA snake_segment_pointers+1,Y
	DEY
	DEY
	BNE loop8
	RTS

;-------------------------------------------------------------
display_mushroom:
        stx x_tmp
        sty y_tmp
	LDX #0
loop4:
	ldy scr_ptr,x
	LDA mushroom_sprite_data,x
	STA (ptr),Y
	INX
	CPX #bytes_per_character
	BNE loop4
	ldx x_tmp
	ldy y_tmp
	RTS

;-------------------------------------------------------------
get_random_position:
        stx x_tmp
	sty y_tmp
        jsr osrnd
	lda $8
	AND #$1F
	STA ptr
	LDA $9
rndloop:
        jsr osrnd
	lda $9
	AND #$1f
	cmp #19
	bcs rndloop
        clc
	adc #$84
	STA ptr+1
        lda #0
	sta $4
	ldx x_tmp
	ldy y_tmp
        RTS

;-------------------------------------------------------------
seed_random:        
	jsr osrnd
	LDA $8
	STA random_bits
	LDA $9
	STA random_bits+1
	LDA $a
	STA random_bits+2
	LDA $b
	STA random_bits+3
	RTS

scr_ptr  .db 0,$20,$40,$60,$80,$a0,$c0,$e0

;-------------------------------------------------------------
;SUBROUTINES
;-------------------------------------------------------------

draw_snake_head:
        stx x_tmp
        sty y_tmp

	LDX #0
loop:
        txa
        tay
	LDA (ptr),y
        LDY scr_ptr,x
	STA (snake_head_ptr),Y
	INX
	CPX #bytes_per_character
	BNE loop

        ldx x_tmp
        ldy y_tmp
	RTS

;---------------------------------------------------------------
draw_snake_segment:
        stx x_tmp
        sty y_tmp

	LDX #0
loop2:
        LDY scr_ptr,x
	LDA snake_body_sprite_data,x
	STA (ptr),Y
	INX
	CPX #bytes_per_character
	BNE loop2

        ldx x_tmp
        ldy y_tmp
	RTS

;---------------------------------------------------------------
delete_snake_segment:
        stx x_tmp
        sty y_tmp

	LDX #0
loop7:
	LDA #0
	ldy scr_ptr,x
	STA (ptr),Y
	INX
	CPX #bytes_per_character
	BNE loop7

        ldx x_tmp
        ldy y_tmp
	RTS

;---------------------------------------------------------------
draw_horizontal_wall:
        lda counter
        pha
loop11:
	LDX #0
loop12:
	LDY scr_ptr,x
        LDA brick_wall_sprite_data,x
	STA (ptr),Y
	INX
	CPX #bytes_per_character
	BNE loop12
	inc ptr
	DEC counter
	BNE loop11
        pla
        sta counter
	RTS

;---------------------------------------------------------------
draw_vertical_wall:
        lda counter
        pha
loop21:
	LDX #0
loop22:
	LDY scr_ptr,x
	LDA brick_wall_sprite_data,x
	STA (ptr),Y
	INX
	CPX #bytes_per_character
	BNE loop22
	INC ptr+1
	DEC counter
	BNE loop21
        pla
        sta counter
	RTS

;---------------------------------------------------------------
increment_score:
	CLC
	LDA score+3
	ADC #1
	STA score+3
	CMP #10
	BMI display_score
	LDA #0
	STA score+3
	CLC
	LDA score+2
	ADC #1
	STA score+2
	CMP #10
	BMI display_score
	LDA #0
	STA score+2
	CLC
	LDA score+1
	ADC #1
	STA score+1
	CMP #10
	BMI display_score
	CLC
	LDA #0
	STA score+1
	LDA score
	ADC #1
	STA score
display_score:
        clc
        lda score
	adc #$30
	sta score_set+2
        lda score+1
	adc #$30
	sta score_set+3
        lda score+2
	adc #$30
	sta score_set+4
        lda score+3
	adc #$30
	sta score_set+5
	jsr display_score_screen
	RTS

;---------------------------------------------------------------
check_high_score:
; on exit: carry set if new high score, &75 = entry position (0-9), &77/&78 = ptr to entry
	LDA #<(hs1+2)
	STA high_score_entry_ptr
	LDA #>(hs1+2)
	STA high_score_entry_ptr+1

    ; Check score with scoretable entries
	LDX #10
loop38:
	LDY #0
loop39:
	LDA score,Y
	ORA #'0'
	CMP (high_score_entry_ptr),Y
	BCC next
	BNE found
	INY
	CPY #4
	BCC loop39
next:
	CLC
	LDA high_score_entry_ptr
	ADC #(hs2-hs1)
	STA high_score_entry_ptr
	BCC *+4
	INC high_score_entry_ptr+1
	DEX
	BNE loop38
	CLC
	BCC done1
found:
    ; Move remaining entries down
	DEX
	STX store0
	BEQ clr
	LDA #<hs9
	STA high_score_copy_ptr
	LDA #>hs9
	STA high_score_copy_ptr+1
	LDA #<hs10
	STA high_score_entry_ptr
	LDA #>hs10
	STA high_score_entry_ptr+1
cplp1:
	LDY #17
cplp2:
	LDA (high_score_copy_ptr),Y
	STA (high_score_entry_ptr),Y
	DEY
	cpy #2
	BNE cplp2
	LDA high_score_copy_ptr
	STA high_score_entry_ptr
	SEC
	SBC #(hs2-hs1)
	STA high_score_copy_ptr
	LDA high_score_copy_ptr+1
	STA high_score_entry_ptr+1
	SBC #0
	STA high_score_copy_ptr+1
	DEX
	BNE cplp1
clr:
    ; Make a new entry
	LDA #9
	SEC
	SBC store0
	STA high_score_entry_pos
	LDY #5
cloop:
	LDA score-2,Y
	ORA #'0'
	STA (high_score_entry_ptr),Y
	DEY
	cpy #2
	bne cloop
	LDA #' '
	LDY #10
cloop2:
	STA (high_score_entry_ptr),Y
	INY
	CPY #18
	BCC cloop2
	; carry already set by above
done1:
	RTS

;---------------------------------------------------------------
edit_high_score:
	JSR display_high_scores
	JSR display_congratulations

	LDA #0
	STA high_score_crsr_pos
	LDA high_score_entry_ptr
	CLC
	ADC #10
	STA high_score_entry_ptr
	BCC *+4
	INC high_score_entry_ptr+1

key:
	jsr osrdkey
	CMP #13  ; return key
	BEQ done3
	CMP #$7f  ; delete key
	BNE notback
	; delete last character
	LDY high_score_crsr_pos
	BEQ key
	DEC high_score_crsr_pos
	lda high_score_crsr_pos
	clc
	adc #12
	tax
	ldy #21
	lda #32
	jsr PUTCHAR
	lda #32
	ldy high_score_crsr_pos
	STA (high_score_entry_ptr),Y
	bne key
notback:
        ldx high_score_crsr_pos
        cpx #8
        beq key
	CMP #$41   ;A
	BCC key
	CPY #$5A   ;Z
	BCS key
	LDY high_score_crsr_pos
	CPY #10
	BCS key
	STA (high_score_entry_ptr),Y
	pha
	lda high_score_crsr_pos
	clc
	adc #12
	tax
	ldy #21
	pla
	jsr PUTCHAR
;	lda high_score_crsr_pos
;	cmp #7
;	beq key
	INC high_score_crsr_pos
	BNE key
done3:
	RTS

;---------------------------------------------------------------
; Display title screen
;---------------------------------------------------------------

display_title_screen:
        jsr clear4
 	ldx #<title_screen_text
        ldy #>title_screen_text
        jmp PUTSTRING

;---------------------------------------------------------------
; Display score
;---------------------------------------------------------------

display_score_screen:
 	ldx #<score_set
        ldy #>score_set
        jmp PUTSTRING

;---------------------------------------------------------------
; Display Pause on screen
;---------------------------------------------------------------

display_pause_on_screen:
 	ldx #<pause_on_text
        ldy #>pause_on_text
        jmp PUTSTRING

;---------------------------------------------------------------
; Display Pause off screen
;---------------------------------------------------------------

display_pause_off_screen:
 	ldx #<pause_off_text
        ldy #>pause_off_text
        jmp PUTSTRING

;---------------------------------------------------------------
; Display Game over
;---------------------------------------------------------------

display_game_over_screen:
 	ldx #<game_over_text
        ldy #>game_over_text
        jmp PUTSTRING

;---------------------------------------------------------------
; Display high scores
;---------------------------------------------------------------

display_high_scores:
        jsr clear4
 	ldx #<high_score_text
        ldy #>high_score_text
        jmp PUTSTRING

;---------------------------------------------------------------
; Display Press space
;---------------------------------------------------------------

display_press_space:
 	ldx #<press_space_text
        ldy #>press_space_text
        jmp PUTSTRING

;---------------------------------------------------------------
; Display Which screen
;---------------------------------------------------------------

display_which_screen:
 	ldx #<which_screen_text
        ldy #>which_screen_text
        jmp PUTSTRING

;---------------------------------------------------------------
; Display status panel
;---------------------------------------------------------------

display_status_panel:
 	jsr clear4
	ldx #<status_panel_text
        ldy #>status_panel_text
        jmp PUTSTRING

;---------------------------------------------------------------
; Display Congratulations text
;---------------------------------------------------------------

display_congratulations:
	ldx #<congratulations_text
        ldy #>congratulations_text
        jmp PUTSTRING


;---------------------------------------------------------------
; Clear screen 4
;---------------------------------------------------------------

scr_adr  equ tmp

clear4:  
        lda #$F0        ; Clear4
        sta $b000

        lda #$80
        sta scr_adr+1
        lda #0
        sta scr_adr
        tay
clrloop:
        sta (scr_adr),y  ; Clear byte
        iny
        bne clrloop
        inc scr_adr+1
        ldx scr_adr+1
        cpx #$98
        bne clrloop
        rts

;---------------------------------------------------------------
; Clear playfield
;---------------------------------------------------------------
clear:
        lda #$84
        sta scr_adr+1
        lda #0
        sta scr_adr
        tay
	jmp clrloop

;---------------------------------------------------------------
; Variables
;---------------------------------------------------------------

screen_duration .db 20   ; 10 sec
random_bits     .dw $ffff,$ffff

;---------------------------------------------------------------
; Data
;---------------------------------------------------------------

title_screen_text:
	.db  6, 1,'*** HYPER VIPER ***',0
        .db  2, 3,'BBC VERSION BY KENTON PRICE',0
        .db  2, 4,'ATOM VERSION BY KEES VAN OSS',0
        .db 14, 6,'----',0
        .db 14, 7,'KEYS',0
        .db 14, 8,'----',0
        .db 11,10,'Z ... LEFT',0
        .db 11,11,'X ... RIGHT',0
        .db 11,12,'; ... UP',0
        .db 11,13,'. ... DOWN',0
        .db 5,15,'CONTROL ... GAME PAUSE',0
        .db $ff

score_set:
	.db  6, 0,'00000',0
        .db $ff

press_space_text:
        .db 6,22,'PRESS SPACE TO PLAY',0
        .db $ff

high_score_text:
	.db  6, 1,'*** HYPER VIPERS ***',0
hs1	.db  8, 5,'00100   RETRO...',0
hs2	.db  8, 6,'00100   SOFTWARE',0
	.db  8, 7,'00100   RETRO...',0
	.db  8, 8,'00100   SOFTWARE',0
	.db  8, 9,'00100   RETRO...',0
	.db  8,10,'00100   SOFTWARE',0
	.db  8,11,'00100   RETRO...',0
	.db  8,12,'00100   SOFTWARE',0
hs9	.db  8,13,'00100   RETRO...',0
hs10	.db  8,14,'00100   SOFTWARE',0
        .db $ff

congratulations_text:
	.db  8, 16,'CONGRATULATIONS!',0
new_high_score_text:
	.db  9, 17,'NEW HIGH SCORE',0
enter_name_text:
        .db 5,19,'PLEASE ENTER YOUR NAME:',0
        .db $ff

status_panel_text:
	.db  0, 0,'SCORE 00000',0
	.db 10, 2,'HYPER VIPER',0
        .db 27, 0,'LIVES',0
        .db $ff

which_screen_text:
 	.db 4,17,'                       ',0
 	.db 5,18,'WHICH SCREEN (1 TO  )',0
 	.db 4,19,'                       ',0
 	.db $ff

pause_on_text
	.db 2, 1,'GAME PAUSE, DELETE CONTINUES',0
	.db $ff

pause_off_text
	.db 2, 1,'                            ',0
	.db $ff

game_over_text:
	.db 10,11,'           ',0
	.db 10,12,' GAME OVER ',0
	.db 10,13,'           ',0
	.db $ff

