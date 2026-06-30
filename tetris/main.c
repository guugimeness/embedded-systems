/*
 * ============================================================
 *  TETRIS ARCADE  —  DE10-Standard  |  ARM Cortex-A9 Bare-Metal
 * ============================================================
 *
 *  Hardware alvo   : Terasic DE10-Standard (SoCFPGA Cyclone V)
 *  Compilador      : arm-altera-eabi-gcc  (compilar_antigravity.bat)
 *  VGA             : 320x240 px, 16-bit RGB565
 *                    pitch = 1024 bytes/linha  (512 px, power-of-2)
 *  Char buffer     : 80 col x 60 lin  @ 0xC9000000
 *  Botoes (KEY_BASE = 0xFF200050)
 *    KEY0  -> Mover Esquerda
 *    KEY1  -> Rotar
 *    KEY2  -> Mover Direita
 *    KEY3  -> Drop instantaneo / Acelerar
 *
 *  Estrutura do codigo:
 *    1. Includes / Defines / Tipos
 *    2. Dados dos tetrominoes (formas + cores)
 *    3. Primitivas graficas (pixel, retangulo, linha, char, numero)
 *    4. Logica do Tetris (grid, colisao, rotacao, limpeza)
 *    5. Maquina de Estados (MENU -> JOGO -> GAME_OVER)
 *    6. main()
 * ============================================================
 */

#include <stdint.h>
#include "address_map_arm.h"

/* ============================================================
 *  SECAO 1 — DEFINES / CONSTANTES / TIPOS
 * ============================================================ */

/* --- Tela VGA ------------------------------------------------ */
#define SCREEN_W    320
#define SCREEN_H    240

/* --- Grid do Tetris ----------------------------------------- */
#define GRID_COLS   10          /* largura em celulas            */
#define GRID_ROWS   20          /* altura  em celulas            */
#define CELL_W       9          /* pixels por celula (horizontal)*/
#define CELL_H      10          /* pixels por celula (vertical)  */

/* Posicao do grid no ecra (pixel superior-esquerdo) */
/* Tela VGA 320x240. Grid = 90x200 px. Para centralizar:
   margem lateral disponivel = 320 - 90 - 115(painel) = 115px
   Centralizamos o grid no lado esquerdo: x = (160-90)/2 = 35 */
#define GRID_X      30          /* margem esquerda               */
#define GRID_Y      20          /* margem superior               */

/* Largura/altura total do grid em pixels */
#define GRID_PX_W   (GRID_COLS * CELL_W)   /* = 90              */
#define GRID_PX_H   (GRID_ROWS * CELL_H)   /* = 200             */

/* --- Painel direito ----------------------------------------- */
/* Separador em x=160 (metade da tela). Painel vai de 160 a 319 */
#define PANEL_X     160         /* divisor vertical (metade)     */
#define PANEL_W     160
#define NEXT_BOX_X  213         /* caixa "NEXT PIECE" x (centrada no painel) */
#define NEXT_BOX_Y  136         /* caixa "NEXT PIECE" y (ajustada ao label)  */

/* --- Cores RGB565 ------------------------------------------- */
/*
 * rgb(r,g,b) runtime helper — usado para calculos dinamicos.
 * Formula: ((r>>3)<<11) | ((g>>2)<<5) | (b>>3)
 */
static inline uint16_t rgb(uint8_t r, uint8_t g, uint8_t b) {
    return (uint16_t)(((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3));
}

/* Paleta principal — literais hex pre-calculados              */
/*  rgb(  0,  0,  0) */ #define COL_BLACK    ((uint16_t)0x0000u)
/*  rgb( 15, 15, 25) */ #define COL_DARK     ((uint16_t)0x0841u)
/*  rgb( 20, 20, 35) */ #define COL_GRID_BG  ((uint16_t)0x1084u)
/*  rgb( 80,200,255) */ #define COL_BORDER   ((uint16_t)0x2F1Fu)
/*  rgb(255,255,255) */ #define COL_WHITE    ((uint16_t)0xFFFFu)
/*  rgb(100,100,110) */ #define COL_GRAY     ((uint16_t)0x6B4Du)
/*  rgb( 10, 10, 20) */ #define COL_PANEL    ((uint16_t)0x0841u)
/*  rgb(255,210, 50) */ #define COL_GOLD     ((uint16_t)0xFD46u)
/*  rgb(255, 50, 50) */ #define COL_RED      ((uint16_t)0xF946u)

/*
 * Cores dos 7 tetrominoes — valores hex pre-calculados
 * Formula aplicada: val = ((r>>3)<<11) | ((g>>2)<<5) | (b>>3)
 *   I ( 0,230,255) -> (0<<11)|(57<<5)|(31)  = 0x073F
 *   J ( 0, 80,255) -> (0<<11)|(20<<5)|(31)  = 0x029F
 *   L (255,140,  0)-> (31<<11)|(35<<5)|(0)  = 0xF8E0
 *   O (255,220,  0)-> (31<<11)|(55<<5)|(0)  = 0xFEE0
 *   S (  0,220, 80)-> (0<<11)|(55<<5)|(10)  = 0x06EA
 *   T (180,  0,255)-> (22<<11)|(0<<5)|(31)  = 0xB01F
 *   Z (255, 30, 60)-> (31<<11)|(7<<5)|(7)   = 0xF8E7
 */
static const uint16_t PIECE_COLORS[7] = {
    0x073Fu,   /* 0 I - ciano neon    rgb(  0,230,255) */
    0x029Fu,   /* 1 J - azul royal    rgb(  0, 80,255) */
    0xF8E0u,   /* 2 L - laranja       rgb(255,140,  0) */
    0xFEE0u,   /* 3 O - amarelo       rgb(255,220,  0) */
    0x06EAu,   /* 4 S - verde neon    rgb(  0,220, 80) */
    0xB01Fu,   /* 5 T - roxo          rgb(180,  0,255) */
    0xF8E7u,   /* 6 Z - vermelho neon rgb(255, 30, 60) */
};

/*
 * Cores de borda (tom escuro da face) — hex pre-calculados
 *   I (  0,100,130)-> (0<<11)|(25<<5)|(16)  = 0x0330
 *   J (  0, 30,120)-> (0<<11)|(7<<5)|(15)   = 0x00EF
 *   L (130, 60,  0)-> (16<<11)|(15<<5)|(0)  = 0x81E0
 *   O (130,100,  0)-> (16<<11)|(25<<5)|(0)  = 0x8320
 *   S (  0,100, 30)-> (0<<11)|(25<<5)|(3)   = 0x0323
 *   T ( 80,  0,130)-> (10<<11)|(0<<5)|(16)  = 0x5010
 *   Z (130, 10, 20)-> (16<<11)|(2<<5)|(2)   = 0x8042
 */
static const uint16_t PIECE_DARK[7] = {
    0x0330u,   /* I */
    0x00EFu,   /* J */
    0x81E0u,   /* L */
    0x8320u,   /* O */
    0x0323u,   /* S */
    0x5010u,   /* T */
    0x8042u,   /* Z */
};

/* --- Estados do jogo ---------------------------------------- */
#define STATE_MENU      0
#define STATE_PLAY      1
#define STATE_GAMEOVER  2

/* --- Niveis / velocidade ------------------------------------ */
/* Com delay_ms(20) por frame: meio-termo entre rapido e lento.
 * Nivel 0: 40*20ms ~= 800ms/celula. Classico e jogavel.        */
static const int LEVEL_SPEED[10] = {
    40, 34, 28, 23, 18, 14, 10, 7, 5, 3
};
#define LINES_PER_LEVEL  2  /* linhas para avancar de nivel     */

/* ============================================================
 *  SECAO 2 — DADOS DOS TETROMINOES
 * ============================================================
 *
 *  Cada peca e representada em 4 rotacoes, em uma matriz 4x4.
 *  O bit [row][col] indica se a celula esta preenchida.
 *  Layout: shapes[tipo][rotacao][linha][coluna]
 */
static const uint8_t SHAPES[7][4][4][4] = {
    /* 0 — I */
    {
        {{0,0,0,0},{1,1,1,1},{0,0,0,0},{0,0,0,0}},
        {{0,0,1,0},{0,0,1,0},{0,0,1,0},{0,0,1,0}},
        {{0,0,0,0},{0,0,0,0},{1,1,1,1},{0,0,0,0}},
        {{0,1,0,0},{0,1,0,0},{0,1,0,0},{0,1,0,0}},
    },
    /* 1 — J */
    {
        {{1,0,0,0},{1,1,1,0},{0,0,0,0},{0,0,0,0}},
        {{0,1,1,0},{0,1,0,0},{0,1,0,0},{0,0,0,0}},
        {{0,0,0,0},{1,1,1,0},{0,0,1,0},{0,0,0,0}},
        {{0,1,0,0},{0,1,0,0},{1,1,0,0},{0,0,0,0}},
    },
    /* 2 — L */
    {
        {{0,0,1,0},{1,1,1,0},{0,0,0,0},{0,0,0,0}},
        {{0,1,0,0},{0,1,0,0},{0,1,1,0},{0,0,0,0}},
        {{0,0,0,0},{1,1,1,0},{1,0,0,0},{0,0,0,0}},
        {{1,1,0,0},{0,1,0,0},{0,1,0,0},{0,0,0,0}},
    },
    /* 3 — O */
    {
        {{0,1,1,0},{0,1,1,0},{0,0,0,0},{0,0,0,0}},
        {{0,1,1,0},{0,1,1,0},{0,0,0,0},{0,0,0,0}},
        {{0,1,1,0},{0,1,1,0},{0,0,0,0},{0,0,0,0}},
        {{0,1,1,0},{0,1,1,0},{0,0,0,0},{0,0,0,0}},
    },
    /* 4 — S */
    {
        {{0,1,1,0},{1,1,0,0},{0,0,0,0},{0,0,0,0}},
        {{0,1,0,0},{0,1,1,0},{0,0,1,0},{0,0,0,0}},
        {{0,0,0,0},{0,1,1,0},{1,1,0,0},{0,0,0,0}},
        {{1,0,0,0},{1,1,0,0},{0,1,0,0},{0,0,0,0}},
    },
    /* 5 — T */
    {
        {{0,1,0,0},{1,1,1,0},{0,0,0,0},{0,0,0,0}},
        {{0,1,0,0},{0,1,1,0},{0,1,0,0},{0,0,0,0}},
        {{0,0,0,0},{1,1,1,0},{0,1,0,0},{0,0,0,0}},
        {{0,1,0,0},{1,1,0,0},{0,1,0,0},{0,0,0,0}},
    },
    /* 6 — Z */
    {
        {{1,1,0,0},{0,1,1,0},{0,0,0,0},{0,0,0,0}},
        {{0,0,1,0},{0,1,1,0},{0,1,0,0},{0,0,0,0}},
        {{0,0,0,0},{1,1,0,0},{0,1,1,0},{0,0,0,0}},
        {{0,1,0,0},{1,1,0,0},{1,0,0,0},{0,0,0,0}},
    },
};

/* ============================================================
 *  SECAO 3 — PRIMITIVAS GRAFICAS
 * ============================================================ */

/*
 * plot_pixel — escreve um pixel em (x, y) com cor RGB565.
 * OBRIGATORIA: le o ponteiro do frame buffer do registrador VGA.
 */
static inline void plot_pixel(int x, int y, uint16_t color) {
    uint32_t pixel_buf_ptr = *(volatile uint32_t *) PIXEL_BUF_CTRL_BASE;
    uint32_t pixel_ptr = pixel_buf_ptr + (y << 10) + (x << 1);
    *(volatile uint16_t *) pixel_ptr = color;
}

/* Retangulo preenchido */
static void fill_rect(int x0, int y0, int w, int h, uint16_t color) {
    int x, y;
    for (y = y0; y < y0 + h && y < SCREEN_H; y++)
        for (x = x0; x < x0 + w && x < SCREEN_W; x++)
            if (x >= 0 && y >= 0)
                plot_pixel(x, y, color);
}

/* Linha horizontal */
static void hline(int x0, int x1, int y, uint16_t color) {
    int x;
    if (y < 0 || y >= SCREEN_H) return;
    for (x = x0; x <= x1; x++)
        if (x >= 0 && x < SCREEN_W)
            plot_pixel(x, y, color);
}

/* Linha vertical */
static void vline(int x, int y0, int y1, uint16_t color) {
    int y;
    if (x < 0 || x >= SCREEN_W) return;
    for (y = y0; y <= y1; y++)
        if (y >= 0 && y < SCREEN_H)
            plot_pixel(x, y, color);
}

/* Retangulo de borda (vazio) */
static void draw_rect(int x0, int y0, int w, int h, uint16_t color) {
    hline(x0, x0+w-1, y0,     color);
    hline(x0, x0+w-1, y0+h-1, color);
    vline(x0,     y0, y0+h-1, color);
    vline(x0+w-1, y0, y0+h-1, color);
}

/* ---------------------------------------------------------------
 *  ESCRITA NO CHARACTER BUFFER VGA
 *  O char buffer e uma grade de 80x60 caracteres ASCII.
 *  Cada posicao e um byte no endereco base 0xC9000000.
 *  Endereco = FPGA_CHAR_BASE + col + row*128
 *  (o IP usa pitch de 128 bytes por linha = power-of-2)
 * --------------------------------------------------------------- */
static void char_write(int col, int row, char c) {
    if (col < 0 || col >= 80 || row < 0 || row >= 60) return;
    volatile char *ptr = (volatile char *)(FPGA_CHAR_BASE + row * 128 + col);
    *ptr = c;
}

static void char_write_str(int col, int row, const char *s) {
    while (*s) {
        char_write(col++, row, *s++);
        if (col >= 80) break;
    }
}

/* Limpa toda a area do char buffer (espaco) */
static void char_clear_all(void) {
    int r, c;
    for (r = 0; r < 60; r++)
        for (c = 0; c < 80; c++)
            char_write(c, r, ' ');
}

/* Escreve numero inteiro nao-negativo em ate 6 digitos */
static void char_write_int(int col, int row, int val) {
    char buf[8];
    int i = 7;
    buf[7] = '\0';
    if (val == 0) { char_write(col, row, '0'); return; }
    while (val > 0 && i > 0) {
        buf[--i] = (char)('0' + (val % 10));
        val /= 10;
    }
    char_write_str(col, row, &buf[i]);
}

/* Escreve numero inteiro centralizado em torno da coluna 'center_col' */
static void char_write_int_centered(int center_col, int row, int val) {
    char buf[8];
    int i = 7, len;
    buf[7] = '\0';
    if (val == 0) {
        char_write(center_col, row, '0');
        return;
    }
    while (val > 0 && i > 0) {
        buf[--i] = (char)('0' + (val % 10));
        val /= 10;
    }
    len = 7 - i;   /* numero de digitos */
    char_write_str(center_col - len / 2, row, &buf[i]);
}

/* ---------------------------------------------------------------
 *  LETRAS GRANDES  (fonte bitmap 5x7, escala configuravel)
 *  Usadas para o titulo TETRIS e GAME OVER na tela VGA.
 * --------------------------------------------------------------- */

/* Bitmap 5x7 para caracteres ASCII 32..90 (espaco a Z) */
static const uint8_t FONT5X7[][7] = {
    /* ' ' */ {0x00,0x00,0x00,0x00,0x00,0x00,0x00},
    /* '!' */ {0x04,0x04,0x04,0x04,0x00,0x04,0x00},
    /* '"' */ {0x0A,0x0A,0x00,0x00,0x00,0x00,0x00},
    /* '#' */ {0x0A,0x1F,0x0A,0x0A,0x1F,0x0A,0x00},
    /* '$' */ {0x04,0x0F,0x14,0x0E,0x05,0x1E,0x04},
    /* '%' */ {0x18,0x19,0x02,0x04,0x08,0x13,0x03},
    /* '&' */ {0x0C,0x12,0x14,0x08,0x15,0x12,0x0D},
    /* ''' */ {0x06,0x04,0x08,0x00,0x00,0x00,0x00},
    /* '(' */ {0x02,0x04,0x08,0x08,0x08,0x04,0x02},
    /* ')' */ {0x08,0x04,0x02,0x02,0x02,0x04,0x08},
    /* '*' */ {0x00,0x04,0x15,0x0E,0x15,0x04,0x00},
    /* '+' */ {0x00,0x04,0x04,0x1F,0x04,0x04,0x00},
    /* ',' */ {0x00,0x00,0x00,0x00,0x06,0x04,0x08},
    /* '-' */ {0x00,0x00,0x00,0x1F,0x00,0x00,0x00},
    /* '.' */ {0x00,0x00,0x00,0x00,0x00,0x06,0x06},
    /* '/' */ {0x00,0x01,0x02,0x04,0x08,0x10,0x00},
    /* '0' */ {0x0E,0x11,0x13,0x15,0x19,0x11,0x0E},
    /* '1' */ {0x04,0x0C,0x04,0x04,0x04,0x04,0x0E},
    /* '2' */ {0x0E,0x11,0x01,0x06,0x08,0x10,0x1F},
    /* '3' */ {0x1F,0x02,0x04,0x02,0x01,0x11,0x0E},
    /* '4' */ {0x02,0x06,0x0A,0x12,0x1F,0x02,0x02},
    /* '5' */ {0x1F,0x10,0x1E,0x01,0x01,0x11,0x0E},
    /* '6' */ {0x06,0x08,0x10,0x1E,0x11,0x11,0x0E},
    /* '7' */ {0x1F,0x01,0x02,0x04,0x04,0x04,0x04},
    /* '8' */ {0x0E,0x11,0x11,0x0E,0x11,0x11,0x0E},
    /* '9' */ {0x0E,0x11,0x11,0x0F,0x01,0x02,0x0C},
    /* ':' */ {0x00,0x06,0x06,0x00,0x06,0x06,0x00},
    /* ';' */ {0x00,0x06,0x06,0x00,0x06,0x04,0x08},
    /* '<' */ {0x02,0x04,0x08,0x10,0x08,0x04,0x02},
    /* '=' */ {0x00,0x00,0x1F,0x00,0x1F,0x00,0x00},
    /* '>' */ {0x10,0x08,0x04,0x02,0x04,0x08,0x10},
    /* '?' */ {0x0E,0x11,0x01,0x06,0x04,0x00,0x04},
    /* '@' */ {0x0E,0x11,0x17,0x15,0x17,0x10,0x0F},
    /* 'A' */ {0x04,0x0A,0x11,0x11,0x1F,0x11,0x11},
    /* 'B' */ {0x1E,0x11,0x11,0x1E,0x11,0x11,0x1E},
    /* 'C' */ {0x0E,0x11,0x10,0x10,0x10,0x11,0x0E},
    /* 'D' */ {0x1C,0x12,0x11,0x11,0x11,0x12,0x1C},
    /* 'E' */ {0x1F,0x10,0x10,0x1E,0x10,0x10,0x1F},
    /* 'F' */ {0x1F,0x10,0x10,0x1E,0x10,0x10,0x10},
    /* 'G' */ {0x0E,0x11,0x10,0x17,0x11,0x11,0x0F},
    /* 'H' */ {0x11,0x11,0x11,0x1F,0x11,0x11,0x11},
    /* 'I' */ {0x0E,0x04,0x04,0x04,0x04,0x04,0x0E},
    /* 'J' */ {0x07,0x02,0x02,0x02,0x02,0x12,0x0C},
    /* 'K' */ {0x11,0x12,0x14,0x18,0x14,0x12,0x11},
    /* 'L' */ {0x10,0x10,0x10,0x10,0x10,0x10,0x1F},
    /* 'M' */ {0x11,0x1B,0x15,0x15,0x11,0x11,0x11},
    /* 'N' */ {0x11,0x19,0x15,0x13,0x11,0x11,0x11},
    /* 'O' */ {0x0E,0x11,0x11,0x11,0x11,0x11,0x0E},
    /* 'P' */ {0x1E,0x11,0x11,0x1E,0x10,0x10,0x10},
    /* 'Q' */ {0x0E,0x11,0x11,0x11,0x15,0x12,0x0D},
    /* 'R' */ {0x1E,0x11,0x11,0x1E,0x14,0x12,0x11},
    /* 'S' */ {0x0F,0x10,0x10,0x0E,0x01,0x01,0x1E},
    /* 'T' */ {0x1F,0x04,0x04,0x04,0x04,0x04,0x04},
    /* 'U' */ {0x11,0x11,0x11,0x11,0x11,0x11,0x0E},
    /* 'V' */ {0x11,0x11,0x11,0x11,0x0A,0x0A,0x04},
    /* 'W' */ {0x11,0x11,0x15,0x15,0x15,0x0A,0x0A},
    /* 'X' */ {0x11,0x11,0x0A,0x04,0x0A,0x11,0x11},
    /* 'Y' */ {0x11,0x11,0x0A,0x04,0x04,0x04,0x04},
    /* 'Z' */ {0x1F,0x01,0x02,0x04,0x08,0x10,0x1F},
};

/*
 * draw_char_scaled — desenha um caractere 'c' na posicao (px, py)
 * com escala 'scale' (pixels por bit da fonte).
 */
static void draw_char_scaled(int px, int py, char c, int scale, uint16_t color) {
    int row, col, sr, sc;
    if (c < 32 || c > 90) return;
    const uint8_t *glyph = FONT5X7[c - 32];
    for (row = 0; row < 7; row++) {
        for (col = 0; col < 5; col++) {
            if (glyph[row] & (0x10 >> col)) {
                for (sr = 0; sr < scale; sr++)
                    for (sc = 0; sc < scale; sc++)
                        plot_pixel(px + col*scale + sc,
                                   py + row*scale + sr,
                                   color);
            }
        }
    }
}

/* Escreve string com fonte escalada */
static void draw_str_scaled(int px, int py, const char *s, int scale, uint16_t color) {
    while (*s) {
        draw_char_scaled(px, py, *s, scale, color);
        px += (5 + 1) * scale;
        s++;
    }
}

/* ============================================================
 *  SECAO 4 — LOGICA DO TETRIS
 * ============================================================ */

/* Grid estatico: 0 = vazio, 1..7 = cor da peca (indice+1) */
static uint8_t grid[GRID_ROWS][GRID_COLS];

/* Peca atual */
static int cur_type;     /* 0..6  */
static int cur_rot;      /* 0..3  */
static int cur_x;        /* coluna (grid)   */
static int cur_y;        /* linha  (grid)   */

/* Proxima peca */
static int next_type;

/* Estatisticas */
static int score;
static int lines_total;
static int level;
static int high_score = 0;  /* melhor pontuacao (resetada ao desligar) */
static int new_record = 0;  /* 1 se o jogador bateu o recorde nesta partida */

/* Seed pseudo-aleatoria (alimentada pelo timer) */
static volatile uint32_t rng_seed = 12345;

/* Leitura do timer ARM (MPCORE Private Timer, livre-roda) */
static inline uint32_t get_timer(void) {
    return *(volatile uint32_t *)(MPCORE_PRIV_TIMER + 0x04); /* Current value */
}

/* Inicializa o timer ARM para auto-reload livre */
static void timer_init(void) {
    volatile uint32_t *load  = (volatile uint32_t *)(MPCORE_PRIV_TIMER + 0x00);
    volatile uint32_t *ctrl  = (volatile uint32_t *)(MPCORE_PRIV_TIMER + 0x08);
    *load = 0xFFFFFFFF;
    *ctrl = 0x00000006; /* auto-reload, enable, sem interrupcao */
}

/* Delay simples em iteracoes (calibrado empiricamente p/ ~200MHz) */
static void delay_ms(int ms) {
    volatile int i;
    for (i = 0; i < ms * 10000; i++);
}

/* XorShift32 — PRNG rapido, sem malloc/rand */
static uint32_t prng_next(void) {
    rng_seed ^= rng_seed << 13;
    rng_seed ^= rng_seed >> 17;
    rng_seed ^= rng_seed << 5;
    return rng_seed;
}

/* --- 7-Bag Randomizer --------------------------------------- */
/*
 * Garante que a cada 7 pecas, uma de cada tipo aparece exatamente
 * uma vez (ordem aleatoria). Implementa Fisher-Yates shuffle.
 */
static uint8_t bag[7];       /* saco atual com indices das pecas */
static int     bag_idx = 7;  /* proximo a retirar; 7 = saco vazio */

/* Preenche e embaralha o saco com as 7 pecas */
static void bag_refill(void) {
    int i, j;
    uint8_t tmp;
    /* Preenche em ordem */
    for (i = 0; i < 7; i++) bag[i] = (uint8_t)i;
    /* Fisher-Yates: embaralha usando PRNG */
    for (i = 6; i > 0; i--) {
        j = (int)(prng_next() % (uint32_t)(i + 1));
        tmp    = bag[i];
        bag[i] = bag[j];
        bag[j] = tmp;
    }
    bag_idx = 0;
}

/* Retira a proxima peca do saco; reabastece automaticamente */
static int next_bag_piece(void) {
    if (bag_idx >= 7) bag_refill();
    return (int)bag[bag_idx++];
}

/* --- Limpa o grid ------------------------------------------- */
static void grid_clear(void) {
    int r, c;
    for (r = 0; r < GRID_ROWS; r++)
        for (c = 0; c < GRID_COLS; c++)
            grid[r][c] = 0;
}

/* --- Verifica colisao da peca (tipo, rot) em posicao (gx, gy) */
static int piece_collides(int type, int rot, int gx, int gy) {
    int r, c;
    for (r = 0; r < 4; r++)
        for (c = 0; c < 4; c++)
            if (SHAPES[type][rot][r][c]) {
                int nx = gx + c;
                int ny = gy + r;
                if (nx < 0 || nx >= GRID_COLS) return 1;
                if (ny >= GRID_ROWS)            return 1;
                if (ny >= 0 && grid[ny][nx])    return 1;
            }
    return 0;
}

/* --- Fixa peca atual no grid -------------------------------- */
static void piece_lock(void) {
    int r, c;
    for (r = 0; r < 4; r++)
        for (c = 0; c < 4; c++)
            if (SHAPES[cur_type][cur_rot][r][c]) {
                int ny = cur_y + r;
                int nx = cur_x + c;
                if (ny >= 0 && ny < GRID_ROWS && nx >= 0 && nx < GRID_COLS)
                    grid[ny][nx] = (uint8_t)(cur_type + 1);
            }
}

/* --- Limpa linhas completas, retorna numero de linhas limpas - */
static int clear_lines(void) {
    int r, c, cleared = 0;
    for (r = GRID_ROWS - 1; r >= 0; ) {
        int full = 1;
        for (c = 0; c < GRID_COLS; c++)
            if (!grid[r][c]) { full = 0; break; }
        if (full) {
            /* Move todas as linhas acima uma posicao para baixo */
            int rr;
            for (rr = r; rr > 0; rr--)
                for (c = 0; c < GRID_COLS; c++)
                    grid[rr][c] = grid[rr-1][c];
            for (c = 0; c < GRID_COLS; c++)
                grid[0][c] = 0;
            cleared++;
            /* Nao incrementa r: re-checa a mesma linha */
        } else {
            r--;
        }
    }
    return cleared;
}

/* Tabela de pontuacao classica (pontos por linhas de uma vez) */
static const int LINE_POINTS[5] = { 0, 100, 300, 500, 800 };

/* --- Spawna nova peca no topo do grid ----------------------- */
/* Retorna 0 se Game Over (colisao imediata), 1 caso OK */
static int spawn_piece(void) {
    cur_type = next_type;
    cur_rot  = 0;
    cur_x    = (GRID_COLS / 2) - 2;
    cur_y    = 0;
    next_type = next_bag_piece();
    if (piece_collides(cur_type, cur_rot, cur_x, cur_y))
        return 0; /* game over */
    return 1;
}

/* ============================================================
 *  SECAO 5 — DESENHO DO JOGO
 * ============================================================ */

/* Desenha uma celula do grid na tela (uma celula = CELL_W x CELL_H px) */
static void draw_cell(int gx, int gy, uint16_t face, uint16_t border) {
    int px = GRID_X + gx * CELL_W;
    int py = GRID_Y + gy * CELL_H;
    /* Face interna */
    fill_rect(px + 1, py + 1, CELL_W - 2, CELL_H - 2, face);
    /* Borda escura (brilho 3D) */
    hline(px,          px + CELL_W - 1, py,          border);
    hline(px,          px + CELL_W - 1, py + CELL_H - 1, border);
    vline(px,          py,              py + CELL_H - 1, border);
    vline(px + CELL_W - 1, py,         py + CELL_H - 1, border);
}

/* Apaga uma celula do grid (pinta com cor de fundo) */
static void erase_cell(int gx, int gy) {
    int px = GRID_X + gx * CELL_W;
    int py = GRID_Y + gy * CELL_H;
    fill_rect(px, py, CELL_W, CELL_H, COL_GRID_BG);
}

/* Redesenha o grid estatico completo */
static void redraw_grid(void) {
    int r, c;
    for (r = 0; r < GRID_ROWS; r++)
        for (c = 0; c < GRID_COLS; c++) {
            if (grid[r][c]) {
                int t = grid[r][c] - 1;
                draw_cell(c, r, PIECE_COLORS[t], PIECE_DARK[t]);
            } else {
                erase_cell(c, r);
            }
        }
}

/* Apaga a peca atual da tela */
static void erase_piece(void) {
    int r, c;
    for (r = 0; r < 4; r++)
        for (c = 0; c < 4; c++)
            if (SHAPES[cur_type][cur_rot][r][c]) {
                int gy = cur_y + r;
                int gx = cur_x + c;
                if (gy >= 0 && gy < GRID_ROWS && gx >= 0 && gx < GRID_COLS)
                    erase_cell(gx, gy);
            }
}

/* Desenha a peca atual na tela */
static void draw_piece(void) {
    int r, c;
    uint16_t face   = PIECE_COLORS[cur_type];
    uint16_t border = PIECE_DARK[cur_type];
    for (r = 0; r < 4; r++)
        for (c = 0; c < 4; c++)
            if (SHAPES[cur_type][cur_rot][r][c]) {
                int gy = cur_y + r;
                int gx = cur_x + c;
                if (gy >= 0 && gy < GRID_ROWS && gx >= 0 && gx < GRID_COLS)
                    draw_cell(gx, gy, face, border);
            }
}

/*
 * draw_ghost_piece — mostra a "sombra" (onde a peca vai cair)
 * Desenha com cor escura/tracejada para auxiliar o jogador.
 */
static void draw_ghost_piece(void) {
    int ghost_y = cur_y;
    while (!piece_collides(cur_type, cur_rot, cur_x, ghost_y + 1))
        ghost_y++;
    if (ghost_y == cur_y) return; /* ja esta no lugar */

    int r, c;
    uint16_t ghost_col = rgb(40, 40, 60); /* cinza muito escuro */
    for (r = 0; r < 4; r++)
        for (c = 0; c < 4; c++)
            if (SHAPES[cur_type][cur_rot][r][c]) {
                int gy = ghost_y + r;
                int gx = cur_x + c;
                if (gy >= 0 && gy < GRID_ROWS && gx >= 0 && gx < GRID_COLS
                    && !grid[gy][gx]) {
                    /* Apenas o contorno da ghost */
                    int px = GRID_X + gx * CELL_W;
                    int py = GRID_Y + gy * CELL_H;
                    hline(px, px + CELL_W - 1, py,          ghost_col);
                    hline(px, px + CELL_W - 1, py + CELL_H - 1, ghost_col);
                    vline(px,          py, py + CELL_H - 1, ghost_col);
                    vline(px + CELL_W - 1, py, py + CELL_H - 1, ghost_col);
                }
            }
}

/*
 * draw_next_piece — desenha a proxima peca centralizada na caixa NEXT.
 * Calcula o bounding box real da peca (min/max col e linha usadas)
 * e centraliza dentro da area interior da caixa (55x50 px).
 */
static void draw_next_piece(void) {
    /* Interior da caixa: x=[NEXT_BOX_X+1 .. NEXT_BOX_X+53], w=53
     *                    y=[NEXT_BOX_Y+1 .. NEXT_BOX_Y+48], h=48  */
    fill_rect(NEXT_BOX_X + 1, NEXT_BOX_Y + 1, 53, 48, COL_PANEL);

    int r, c;
    uint16_t face   = PIECE_COLORS[next_type];
    uint16_t border = PIECE_DARK[next_type];
    int bsz = 10; /* tamanho da mini-celula em pixels */

    /* --- Calcula bounding box da peca (grade 4x4) ----------- */
    int min_c = 4, max_c = -1, min_r = 4, max_r = -1;
    for (r = 0; r < 4; r++)
        for (c = 0; c < 4; c++)
            if (SHAPES[next_type][0][r][c]) {
                if (c < min_c) min_c = c;
                if (c > max_c) max_c = c;
                if (r < min_r) min_r = r;
                if (r > max_r) max_r = r;
            }

    int piece_cols = max_c - min_c + 1; /* largura em celulas */
    int piece_rows = max_r - min_r + 1; /* altura  em celulas */

    /* Tamanho em pixels da peca */
    int pw = piece_cols * bsz;
    int ph = piece_rows * bsz;

    /* Centro da area interior da caixa */
    int box_cx = NEXT_BOX_X + 1 + 53 / 2;
    int box_cy = NEXT_BOX_Y + 1 + 48 / 2;

    /* Pixel superior-esquerdo para centralizar */
    int origin_x = box_cx - pw / 2;
    int origin_y = box_cy - ph / 2;

    /* --- Desenha cada celula ativa com offset do bounding box  */
    for (r = 0; r < 4; r++)
        for (c = 0; c < 4; c++)
            if (SHAPES[next_type][0][r][c]) {
                int px = origin_x + (c - min_c) * bsz;
                int py = origin_y + (r - min_r) * bsz;
                fill_rect(px + 1, py + 1, bsz - 2, bsz - 2, face);
                hline(px, px + bsz - 1, py,           border);
                hline(px, px + bsz - 1, py + bsz - 1, border);
                vline(px,           py, py + bsz - 1, border);
                vline(px + bsz - 1, py, py + bsz - 1, border);
            }
}

/*
 * draw_brick_bg — preenche regiao retangular com textura de tijolos
 * cinzas estilo NES Tetris.
 * Tijolo: 8x4 px. Linhas alternadas com offset de 4px (meio tijolo).
 */
static void draw_brick_bg(int x0, int y0, int w, int h) {
    uint16_t col_hi     = rgb(160, 160, 160);
    uint16_t col_face   = rgb(108, 108, 108);
    uint16_t col_sh     = rgb(68,  68,  68);
    uint16_t col_mortar = rgb(45,  45,  45);
    int x, y;
    for (y = y0; y < y0 + h; y++) {
        int row   = (y - y0) >> 2;
        int by    = (y - y0) & 3;
        int shift = (row & 1) << 2;
        for (x = x0; x < x0 + w; x++) {
            int bx = (x - x0 + shift) & 7;
            uint16_t col;
            if      (bx == 7 || by == 3) col = col_mortar;
            else if (by == 0 || bx == 0) col = col_hi;
            else if (by == 2 || bx == 6) col = col_sh;
            else                         col = col_face;
            plot_pixel(x, y, col);
        }
    }
}

/*
 * draw_hud — redesenha o HUD (score/level/next) no painel direito
 * Usa Character Buffer para textos, VGA para graficos.
 */
static void draw_hud(void) {
    int bx = 185, bw = 110, bh = 22;

    /* --- Titulo TETRIS (sem caixa, centrado) ---------------- */
    char_write_str(57, 1, "TETRIS");

    /* --- 1. SCORE ------------------------------------------- */
    fill_rect(bx + 1, 15, bw - 2, bh - 2, COL_DARK);
    draw_rect(bx, 14, bw, bh, COL_WHITE);
    char_write_str(58, 5, "SCORE");
    char_write_int_centered(60, 7, score);

    /* --- 2. HIGH SCORE -------------------------------------- */
    fill_rect(bx + 1, 41, bw - 2, bh - 2, COL_DARK);
    draw_rect(bx, 40, bw, bh, COL_WHITE);
    char_write_str(55, 11, "HIGH SCORE");
    char_write_int_centered(60, 13, high_score);

    /* --- 3. LEVEL ------------------------------------------- */
    fill_rect(bx + 1, 67, bw - 2, bh - 2, COL_DARK);
    draw_rect(bx, 66, bw, bh, COL_WHITE);
    char_write_str(58, 18, "LEVEL");
    char_write_int_centered(60, 20, level + 1);

    /* --- 4. LINES ------------------------------------------- */
    fill_rect(bx + 1, 93, bw - 2, bh - 2, COL_DARK);
    draw_rect(bx, 92, bw, bh, COL_WHITE);
    char_write_str(58, 24, "LINES");
    char_write_int_centered(60, 26, lines_total);

    /* --- NEXT ---------------------------------------------- */
    char_write_str(58, 32, "NEXT");
}

/* Desenha toda a interface do jogo (primeira vez) */
static void draw_game_ui(void) {
    /* Fundo geral: textura de tijolos cinzas (estilo NES Tetris) */
    draw_brick_bg(0, 0, SCREEN_W, SCREEN_H);

    /* Fundo do grid: preto */
    fill_rect(GRID_X, GRID_Y, GRID_PX_W, GRID_PX_H, COL_BLACK);

    /* Grade de linhas internas (muito sutis sobre fundo preto) */
    {
        int r, c;
        uint16_t grid_line_col = rgb(25, 25, 25);
        for (r = 1; r < GRID_ROWS; r++)
            hline(GRID_X, GRID_X + GRID_PX_W - 1,
                  GRID_Y + r * CELL_H, grid_line_col);
        for (c = 1; c < GRID_COLS; c++)
            vline(GRID_X + c * CELL_W, GRID_Y,
                  GRID_Y + GRID_PX_H - 1, grid_line_col);
    }

    /* Borda externa do grid (neon) */
    draw_rect(GRID_X - 2, GRID_Y - 2,
              GRID_PX_W + 4, GRID_PX_H + 4, COL_BORDER);
    draw_rect(GRID_X - 1, GRID_Y - 1,
              GRID_PX_W + 2, GRID_PX_H + 2, rgb(30, 100, 160));

    /* Divisor vertical entre grid e painel (neon) */
    vline(PANEL_X,     0, SCREEN_H - 1, COL_BORDER);
    vline(PANEL_X + 1, 0, SCREEN_H - 1, rgb(30, 100, 160));

    /* Caixa NEXT PIECE: fundo escuro + borda neon */
    fill_rect(NEXT_BOX_X + 1, NEXT_BOX_Y + 1, 53, 48, COL_DARK);
    draw_rect(NEXT_BOX_X, NEXT_BOX_Y, 55, 50, COL_BORDER);

    /* HUD texto */
    char_clear_all();
    draw_hud();
    draw_next_piece();
}

/* ============================================================
 *  SECAO 6 — LEITURA DOS BOTOES
 * ============================================================
 *
 *  KEY_BASE = 0xFF200050
 *  Os botoes sao ativos em LOW (0 = pressionado).
 *  Lemos a borda de descida via "not-pressed before, pressed now".
 */
static uint32_t key_prev = 0; /* estado anterior (todos soltos) */

/*
 * read_keys — retorna mascara de botoes a processar neste frame.
 *
 * KEY1 (0x2) e KEY2 (0x4): apenas borda de subida (dispara uma vez).
 * KEY0 (0x1) e KEY3 (0x8): DAS (Delayed Auto Shift) estilo NES:
 *   - Dispara imediatamente na borda de subida.
 *   - Aguarda DAS_DELAY frames antes de comecar a repetir.
 *   - Repete a cada DAS_REPEAT frames enquanto mantido.
 */

/* Parametros DAS (em frames; 1 frame ~= 12ms real no Cortex-A9) */
#define DAS_DELAY   12   /* ~144ms ate comecar a repetir */
#define DAS_REPEAT   4   /* ~48ms entre repeticoes       */

static int das_counter_k0 = 0;  /* contador DAS para KEY0 (direita) */
static int das_counter_k3 = 0;  /* contador DAS para KEY3 (esquerda)*/

static uint32_t read_keys(void) {
    volatile uint32_t *key_reg = (volatile uint32_t *) KEY_BASE;
    uint32_t pressed = *key_reg & 0xF;  /* 4 bits relevantes */
    uint32_t edge    = pressed & (~key_prev);  /* borda de subida */
    uint32_t result  = 0;

    /* --- KEY1 e KEY2: borda de subida apenas ------------------- */
    result |= edge & (0x2 | 0x4);

    /* --- KEY0 (direita): DAS ----------------------------------- */
    if (pressed & 0x1) {
        if (edge & 0x1) {
            /* Primeiro pressionamento: dispara e reinicia contador */
            result |= 0x1;
            das_counter_k0 = 0;
        } else {
            /* Mantido: incrementa e repete apos DAS_DELAY + repeticoes */
            das_counter_k0++;
            if (das_counter_k0 >= DAS_DELAY) {
                int rep = das_counter_k0 - DAS_DELAY;
                if (rep % DAS_REPEAT == 0)
                    result |= 0x1;
            }
        }
    } else {
        das_counter_k0 = 0;  /* solto: zera contador */
    }

    /* --- KEY3 (esquerda): DAS ---------------------------------- */
    if (pressed & 0x8) {
        if (edge & 0x8) {
            result |= 0x8;
            das_counter_k3 = 0;
        } else {
            das_counter_k3++;
            if (das_counter_k3 >= DAS_DELAY) {
                int rep = das_counter_k3 - DAS_DELAY;
                if (rep % DAS_REPEAT == 0)
                    result |= 0x8;
            }
        }
    } else {
        das_counter_k3 = 0;
    }

    key_prev = pressed;
    return result;
}

/* Versao simples: retorna 1 se QUALQUER botao for pressionado */
static int any_key_pressed(void) {
    volatile uint32_t *key_reg = (volatile uint32_t *) KEY_BASE;
    uint32_t pressed = *key_reg & 0xF;
    return (pressed != 0); /* algum bit = 1 */
}

/* ============================================================
 *  SECAO 7 — TELA DE MENU
 * ============================================================ */

static void draw_menu(void) {
    /* Fundo com gradiente vertical (roxo escuro -> preto) */
    {
        int y;
        for (y = 0; y < SCREEN_H; y++) {
            uint8_t v = (uint8_t)(80 - (y * 80) / SCREEN_H);
            uint16_t col = rgb((uint8_t)(v / 2), 0, v);
            hline(0, SCREEN_W - 1, y, col);
        }
    }

    /* Decoracao: linhas horizontais neon */
    hline(0, SCREEN_W-1, 5,          COL_BORDER);
    hline(0, SCREEN_W-1, SCREEN_H-6, COL_BORDER);

    /* Blocos coloridos decorativos (como peças de tetris) */
    fill_rect( 10, 15, 16, 16, PIECE_COLORS[0]);
    fill_rect( 30, 15, 16, 16, PIECE_COLORS[1]);
    fill_rect( 50, 15, 16, 16, PIECE_COLORS[2]);
    fill_rect( 70, 15, 16, 16, PIECE_COLORS[3]);
    fill_rect(234, 15, 16, 16, PIECE_COLORS[4]);
    fill_rect(254, 15, 16, 16, PIECE_COLORS[5]);
    fill_rect(274, 15, 16, 16, PIECE_COLORS[6]);
    fill_rect(294, 15, 16, 16, PIECE_COLORS[0]);

    /* Titulo "TETRIS" com sombra */
    /* "TETRIS" (6 letras) em escala 4 ocupa 144 pixels. Para centralizar em 320: X = 88 */
    draw_str_scaled(90, 62, "TETRIS", 4, COL_DARK); // sombra
    {
        const char *title = "TETRIS";
        uint16_t title_colors[6] = {
            PIECE_COLORS[0], PIECE_COLORS[3], PIECE_COLORS[2],
            PIECE_COLORS[4], PIECE_COLORS[5], PIECE_COLORS[6]
        };
        int i;
        for (i = 0; i < 6; i++)
            draw_char_scaled(88 + i * 24, 60, title[i], 4, title_colors[i]);
    }

    /* Subtitulo "ARCADE" */
    /* "ARCADE" (6 letras) em escala 2 ocupa 72 pixels. Centralizar: X = 124 */
    draw_str_scaled(124, 108, "ARCADE", 2, rgb(200, 200, 200));

    /* Decoracao: grade de mini-blocos no rodape */
    {
        int c, type = 0;
        for (c = 0; c < 20; c++) {
            fill_rect(c * 16, 195, 14, 14, PIECE_COLORS[type % 7]);
            type++;
        }
    }

    /* Limpa char buffer e escreve instrucoes */
    char_clear_all();
    /* Textos centralizados em 80 colunas */
    char_write_str(27, 35, "PRESS ANY BUTTON TO START");
    char_write_str(29, 40, "KEY3=LEFT  KEY0=RIGHT");
    char_write_str(29, 42, "KEY1=ROTATE  KEY2=DROP");
}

/* ============================================================
 *  SECAO 8 — TELA DE GAME OVER
 * ============================================================ */

static void draw_gameover(void) {
    /* Limpa a tela inteira (pixels) e o char buffer */
    fill_rect(0, 0, SCREEN_W, SCREEN_H, COL_BLACK);
    char_clear_all();

    /* Caixa principal — altura aumentada para caber high score e new record */
    fill_rect(10, 52, 300, 150, rgb(20, 0, 30));
    draw_rect(10, 52, 300, 150, COL_RED);
    draw_rect(12, 54, 296, 146, rgb(180, 0, 0));

    /* "GAME OVER" centralizado (escala 3: 9*18=162px; x=(320-162)/2=79) */
    draw_str_scaled(79, 62, "GAME OVER", 3, COL_RED);

    /* Pontuacao final */
    char_write_str(32, 28, "FINAL SCORE:");
    char_write_int(45, 28, score);

    /* Melhor pontuacao */
    char_write_str(32, 30, "HIGH  SCORE:");
    char_write_int(45, 30, high_score);

    /* Banner de novo recorde (so aparece se bateu o high score) */
    if (new_record) {
        /* Destaque em amarelo-ouro usando VGA bitmap (escala 2) */
        /* "NEW RECORD!!!" = 13 chars * 12px = 156px; x=(320-156)/2 = 82 */
        draw_str_scaled(82, 134, "NEW RECORD!!!", 2, COL_GOLD);
    }

    /* Instrucao para reiniciar */
    char_write_str(28, 39, "PRESS BUTTON TO RESTART");
}

/* ============================================================
 *  SECAO 9 — ANIMACAO DE LINHA LIMPA (flash)
 * ============================================================ */

static void flash_lines(int row_start[], int count) {
    int f, i, c;
    for (f = 0; f < 4; f++) {
        uint16_t col = (f % 2 == 0) ? COL_WHITE : COL_GOLD;
        for (i = 0; i < count; i++) {
            int r = row_start[i];
            for (c = 0; c < GRID_COLS; c++) {
                int px = GRID_X + c * CELL_W;
                int py = GRID_Y + r * CELL_H;
                fill_rect(px, py, CELL_W, CELL_H, col);
            }
        }
        delay_ms(60);
    }
}

/* ============================================================
 *  SECAO 10 — MAQUINA DE ESTADOS PRINCIPAL
 * ============================================================ */

int main(void) {
    int game_state = STATE_MENU;
    int fall_counter = 0;
    int blink_counter = 0;
    int blink_visible = 1;

    /* Inicializa timer ARM */
    timer_init();

    /* Seed do PRNG com valor do timer */
    rng_seed = get_timer() ^ 0xDEAD1234;
    if (rng_seed == 0) rng_seed = 0x12345678;

    /* Zera estado de botoes */
    key_prev = 0;

    /* --------------------------------------------------------
     *  LOOP PRINCIPAL
     * -------------------------------------------------------- */
    while (1) {

        /* Alimenta o PRNG com o timer a cada frame (entropia) */
        rng_seed ^= get_timer();
        if (rng_seed == 0) rng_seed = 1;

        /* ==================================================
         *  ESTADO: MENU
         * ================================================== */
        if (game_state == STATE_MENU) {

            draw_menu();

            /* Blink "PRESS ANY BUTTON": pisca no char buffer */
            while (1) {
                /* Aguarda botao, piscando o texto */
                blink_counter++;
                if (blink_counter > 20) {
                    blink_counter = 0;
                    blink_visible = !blink_visible;
                    if (blink_visible)
                        char_write_str(27, 35, "PRESS ANY BUTTON TO START");
                    else
                        char_write_str(27, 35, "                         ");
                }

                /* Alimenta PRNG continuamente (gera entropia com o tempo) */
                rng_seed ^= get_timer();
                if (rng_seed == 0) rng_seed = 1;

                delay_ms(8);

                if (any_key_pressed()) {
                    /* Aguarda soltar o botao */
                    while (any_key_pressed());
                    break;
                }
            }

            /* Inicializa jogo */
            grid_clear();
            score       = 0;
            lines_total = 0;
            level       = 0;
            fall_counter = 0;

            bag_idx   = 7;               /* reinicia o saco a cada partida */
            next_type = next_bag_piece();
            /* Spawn inicial */
            if (!spawn_piece()) {
                /* Nao deveria acontecer no inicio — reset */
                grid_clear();
                spawn_piece();
            }

            draw_game_ui();
            redraw_grid();
            draw_ghost_piece();
            draw_piece();

            key_prev = 0; /* limpa estado dos botoes */
            game_state = STATE_PLAY;
        }

        /* ==================================================
         *  ESTADO: JOGO
         * ================================================== */
        else if (game_state == STATE_PLAY) {

            uint32_t edge = read_keys();

            int moved = 0;

            /* --- KEY0: Mover Direita ----------------------- */
            if (edge & 0x1) {
                if (!piece_collides(cur_type, cur_rot, cur_x + 1, cur_y)) {
                    erase_piece();
                    /* Tambem apaga ghost anterior */
                    redraw_grid();
                    cur_x++;
                    moved = 1;
                }
            }

            /* --- KEY2: Hard Drop -------------------------- */
            if (edge & 0x4) {
                erase_piece();
                redraw_grid();
                while (!piece_collides(cur_type, cur_rot, cur_x, cur_y + 1))
                    cur_y++;
                /* Pontua o drop */
                score += 2 * (GRID_ROWS - cur_y);
                moved = 1;
                fall_counter = LEVEL_SPEED[level]; /* forca lock */
            }

            /* --- KEY1: Rotar ------------------------------ */
            if (edge & 0x2) {
                int new_rot = (cur_rot + 1) % 4;
                /* Wall-kick: tenta deslocar lateralmente se necessario */
                if (!piece_collides(cur_type, new_rot, cur_x, cur_y)) {
                    erase_piece();
                    redraw_grid();
                    cur_rot = new_rot;
                    moved = 1;
                } else if (!piece_collides(cur_type, new_rot, cur_x + 1, cur_y)) {
                    erase_piece();
                    redraw_grid();
                    cur_rot = new_rot;
                    cur_x++;
                    moved = 1;
                } else if (!piece_collides(cur_type, new_rot, cur_x - 1, cur_y)) {
                    erase_piece();
                    redraw_grid();
                    cur_rot = new_rot;
                    cur_x--;
                    moved = 1;
                }
                /* Se ainda colidir, nao rota */
            }

            /* --- KEY3: Mover Esquerda ---------------------- */
            if (edge & 0x8) {
                if (!piece_collides(cur_type, cur_rot, cur_x - 1, cur_y)) {
                    erase_piece();
                    redraw_grid();
                    cur_x--;
                    moved = 1;
                }
            }

            /* --- Queda automatica ------------------------- */
            fall_counter++;
            if (fall_counter >= LEVEL_SPEED[level]) {
                fall_counter = 0;
                if (!piece_collides(cur_type, cur_rot, cur_x, cur_y + 1)) {
                    erase_piece();
                    redraw_grid();
                    cur_y++;
                    moved = 1;
                } else {
                    /* Fixa peca */
                    piece_lock();

                    /* Verifica linhas completas */
                    int cleared_rows[4];
                    int cleared_count = 0;
                    {
                        int r;
                        for (r = 0; r < GRID_ROWS && cleared_count < 4; r++) {
                            int c, full = 1;
                            for (c = 0; c < GRID_COLS; c++)
                                if (!grid[r][c]) { full = 0; break; }
                            if (full)
                                cleared_rows[cleared_count++] = r;
                        }
                    }

                    if (cleared_count > 0) {
                        flash_lines(cleared_rows, cleared_count);
                        int n = clear_lines();
                        lines_total += n;
                        score += LINE_POINTS[n] * (level + 1);
                        /* Avanca nivel */
                        level = lines_total / LINES_PER_LEVEL;
                        if (level > 9) level = 9;
                    }

                    /* Spawn nova peca primeiro para atualizar cur_type e next_type */
                    int spawn_ok = spawn_piece();

                    /* Atualiza HUD e painel NEXT com o novo next_type */
                    char_clear_all();
                    draw_hud();
                    redraw_grid();
                    draw_next_piece();

                    if (!spawn_ok) {
                        /* GAME OVER: atualiza high score antes de desenhar */
                        if (score > high_score) {
                            high_score = score;
                            new_record = 1;
                        } else {
                            new_record = 0;
                        }
                        draw_gameover();
                        game_state = STATE_GAMEOVER;
                        goto next_frame;
                    }

                    /* Redesenha com nova peca */
                    redraw_grid();
                    draw_ghost_piece();
                    draw_piece();
                    goto next_frame;
                }
            }

            /* Redesenha ghost + peca atual se houve movimento */
            if (moved) {
                draw_ghost_piece();
                draw_piece();
            }
        }

        /* ==================================================
         *  ESTADO: GAME OVER
         * ================================================== */
        else if (game_state == STATE_GAMEOVER) {
            /* Aguarda qualquer botao para voltar ao menu */
            if (read_keys()) {
                game_state = STATE_MENU;
                /* Limpa tela para o menu */
                fill_rect(0, 0, SCREEN_W, SCREEN_H, COL_BLACK);
                char_clear_all();
            }
        }

        next_frame:
        /* Meio-termo: delay_ms(20) ~= 5ms real no Cortex-A9 200MHz */
        delay_ms(12);

    } /* while(1) */

    return 0;
}
