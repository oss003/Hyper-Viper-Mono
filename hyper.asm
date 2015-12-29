codebase     equ $2a00
sprites      equ $3900

               org codebase-22

;****************************************
; Header for Atom emulator Wouter Ras
		 .db "MHYPER          "
		 .dw codebase
		 .dw codebase
		 .dw eind_asm-start_asm
;****************************************
start_asm    
               include hyper.inc
               include font.inc
               org $3900
               include sprites.inc
	       include sound.inc
eind_asm
