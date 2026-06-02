org 0h

RS equ P1.3
EN equ P1.2
DELAY_VAL equ 08ACFh

main:
	clr RS 	; RS = 0 (instr)
	call funcS 	; Modo de envio 4 em 4 bits
	call dispC	 ; Liga o display
	call entryM 	; Shift Right

next:
	call scanKey	 ; Varre a tecla
	setb RS	 ; RS = 1 (data)
	clr A
	mov A, r7 	; Passa os dados para o ACC
	call sendCarac	 ; Envia o caracter para o LCD
	cjne A, #'#', next

end:  
	sjmp end

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

scanKey:
	; Linha 0
	clr P0.3
	call lcode0
	setb P0.3
	jb f0, done

	; Linha 1
	clr P0.2
	call lcode1
	setb P0.2
	jb f0, done

	; Linha 2
	clr P0.1
	call lcode2
	setb P0.1
	jb f0, done

	; Linha 3
	clr P0.0
	call lcode3
	setb P0.0
	jb f0, done

	jmp scanKey

done:
	clr f0
	ret

lcode0:
	jnb P0.4, keycode02
	jnb P0.5, keycode01
	jnb P0.6, keycode00
	ret

lcode1:
	jnb P0.4, keycode12
	jnb P0.5, keycode11
	jnb P0.6, keycode10
	ret

lcode2:
	jnb P0.4, keycode22
	jnb P0.5, keycode21
	jnb P0.6, keycode20
	ret

lcode3:
	jnb P0.4, keycode32
	jnb P0.5, keycode31
	jnb P0.6, keycode30
	ret

keycode00:
	setb f0
	call ESPSOL
	mov r7, #'1'
	ret

keycode01:
	setb f0
	call ESPSOL
	mov r7, #'2'
	ret

keycode02:
	setb f0
	call ESPSOL
	mov r7, #'3'
	ret

keycode10:
	setb f0
	call ESPSOL
	mov r7, #'4'
	ret

keycode11:
	setb f0
	call ESPSOL
	mov r7, #'5'
	ret

keycode12:
	setb f0
	call ESPSOL
	mov r7, #'6'
	ret

keycode20:
	setb f0
	call ESPSOL
	mov r7, #'7'
	ret

keycode21:
	setb f0
	call ESPSOL
	mov r7, #'8'
	ret

keycode22:
	setb f0
	call ESPSOL
	mov r7, #'9'
	ret

keycode30:
	setb f0
	call ESPSOL
	mov r7, #'*'
	ret

keycode31:
	setb f0
	call ESPSOL
	mov r7, #'0'
	ret

keycode32:
	setb f0
	call ESPSOL
	mov r7, #'#'
	ret

; Sub-rotina para eliminar o bounce do aperto da tecla
ESPSOL:
	mov A, P0
	anl A, #070h
	cjne A, #070h, ESPSOL

	mov TMOD, #01h
	mov TH0, #high(DELAY_VAL)
	mov TL0, #low(DELAY_VAL)
	setb tr0
	jnb tf0, $
	clr tr0
	clr tf0
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
