#include <reg51.h>

// Definições de pinos
sbit RS = P1^3;
sbit EN = P1^2;

// Constantes
#define DELAY_VAL 0x8ACF

// Variáveis globais
unsigned char bottom_password[4];
unsigned char count = 0;
unsigned char rom_password[4] = {'1','2','3','4'};

// Protótipos
void delay(void);
void clock(void);
void sendCarac(unsigned char c);
void funcS(void);
void dispC(void);
void entryM(void);
void scanKey(void);
void verify_password(void);
void correct_password(void);
void wrong_password(void);
void second_line(void);
void ESPSOL(void);

// ================= MAIN =================
// Função principal: inicializa o LCD, exibe "SENHA:" e fica lendo teclas
// até que 4 caracteres sejam digitados. Depois verifica a senha.
void main() {
    RS = 0;
    count = 0;

    funcS();
    dispC();
    entryM();

    // print "SENHA:"
    RS = 1;
    sendCarac('S');
    sendCarac('e');
    sendCarac('n');
    sendCarac('h');
    sendCarac('a');
    sendCarac(':');

    while (1) {
        scanKey();

        RS = 1;
        sendCarac('*');  // mostra * no display

        if (count == 4) {
            verify_password();
            while(1); // trava após verificação
        }
    }
}

// ================= VERIFICAÇÃO =================
// Compara a senha digitada com a senha correta armazenada
void verify_password() {
    unsigned char i;

    for (i = 0; i < 4; i++) {
        if (bottom_password[i] != rom_password[i]) {
            wrong_password();
            return;
        }
    }
    correct_password();
}

// ================= RESULTADOS =================
// Exibe "BEM VINDO" na segunda linha do LCD
void correct_password() {
    second_line();

    RS = 1;
    sendCarac('B');
    sendCarac('E');
    sendCarac('M');
    sendCarac(' ');
    sendCarac('V');
    sendCarac('I');
    sendCarac('N');
    sendCarac('D');
    sendCarac('O');
}

// Exibe "SENHA ERRADA" na segunda linha do LCD
void wrong_password() {
    second_line();

    RS = 1;
    sendCarac('S');
    sendCarac('E');
    sendCarac('N');
    sendCarac('H');
    sendCarac('A');
    sendCarac(' ');
    sendCarac('E');
    sendCarac('R');
    sendCarac('R');
    sendCarac('A');
    sendCarac('D');
    sendCarac('A');
}

// ================= LCD =================
// Move o cursor do LCD para o início da segunda linha
void second_line() {
    RS = 0;
    sendCarac(0xC0);
}

// Envia um byte (comando ou caractere) para o LCD em modo 4 bits
// Divide o byte em dois nibbles e usa o sinal EN para confirmar envio
void sendCarac(unsigned char c) {
    // nibble alto
    P1 = (P1 & 0x0F) | (c & 0xF0);
    clock();

    // nibble baixo
    P1 = (P1 & 0x0F) | ((c << 4) & 0xF0);
    clock();

    delay();
}

// Gera um pulso no pino EN para fazer o LCD ler os dados
void clock() {
    EN = 1;
    EN = 0;
}

// Gera um pequeno atraso para o LCD processar comandos
void delay() {
    unsigned int i;
    for (i = 0; i < 500; i++);
}

// ================= INICIALIZAÇÃO LCD =================
// Coloca o LCD em modo de operação de 4 bits
void funcS() {
    P1 = 0x20;
    clock();
    delay();
    clock();

    P1 = 0x80;
    clock();
    delay();
}

// Liga o display e ativa cursor com piscar
void dispC() {
    P1 = 0x00;
    clock();

    P1 = 0xF0;
    clock();
    delay();
}

// Define o modo de entrada: cursor avança para a direita
void entryM() {
    P1 = 0x00;
    clock();

    P1 = 0x60;
    clock();
    delay();
}

// ================= TECLADO =================
// Varre o teclado matricial e identifica qual tecla foi pressionada
// Armazena o valor digitado no vetor de senha
void scanKey() {
    while (1) {
        // Linha 0
        P0 = 0xF7;
        if (!(P0 & 0x10)) { bottom_password[count++] = '3'; ESPSOL(); return; }
        if (!(P0 & 0x20)) { bottom_password[count++] = '2'; ESPSOL(); return; }
        if (!(P0 & 0x40)) { bottom_password[count++] = '1'; ESPSOL(); return; }

        // Linha 1
        P0 = 0xFB;
        if (!(P0 & 0x10)) { bottom_password[count++] = '6'; ESPSOL(); return; }
        if (!(P0 & 0x20)) { bottom_password[count++] = '5'; ESPSOL(); return; }
        if (!(P0 & 0x40)) { bottom_password[count++] = '4'; ESPSOL(); return; }

        // Linha 2
        P0 = 0xFD;
        if (!(P0 & 0x10)) { bottom_password[count++] = '9'; ESPSOL(); return; }
        if (!(P0 & 0x20)) { bottom_password[count++] = '8'; ESPSOL(); return; }
        if (!(P0 & 0x40)) { bottom_password[count++] = '7'; ESPSOL(); return; }

        // Linha 3
        P0 = 0xFE;
        if (!(P0 & 0x10)) { bottom_password[count++] = '#'; ESPSOL(); return; }
        if (!(P0 & 0x20)) { bottom_password[count++] = '0'; ESPSOL(); return; }
        if (!(P0 & 0x40)) { bottom_password[count++] = '*'; ESPSOL(); return; }
    }
}

// ================= DEBOUNCE =================
// Evita múltiplas leituras da mesma tecla (efeito bounce)
void ESPSOL() {
    unsigned int i;
    for (i = 0; i < 31000; i++);
}