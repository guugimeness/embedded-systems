#include <reg51.h>

// ================= PINOS =================
sbit RS = P1^3;
sbit EN = P1^2;

// ================= PROTÓTIPOS =================
void delay(void);
void gameDelay(unsigned int speed);
void clock(void);
void sendCarac(unsigned char c);

void funcS(void);
void dispC(void);
void entryM(void);

void first_line(void);
void second_line(void);

void createChar(unsigned char addr, unsigned char *pattern);

void drawScreen(void);
void clearWorld(void);
void updateGame(void);
bit scanJump(void);
bit checkCollision(void);
void resetGame(void);
void clearDisplay(void);

// ================= SPRITES =================
code unsigned char cacto[8] = {
    0x04,
    0x05,
    0x15,
    0x15,
    0x1F,
    0x04,
    0x04,
    0x00
};

code unsigned char cacto_noite[8] = {
    0x1B,
    0x1B,
    0x0A,
    0x0A,
    0x00,
    0x1B,
    0x1B,
    0x1F
};

code unsigned char bird[8] = {
    0x00,
    0x00,
    0x11,
    0x0A,
    0x0E,
    0x04,
    0x00,
    0x00
};

code unsigned char bird_noite[8] = {
    0x1F,
    0x1F,
    0x0E,
    0x15,
    0x11,
    0x1B,
    0x1F,
    0x1F
};

code unsigned char trex[8] = {
    0x00,
    0x07,
    0x17,
    0x16,
    0x16,
    0x1F,
    0x1D,
    0x0C
};

code unsigned char trex_noite[8] = {
    0x1F,
    0x18,
    0x08,
    0x09,
    0x09,
    0x00,
    0x02,
    0x13
};

// ================= MUNDO =================
unsigned char cactos[40];
unsigned char passaros[40];
unsigned char offset = 0;
bit trexPulando = 0;
unsigned int score = 0;
unsigned int count_pulo = 0;
unsigned int gameSpeed = 40000;
unsigned char centenas;
unsigned char dezenas;
unsigned char unidades;
bit modoNoite = 0;

// ================= MAIN =================
void main() {

    RS = 0;

    funcS();
    dispC();
    entryM();

    createChar(0, cacto);
    createChar(1, bird);
    createChar(2, trex);

    clearWorld();

    // Obstáculos iniciais
    cactos[10] = 1;
    cactos[17] = 1;
    cactos[28] = 1;
    cactos[35] = 1;

    passaros[5]  = 1;
    passaros[22] = 1;

    while(1) {

        drawScreen();

        gameDelay(gameSpeed);

        updateGame();

        if(checkCollision()) {

            resetGame();
        }
    }
}

// ================= UPDATE =================
void updateGame() {

    // ================= PULO =================
    if(scanJump()) {
        count_pulo++;

        if(count_pulo >= 4) {
            score = score - 2;
        }
        trexPulando = 1;
    }
    else {
        if(count_pulo>0){
            count_pulo--;   
        }
        trexPulando = 0;
    }

    // ================= SCORE =================
    score++;

    centenas = (score / 100) % 10;
    dezenas  = (score / 10) % 10;
    unidades = score % 10;

    if(score % 25 == 0 && gameSpeed > 2000) {

        gameSpeed = (gameSpeed * 2) / 3;
    }

    if(score >= 100 && !modoNoite) {
        modoNoite = 1;

        createChar(0, cacto_noite);
        createChar(1, bird_noite);
        createChar(2, trex_noite);
    }

    // ================= SCROLL =================
    offset++;

    if(offset >= 40)
        offset = 0;
}

// ================= RENDER =================
void drawScreen() {

    unsigned char i;
    unsigned char idx;

    // -------- LINHA DE CIMA --------
    first_line();

    for(i = 0; i < 13; i++) {

        idx = (offset + i) % 40;

        RS = 1;

        if(i == 0 && trexPulando) {
            sendCarac(2);
        }
        else if(passaros[idx]) {
            sendCarac(1);
        }
        else {
            if(modoNoite)
                sendCarac(0xFF);
            else
                sendCarac(' ');        
        }
    }
    
    RS = 1;

    sendCarac(centenas + '0');
    sendCarac(dezenas + '0');
    sendCarac(unidades + '0');

    // -------- LINHA DE BAIXO --------
    second_line();

    for(i = 0; i < 16; i++) {

        idx = (offset + i) % 40;

        RS = 1;

        if(i == 0 && !trexPulando) {
            sendCarac(2);
        }
        else if(cactos[idx]) {
            sendCarac(0);
        }
        else {
            if(modoNoite)
                sendCarac(0xFF);
            else
                sendCarac(' ');
        }
    }
}

// ================= LIMPA MUNDO =================
void clearWorld() {

    unsigned char i;

    for(i = 0; i < 40; i++) {
        cactos[i] = 0;
        passaros[i] = 0;
    }
}

void clearDisplay() {

    RS = 0;

    sendCarac(0x01);

    delay();
}

bit checkCollision() {

    unsigned char trexIdx;

    trexIdx = (offset + 0) % 40;

    // Colisão com cacto
    if(cactos[trexIdx] && !trexPulando) {
        return 1;
    }

    // Colisão com pássaro
    if(passaros[trexIdx] && trexPulando) {
        return 1;
    }

    return 0;
}

void resetGame() {

    clearWorld();

    clearDisplay();

    first_line();

    RS = 1;

    sendCarac('G');
    sendCarac('A');
    sendCarac('M');
    sendCarac('E');

    sendCarac(' ');

    sendCarac('O');
    sendCarac('V');
    sendCarac('E');
    sendCarac('R');


    second_line();

    RS = 1;

    sendCarac('S');
    sendCarac('c');
    sendCarac('o');
    sendCarac('r');
    sendCarac('e');
    sendCarac(':');
    sendCarac(' ');
    sendCarac(((score / 100) % 10) + '0');
    sendCarac(((score / 10) % 10) + '0');
    sendCarac((score % 10) + '0');

    gameDelay(30000);

    while(!scanJump());

    // Reinicia variáveis
    offset = 0;

    score = 0;
    count_pulo = 0;

    trexPulando = 0;
    modoNoite = 0;

    createChar(0, cacto);
    createChar(1, bird);
    createChar(2, trex);

    // Obstáculos iniciais
    cactos[10] = 1;
    cactos[17] = 1;
    cactos[28] = 1;
    cactos[35] = 1;

    passaros[5]  = 1;
    passaros[22] = 1;

    gameSpeed = 20000;
}

// ================= LCD =================
void first_line() {
    RS = 0;
    sendCarac(0x80);
}

void second_line() {
    RS = 0;
    sendCarac(0xC0);
}

// ================= CGRAM =================
void createChar(unsigned char addr, unsigned char *pattern) {

    unsigned char i;

    RS = 0;

    sendCarac(0x40 + (addr * 8));

    for(i = 0; i < 8; i++) {

        RS = 1;
        sendCarac(pattern[i]);
    }
}

// ================= LCD SEND =================
void sendCarac(unsigned char c) {

    // nibble alto
    P1 = (P1 & 0x0F) | (c & 0xF0);
    clock();

    // nibble baixo
    P1 = (P1 & 0x0F) | ((c << 4) & 0xF0);
    clock();

    delay();
}

// ================= CLOCK =================
void clock() {

    EN = 1;
    EN = 0;
}

// ================= DELAY LCD =================
void delay() {

    unsigned int i;

    for(i = 0; i < 500; i++);
}

// ================= DELAY JOGO =================
void gameDelay(unsigned int speed) {

    unsigned int i;

    for(i = 0; i < speed; i++);
}

// ================= LCD INIT =================
void funcS() {

    P1 = 0x20;
    clock();

    delay();

    clock();

    P1 = 0x80;
    clock();

    delay();
}

void dispC() {

    P1 = 0x00;
    clock();

    P1 = 0xF0;
    clock();

    delay();
}

void entryM() {

    P1 = 0x00;
    clock();

    P1 = 0x60;
    clock();

    delay();
}

bit scanJump() {

    // Linha 2
    P0 = 0xFD;

    // Botão 8
    if (!(P0 & 0x20)) {
        return 1;
    }

    return 0;
}