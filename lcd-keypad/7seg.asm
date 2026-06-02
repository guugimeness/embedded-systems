org 0h

; Bits para selecionar o display
DISPS0 equ P3.3 
DISPS1 equ P3.4
CS equ P0.7

main:
    ; Display 3 mostra 8
    call write8
    call dispSelec3
    call delay_200us

    ; Display 2 mostra 2
    call write2
    call dispSelec2
    call delay_200us

    ; Display 1 mostra 0
    call write0
    call dispSelec1
    call delay_200us

    ; Display 0 mostra 7
    call write7
    call dispSelec0
    call delay_200us

    sjmp main

write8:
    clr P1.0
    clr P1.1
    clr P1.2
    clr P1.3
    clr P1.4
    clr P1.5
    clr P1.6
    setb P1.7
    ret

write2:
    clr P1.0
    clr P1.1
    setb P1.2
    clr P1.3
    clr P1.4
    setb P1.5
    clr P1.6

    setb P1.7
    ret

write0:
    clr P1.0
    clr P1.1
    clr P1.2
    clr P1.3
    clr P1.4
    clr P1.5
    setb P1.6

    setb P1.7
    ret

write7:
    clr P1.0
    clr P1.1
    clr P1.2
    setb P1.3
    setb P1.4
    setb P1.5
    setb P1.6

    setb P1.7
    ret

dispSelec0:
    clr DISPS0
    clr DISPS1
    setb CS
    ret

dispSelec1:
    setb DISPS0
    clr DISPS1
    setb CS
    ret

dispSelec2:
    clr DISPS0
    setb DISPS1
    setb CS
    ret

dispSelec3:
    setb DISPS0
    setb DISPS1
    setb CS
    ret

delay_200us:
    mov r0, #064h
    djnz r0, $
    ret