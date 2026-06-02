org 0h
    ljmp main

org 23h

recepcao:
    mov A, SBUF
    clr RI
	setb RS	 ; RS = 1 (data)
	call sendCarac	 ; Envia o caracter para o LCD
	reti

org 30h

RS equ P1.3
EN equ P1.2

main:
    ; LCD
	clr RS 	; RS = 0 (instr)
	call funcS 	; Modo de envio 4 em 4 bits
	call dispC	 ; Liga o display
	call entryM 	; Shift Right

    ; Timer
    mov TMOD, #20h  ; Auto-reload
    mov TH1, #0F9h  ; Recarga
    mov TL1, #0F9h  ; Contagem
	orl PCON, #80h  ; <-- O SEGREDO AQUI: Seta o bit SMOD = 1
    mov SCON, #50h  ; Serial em Modo 1 (8-bits), habilita recepção (REN=1)
    setb TR1        ; Liga o Timer 1

    ; Interrupção
    setb ES         
    setb EA         

    sjmp $

funcS:
	; DAT = 02h (Modo de 4 bit)
	clr P1.7
	clr P1.6
	setb P1.5
	clr P1.4
	
	call clock
	call delay
	call clock

	; DAT = 08h (LCD de 2 linhas)
	setb P1.7
	clr P1.6
	clr P1.5
	clr P1.4

	call clock
	call delay

	ret

dispC:
	; DAT = 0h
	clr P1.7
	clr P1.6
	clr P1.5
	clr P1.4
	
	call clock

	; DAT = 0Fh (Liga LCD e cursor)
	setb P1.7
	setb P1.6
	setb P1.5
	setb P1.4

	call clock
	call delay

	ret

entryM:
	; DAT = 0h
	clr P1.7
	clr P1.6
	clr P1.5
	clr P1.4
	
	call clock

	; DAT = 06h (Deslocamento à Direita)
	clr P1.7
	setb P1.6
	setb P1.5
	clr P1.4

	call clock
	call delay

	ret

sendCarac:
	; Envia nibble alto (bits 7–4)
	mov C, ACC.7
	mov P1.7, C
	mov C, ACC.6
	mov P1.6, C
	mov C, ACC.5
	mov P1.5, C
	mov C, ACC.4
	mov P1.4, C

	call clock	

	; Envia nibble baixo (bits 3–0)
	mov C, ACC.3
	mov P1.7, C
	mov C, ACC.2
	mov P1.6, C
	mov C, ACC.1
	mov P1.5, C
	mov C, ACC.0
	mov P1.4, C

	call clock
	call delay

	ret

clock:
	setb EN
	clr EN
	ret

delay:
	mov r0, #032h
	djnz r0, $
	ret
