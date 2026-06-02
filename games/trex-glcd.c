#include <reg51.h>

/* ================= PINOS ================= */
#define GlcdDataBus P3

sbit RS  = P2^0;
sbit RW  = P2^1;
sbit EN  = P2^2;
sbit PSB = P2^3;
sbit RST = P2^5;

/* ================= VARIÁVEIS GLOBAIS ================= */
// Pontuação separada por dígitos elimina cálculos pesados de divisão/módulo no 8051
unsigned char score_d1 = 0; // unidade
unsigned char score_d2 = 0; // dezena
unsigned char score_d3 = 0; // centena

unsigned char speed_counter = 0; 
unsigned char speed = 1;       // velocidade (passo) de scroll
unsigned char scroll_byte = 0; // byte de scroll para o mapa
bit dino_position = 0;         // posição do dinossauro (0 = chão, 1 = ar)
bit game_over = 0;

// Variáveis de controle de tempo e modo dia/noite
unsigned char game_tick = 0;     // Desacopla leitura do botão (Controla a velocidade do jogo)
unsigned char night_counter = 0; // Contador para a troca de dia/noite
bit day_night_mode = 0;          // 0 = Dia (Fundo claro), 1 = Noite (Fundo escuro)

/* ================= SPRITE E FONTES ================= */
code unsigned char dino_bitmap[32] = {
    0x00, 0x3E, 0x00, 0x6F, 0x80, 0x7F, 0x80, 0x7F, 
    0x80, 0x70, 0xC0, 0xFC, 0xE7, 0xF8, 0xFF, 0xFE, 
    0x7F, 0xFA, 0x3F, 0xF0, 0x1F, 0xE0, 0x0F, 0xE0, 
    0x0E, 0xC0, 0x0C, 0xC0, 0x0C, 0xC0, 0x0E, 0xE0
};

code unsigned char font[10][8] = {
    {0x3C,0x66,0x6E,0x76,0x66,0x66,0x3C,0x00}, // '0'
    {0x18,0x38,0x18,0x18,0x18,0x18,0x7E,0x00}, // '1'
    {0x3C,0x66,0x06,0x0C,0x30,0x60,0x7E,0x00}, // '2'
    {0x3C,0x66,0x06,0x1C,0x06,0x66,0x3C,0x00}, // '3'
    {0x0C,0x1C,0x2C,0x4C,0x7E,0x0C,0x0C,0x00}, // '4'
    {0x7E,0x60,0x7C,0x06,0x06,0x66,0x3C,0x00}, // '5'
    {0x3C,0x66,0x60,0x7C,0x66,0x66,0x3C,0x00}, // '6'
    {0x7E,0x06,0x0C,0x18,0x30,0x60,0x60,0x00}, // '7'
    {0x3C,0x66,0x66,0x3C,0x66,0x66,0x3C,0x00}, // '8'
    {0x3C,0x66,0x66,0x3E,0x06,0x66,0x3C,0x00}  // '9'
};

/* ================= MAPA COMPRIMIDO ================= */
code unsigned char map_9_12[56] = {
    0x00,0x00,0x06,0x00,0x00,0x00,0x00,0x00,0x00,0x1C,0x00,0x00,0x00,0x00,
    0x00,0x00,0x1F,0x00,0x00,0x00,0x00,0x00,0x00,0x7E,0x00,0x00,0x00,0x00,
    0x00,0x00,0x3F,0x80,0x00,0x00,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0x00,
    0x00,0x00,0x7F,0xC0,0x00,0x00,0x00,0x00,0x01,0xFF,0x80,0x00,0x00,0x00
};
code unsigned char map_18_22[70] = {
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xC0,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x03,0xE0,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x07,0xF0,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0F,0xF8,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x1F,0xFC,0x00
};
code unsigned char map_38_43[84] = {
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x02,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x02,0x20,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x02,0x20,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x03,0x20,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x40,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0xC0,0x00,0x00,0x00,0x00,0x00
};
code unsigned char map_49[14] = {0x00,0x00,0x00,0x00,0x33,0x00,0x00,0x00,0x00,0x00,0x00,0xCC,0x0C,0xC0};
code unsigned char map_51[14] = {0x00,0x00,0x00,0x00,0x33,0x30,0x00,0x00,0x00,0x00,0x00,0xCC,0xCC,0xCC};
code unsigned char map_55[14] = {0x00,0x00,0x00,0x00,0x3F,0xF0,0x00,0x00,0x00,0x00,0x00,0xFF,0xCF,0xFC};
code unsigned char map_57[14] = {0x00,0x00,0x00,0x00,0x03,0x00,0x00,0x00,0x00,0x00,0x00,0x0C,0x00,0xC0};

/* ================= PROTOTIPOS ================= */
void glcdDelay(void);
void glcdBusyWait(void);
void glcdWriteCmd(unsigned char cmd);
void glcdWriteData(unsigned char dat);
void glcdInit(void);
void glcdClear(void);
void glcdSetAddr(unsigned char px, unsigned char py);
void draw16x16(unsigned char x, unsigned char y, unsigned char *sprite);
void drawDino(bit dino_position);
void drawChar(unsigned char x, unsigned char y, unsigned char c);
unsigned char getMapByte(unsigned char linha, unsigned char col);
void drawMap(unsigned char scroll_byte);
void drawScore(void);
void drawGameOver(void);
void scanJump(void);
bit checkCollision(void);
void updateGame(void);
void delayMs(unsigned int ms);

/* ================= BAIXO NÍVEL GLCD ===================== */
void glcdDelay(void) {
    unsigned char i;
    for(i = 0; i < 5; i++);
}

void glcdBusyWait(void) {
    unsigned char st;
    GlcdDataBus = 0xFF;
    RS = 0; RW = 1;
    do {
        EN = 1; glcdDelay();
        st = GlcdDataBus;
        EN = 0; glcdDelay();
    } while(st & 0x80);
    RW = 0;
}

void glcdWriteCmd(unsigned char cmd) {
    glcdBusyWait();
    RS = 0; RW = 0;
    GlcdDataBus = cmd;
    EN = 1; glcdDelay(); EN = 0; glcdDelay();
}

void glcdWriteData(unsigned char dat) {
    glcdBusyWait();
    RS = 1; RW = 0;
    
    // Inversão bit a bit para simular o modo Noite
    if (day_night_mode) {
        GlcdDataBus = ~dat; 
    } else {
        GlcdDataBus = dat;
    }
    
    EN = 1; glcdDelay(); EN = 0; glcdDelay();
}

void glcdInit(void) {
    PSB = 1;
    RST = 0; glcdDelay(); glcdDelay();
    RST = 1; glcdDelay(); glcdDelay();
    glcdWriteCmd(0x30); glcdWriteCmd(0x30);
    glcdWriteCmd(0x0C); glcdWriteCmd(0x01);
    glcdWriteCmd(0x34); glcdWriteCmd(0x36);
}

void glcdClear(void) {
    unsigned char i, j;
    for(i = 0; i < 64; i++) {
        glcdWriteCmd(0x80 | i);
        for(j = 0; j < 16; j++) {
            glcdWriteData(0x00);
            glcdWriteData(0x00);
        }
    }
}

void glcdSetAddr(unsigned char px, unsigned char py) {
    unsigned char gy, gx;
    if(py < 32) { gy = py; gx = px >> 4; } 
    else { gy = py - 32; gx = (px >> 4) + 8; }
    glcdWriteCmd(0x80 | gy);
    glcdWriteCmd(0x80 | gx);
}

/* ================= DESENHO E MAPEAMENTO ================= */
unsigned char getMapByte(unsigned char linha, unsigned char col) {
    if (linha >= 57) return map_57[col];
    if (linha >= 55) return map_55[col];
    if (linha >= 51) return map_51[col];
    if (linha >= 49) return map_49[col];
    if (linha >= 38 && linha <= 43) return map_38_43[(linha - 38) * 14 + col];
    if (linha >= 18 && linha <= 22) return map_18_22[(linha - 18) * 14 + col];
    if (linha >= 9  && linha <= 12) return map_9_12[(linha - 9) * 14 + col];
    return 0x00;
}

void draw16x16(unsigned char px, unsigned char py, unsigned char *sprite) {
    unsigned char i;
    for(i = 0; i < 16; i++) {
        glcdSetAddr(px, py + i);
        if (sprite == 0) {
            glcdWriteData(0x00);
            glcdWriteData(0x00);
        } else {
            glcdWriteData(sprite[i * 2]);
            glcdWriteData(sprite[i * 2 + 1]);
        }
    }
}

/* ATENÇÃO: drawChar completamente corrigida para funcionar no ST7920 */
void drawChar(unsigned char x, unsigned char y, unsigned char c) {
    unsigned char i;
    // ST7920 usa palavras de 16-bits para desenhar (2 bytes).
    // Percorremos de cima para baixo (y + i) enviando os 8 bits do char e 8 bits de padding.
    for(i = 0; i < 8; i++) {
        glcdSetAddr(x, y + i);        // y é a linha da tela (0-63)
        glcdWriteData(font[c][i]);    // 1º Byte: Metade esquerda (onde a fonte é impressa)
        glcdWriteData(0x00);          // 2º Byte: Metade direita (Preenchimento)
    }
}

void drawDino(bit dino_position) {
    if (dino_position == 0) {
        draw16x16(0, 32, 0);
        draw16x16(0, 48, dino_bitmap);
    } else {
        draw16x16(0, 48, 0);
        draw16x16(0, 32, dino_bitmap);
    }
		if (day_night_mode == 1) {
        draw16x16(0, 0, 0);   // y =  0..15
        draw16x16(0, 16, 0);  // y = 16..31
        // A faixa 32..47 já é limpa pelo próprio código (quando dino está no ar, limpamos 48;
        // e quando está no chão, o draw16x16(0,32,0) garante a limpeza). Portanto, apenas
        // as duas primeiras faixas são necessárias para cobrir tudo acima do sprite.
    }
}

void drawMap(unsigned char scroll) {
    unsigned char linha, col_byte, col_idx;
    for (linha = 0; linha < 64; linha++) {
        glcdSetAddr(16, linha);
        // O loop varre as 14 colunas de bytes enviando exatos 14 bytes (7 words de 16bits)
        for (col_byte = 0; col_byte < 14; col_byte++) {
            col_idx = scroll + col_byte;
            if (col_idx >= 14) col_idx -= 14;
            
            glcdWriteData(getMapByte(linha, col_idx));
        }
    }
}

void drawScore(void) {
    // Coordenadas X precisam ser múltiplas de 16 (16, 32, 48, ... 80, 96, 112)
    drawChar(80, 0, score_d3);  // Centena
    drawChar(96, 0, score_d2);  // Dezena
    drawChar(112, 0, score_d1); // Unidade
}

void drawGameOver(void) {
    glcdClear();
    // Exibe pontuação final centralizada com precisão (X também são múltiplos de 16)
    drawChar(48, 28, score_d3);
    drawChar(64, 28, score_d2);
    drawChar(80, 28, score_d1);
}

/* ================= LÓGICA DO JOGO ======================= */
void scanJump() {
    P0 = 0xFD;
    dino_position = !(P0 & 0x20);
}

bit checkCollision(void) {
    unsigned char dx, dy;
    unsigned char y_dino = (dino_position == 0) ? 48 : 32;
    unsigned char dino_byte, dino_bit;
    unsigned char y, mx, map_x, map_byte, map_bit;

    for (dy = 0; dy < 16; dy++) {
        y = y_dino + dy;
        if (y >= 64) continue;

        for (dx = 0; dx < 16; dx++) {
            dino_byte = dy * 2 + (dx >> 3);
            dino_bit = 0x80 >> (dx & 0x07);

            if (!(dino_bitmap[dino_byte] & dino_bit)) continue;

            mx = scroll_byte * 8 + dx;
            if (mx < 16) continue;
            map_x = mx - 16;

            map_byte = map_x >> 3;
            map_bit  = 0x80 >> (map_x & 0x07);

            if (getMapByte(y, map_byte) & map_bit)
                return 1; 
        }
    }
    return 0;
}

void updateGame(void) {
    scroll_byte += speed;
    if (scroll_byte >= 14) scroll_byte -= 14; 

    // Incremento manual de pontuação livre de divisões pesadas
    score_d1++;
    if (score_d1 > 9) {
        score_d1 = 0;
        score_d2++;
        if (score_d2 > 9) {
            score_d2 = 0;
            score_d3++;
            if (score_d3 > 9) score_d3 = 9; 
        }
    }

    // Lógica para alternar Dia e Noite a cada 50 pontos
    night_counter++;
    if (night_counter >= 50) {
        night_counter = 0;
        day_night_mode = !day_night_mode; // Inverte o estado da tela (Dia/Noite)
    }

    // Aumento progressivo de velocidade do mapa
    speed_counter++;
    if (speed_counter >= 100) { // Agora demora muito mais para ficar mais difícil
        speed_counter = 0;
        if (speed < 14) speed++;
    }
}

void delayMs(unsigned int ms) {
    unsigned int i;
    unsigned char j;
    for(i = 0; i < ms; i++)
        for(j = 0; j < 125; j++);
}

/* ========================= MAIN ========================== */
void main(void) {
    glcdInit();
    glcdClear();

    while(1) {
        // Se o jogo acabou, congela e aguarda pulo pra resetar
        if (game_over) {
            if ((P0 & 0x20) == 0) {
                score_d1 = 0;
                score_d2 = 0;
                score_d3 = 0;
                night_counter = 0;
                day_night_mode = 0; // Inicia sempre no modo Dia (fundo claro)
                speed_counter = 0;
                speed = 1;
                scroll_byte = 0;
                dino_position = 0;
                game_tick = 0;
                game_over = 0;
                glcdClear();
                delayMs(200); // Debounce pro botão
            }
            delayMs(10);
            continue; // Evita atualizar a lógica enquanto espera
        }

        // Leitura contínua do botão a cada 10ms (Rápido, sem atrasos)
        scanJump();

        // Controla a velocidade gráfica: Jogo bem mais lento (8 loops)
        game_tick++;
        if (game_tick >= 8) {  // Aumentei de 4 para 8. Reduz a velocidade do mapa pela metade
            game_tick = 0;

            drawMap(scroll_byte);
            drawDino(dino_position);
            drawScore(); // Assegura a pontuação por cima do cenário
            updateGame();

            if (checkCollision()) {
                game_over = 1;
                drawGameOver(); // Atualiza a tela preta com pontos UMA vez
            }
        }

        delayMs(10); // Loop base de tempo
    }
}