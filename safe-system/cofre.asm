org 0h

RS equ P1.3
EN equ P1.2
DELAY_VAL equ 08ACFh
bottom_password equ 30h   ; endereço base da RAM para senha digitada
count equ 20h   ; índice atual na senha digitada
temp equ 35h		 ; endereço base na RAM para comparação

main:
	clr RS 	; RS = 0 (instr)
	mov count, #00h

	call funcS 	; modo de envio 4 em 4 bits
	call dispC	 ; liga o display
	call entryM 	; shift right

display_password:
	setb RS

	mov A, #'S'
	call sendCarac

	mov A, #'e'
	call sendCarac

	mov A, #'n'
	call sendCarac

	mov A, #'h'
	call sendCarac

	mov A, #'a'
	call sendCarac

	mov A, #03Ah	; :
	call sendCarac

next:
	call scanKey	 ; varre a tecla
	setb RS	 ; RS = 1 (data)
	clr A
	mov A, r7 	; passa os dados para o acumulador
	call sendCarac	 ; envia o caracter para o LCD

	mov A, count
	cjne A, #04h, next	; compara contador com 4

	call verify_password    ; depois de 4 digitações, compara

	call end

end:  
	sjmp end

verify_password:
	mov R0, #bottom_password		; RO = endereço base da senha digitada (RAM)
	mov DPTR, #rom_password	; DPTR = endereço base da senha correta (ROM)
	mov R2, #04h

verify_loop:
	; pega valor digitado
	mov A, @R0
	mov temp, A		

	; pega valor da ROM
	clr A
	movc A, @A+DPTR

	; compara
	cjne A, temp, wrong_password

	inc R0	; incrementa endereço base da RAM
	inc DPTR	; incrementa endereço base da ROM
	djnz R2, verify_loop		; decrementa e pula se não for zero

	sjmp correct_password
	sjmp end

correct_password:
	call second_line	

	setb RS         ; modo dados
	mov A, #'B'
	call sendCarac
	mov A, #'E'
	call sendCarac
	mov A, #'M'
	call sendCarac
	mov A, #' '
	call sendCarac
	mov A, #'V'
	call sendCarac
	mov A, #'I'
	call sendCarac
	mov A, #'N'
	call sendCarac
	mov A, #'D'
	call sendCarac
	mov A, #'O'
	call sendCarac
	ret

wrong_password:
	call second_line

	setb RS
	mov A, #'S'
	call sendCarac
	mov A, #'E'
	call sendCarac
	mov A, #'N'
	call sendCarac
	mov A, #'H'
	call sendCarac
	setb RS
	mov A, #'A'
	call sendCarac
	mov A, #' '
	call sendCarac
	mov A, #'E'
	call sendCarac
	mov A, #'R'
	call sendCarac
	mov A, #'R'
	call sendCarac
	mov A, #'A'
	call sendCarac
	mov A, #'D'
	call sendCarac
	mov A, #'A'
	call sendCarac
	ret

second_line:
	clr RS          ; modo comando
	mov A, #0C0h    ; início da 2ª linha
	call sendCarac
	ret

funcS:
	; DAT = 02h
	clr P1.7
	clr P1.6
	setb P1.5
	clr P1.4
	
	call clock
	call delay
	call clock

	; DAT = 08h
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

	; DAT = 0Fh
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

	; DAT = 06h
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

keycode00:
	setb f0
	call ESPSOL
	mov r6, #'1'
	call write_carac
	mov r7, #'*'
	ret

keycode01:
	setb f0
	call ESPSOL
	mov r6, #'2'
	call write_carac
	mov r7, #'*'
	ret

keycode02:
	setb f0
	call ESPSOL
	mov r6, #'3'
	call write_carac
	mov r7, #'*'
	ret

lcode1:
	jnb P0.4, keycode12
	jnb P0.5, keycode11
	jnb P0.6, keycode10
	ret

keycode10:
	setb f0
	call ESPSOL
	mov r6, #'4'
	call write_carac
	mov r7, #'*'
	ret

keycode11:
	setb f0
	call ESPSOL
	mov r6, #'5'
	call write_carac
	mov r7, #'*'
	ret

keycode12:
	setb f0
	call ESPSOL
	mov r6, #'6'
	call write_carac
	mov r7, #'*'
	ret

lcode2:
	jnb P0.4, keycode22
	jnb P0.5, keycode21
	jnb P0.6, keycode20
	ret

keycode20:
	setb f0
	call ESPSOL
	mov r6, #'7'
	call write_carac
	mov r7, #'*'
	ret

keycode21:
	setb f0
	call ESPSOL
	mov r6, #'8'
	call write_carac
	mov r7, #'*'
	ret

keycode22:
	setb f0
	call ESPSOL
	mov r6, #'9'
	call write_carac
	mov r7, #'*'
	ret

lcode3:
	jnb P0.4, keycode32
	jnb P0.5, keycode31
	jnb P0.6, keycode30
	ret

keycode30:
	setb f0
	call ESPSOL
	mov r6, #'*'
	call write_carac
	mov r7, #'*'
	ret

keycode31:
	setb f0
	call ESPSOL
	mov r6, #'0'
	call write_carac
	mov r7, #'*'
	ret

keycode32:
	setb f0
	call ESPSOL
	mov r6, #'#'
	call write_carac
	mov r7, #'*'	
	ret

write_carac:
	mov A, count
	add A, #bottom_password   ; A = endereço final
	mov R0, A            ; R0 = ponteiro

	mov A, r6            ; A = valor digitado
	mov @R0, A           ; salva na RAM

	inc count
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

rom_password:
	DB '8','2','7','1'
