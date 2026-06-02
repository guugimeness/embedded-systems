#include <reg51.h>

/* ================= PINOS DE HARDWARE ================= */
#define GlcdDataBus P3

sbit RS  = P2^0;
sbit RW  = P2^1;
sbit EN  = P2^2;
sbit PSB = P2^3;
sbit RST = P2^5;

/* ================= VARIÁVEIS GLOBAIS ================= */
unsigned char car_x = 48;       
unsigned char enemy_x = 0;      
unsigned char enemy_y = 0;      
bit enemy_active = 0;

unsigned char crashes = 0;      
unsigned char kms = 0;          
unsigned char game_time = 0;    

unsigned char track_scroll = 0; 
unsigned char game_tick = 0;    
bit game_over = 0;

unsigned char btn_cooldown = 0; 
unsigned char blink_timer = 0;  
unsigned char crash_seq = 0;    

unsigned char kms_tick = 0;
unsigned char spawn_timer = 0;
unsigned char enemy_lane_cycle = 0;

/* ================= SPRITES E FONTES ================= */
code unsigned char player_car_bmp[32] = {
 0x00,0x00, 0x01,0x80, 0x03,0xC0, 0x32,0x4C, 0x3F,0xFC, 0x3F,0xFC, 0x33,0xCC, 0x03,0xC0, 0x03,0xC0, 0x33,0xCC, 0x33,0xCC, 0x3F,0xFC, 0x3F,0xFC, 0x33,0xCC, 0x01,0x80, 0x00,0x00
};

code unsigned char enemy_car_bmp[32] = {
 0x00,0x00, 0x01,0x80, 0x02,0x40, 0x33,0xCC, 0x3F,0xFC, 0x3E,0xFC, 0x33,0x4C, 0x02,0xC0, 0x03,0x40, 0x32,0xCC, 0x33,0x4C, 0x3E,0xFC, 0x3F,0x7C, 0x33,0xCC, 0x01,0x80, 0x00,0x00
};

code unsigned char crash_bmp[32] = {
    0x00, 0x00, 0x04, 0x20, 0x11, 0x88, 0x2A, 0x54,
    0x54, 0x2A, 0xA1, 0x85, 0x4B, 0xD2, 0x9F, 0xF9,
    0x9F, 0xF9, 0x4B, 0xD2, 0xA1, 0x85, 0x54, 0x2A,
    0x2A, 0x54, 0x11, 0x88, 0x04, 0x20, 0x00, 0x00
};

unsigned char code amb1_bmp[32] = {
    0x00,0x00, 0x00,0x00, 0x00,0x00, 0x00,0xC0, 0x00,0xC0, 0x7F,0xE0, 0x7F,0x20, 0x67,0x20, 0x43,0xE0, 0x43,0xFC, 0x67,0xFC, 0x7F,0xFC, 0x7F,0xFC, 0x30,0x18, 0x30,0x18, 0x00,0x00
};

code unsigned char amb2_bmp[32] = {
 0x00,0x00, 0x00,0x00, 0x00,0x00, 0x00,0x00, 0x00,0x00, 0x7F,0xE0, 0x7F,0x20, 0x67,0x20, 0x43,0xE0, 0x43,0xFC, 0x67,0xFC, 0x7F,0xFC, 0x7F,0xFC, 0x30,0x18, 0x30,0x18, 0x00,0x00
};

code unsigned char blank_bmp[32] = {0};

code unsigned char wall_bmp[32] = {
    0xE0, 0x00, 0xE0, 0x00, 0xE0, 0x00, 0xE0, 0x00,
    0xE0, 0x00, 0xE0, 0x00, 0xE0, 0x00, 0xE0, 0x00,
    0xE0, 0x00, 0xE0, 0x00, 0xE0, 0x00, 0xE0, 0x00,
    0xE0, 0x00, 0xE0, 0x00, 0xE0, 0x00, 0xE0, 0x00
};

code unsigned char diag_right_bmp[32] = {
    0x00,0x07, 0x00,0x07, 0x00,0x07, 0x00,0x0F, 0x00,0x1E, 0x00,0x3C, 0x00,0x78, 0x00,0xF0, 0x01,0xE0, 0x03,0xC0, 0x07,0x80, 0x0F,0x00, 0x1E,0x00, 0x7C,0x00, 0xF0,0x00, 0xE0,0x00
};

code unsigned char diag_left_bmp[32] = {
    0xE0,0x00, 0xE0,0x00, 0xE0,0x00, 0xF0,0x00, 0x78,0x00, 0x3C,0x00, 0x1E,0x00, 0x0F,0x00, 0x07,0x80, 0x03,0xC0, 0x01,0xE0, 0x00,0xF0, 0x00,0x78, 0x00,0x3E, 0x00,0x0F, 0x00,0x07
};

code unsigned char font[18][8] = {
    {0x3C,0x66,0x6E,0x76,0x66,0x66,0x3C,0x00}, // 0
    {0x18,0x38,0x18,0x18,0x18,0x18,0x7E,0x00}, // 1
    {0x3C,0x66,0x06,0x0C,0x30,0x60,0x7E,0x00}, // 2
    {0x3C,0x66,0x06,0x1C,0x06,0x66,0x3C,0x00}, // 3
    {0x0C,0x1C,0x2C,0x4C,0x7E,0x0C,0x0C,0x00}, // 4
    {0x7E,0x60,0x7C,0x06,0x06,0x66,0x3C,0x00}, // 5
    {0x3C,0x66,0x60,0x7C,0x66,0x66,0x3C,0x00}, // 6
    {0x7E,0x06,0x0C,0x18,0x30,0x60,0x60,0x00}, // 7
    {0x3C,0x66,0x66,0x3C,0x66,0x66,0x3C,0x00}, // 8
    {0x3C,0x66,0x66,0x3E,0x06,0x66,0x3C,0x00}, // 9
    {0x38,0x6C,0xC6,0xFE,0xC6,0xC6,0xC6,0x00}, // A
    {0xFE,0xC0,0xC0,0xFC,0xC0,0xC0,0xFE,0x00}, // E
    {0x7C,0xC6,0xC0,0xCE,0xC6,0xC6,0x7C,0x00}, // G
    {0xC6,0xEE,0xFE,0xD6,0xC6,0xC6,0xC6,0x00}, // M
    {0x7C,0xC6,0xC6,0xC6,0xC6,0xC6,0x7C,0x00}, // O
    {0xFC,0xC6,0xC6,0xFC,0xD8,0xCC,0xC6,0x00}, // R
    {0xC6,0xC6,0xC6,0xC6,0x6C,0x38,0x10,0x00}, // V
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}  // Espaço
};

code unsigned char track_map[32] = {
    1, 1, 1, 1, 1, 2, 2, 2, 
    2, 2, 2, 2, 2, 2, 1, 1, 
    0, 0, 0, 0, 1, 1, 2, 2, 
    1, 1, 0, 0, 0, 1, 1, 1
};

/* ================= PROTÓTIPOS ================= */
void glcdDelay(void);
void glcdBusyWait(void);
void glcdWriteCmd(unsigned char cmd);
void glcdWriteData(unsigned char dat);
void glcdInit(void);
void glcdClear(void);
void glcdSetAddr(unsigned char px, unsigned char py);
void draw16x16(unsigned char x, unsigned char y, unsigned char code *sprite);
void drawPanel(void);
void drawTrack(void);
void drawGameOver(void);
void scanButtons(void);
bit checkCollision(void);
void updateGame(void);
void delayMs(unsigned int ms);

/* ================= CONTROLE GLCD ================= */
void glcdDelay(void) {
    unsigned char i;
    for(i = 0; i < 5; i++);
}

void glcdBusyWait(void) {
    unsigned char st;
    unsigned int timeout = 0;
    GlcdDataBus = 0xFF;
    RS = 0; RW = 1;
    do {
        EN = 1; glcdDelay();
        st = GlcdDataBus;
        EN = 0; glcdDelay();
        timeout++;
        if(timeout > 500) break; 
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
    GlcdDataBus = dat;
    EN = 1; glcdDelay(); EN = 0; glcdDelay();
}

void glcdInit(void) {
    PSB = 1;
    RST = 0; delayMs(10);
    RST = 1; delayMs(10);
    glcdWriteCmd(0x30); glcdWriteCmd(0x30);
    glcdWriteCmd(0x0C); glcdWriteCmd(0x01);
    glcdWriteCmd(0x34); glcdWriteCmd(0x36);
}

void glcdClear(void) {
    unsigned char i, j;
    for(i = 0; i < 32; i++) {
        glcdWriteCmd(0x80 | i); 
        glcdWriteCmd(0x80 | 0); 
        for(j = 0; j < 16; j++) { 
            glcdWriteData(0x00);
            glcdWriteData(0x00);
        }
    }
}

void glcdSetAddr(unsigned char px, unsigned char py) {
    unsigned char gx = px >> 4;
    if (py >= 32) {
        py -= 32;
        gx += 8;
    }
    glcdWriteCmd(0x80 | py);
    glcdWriteCmd(0x80 | gx);
}

/* ================= RENDERIZAÇÃO ================= */
void draw16x16(unsigned char px, unsigned char py, unsigned char code *sprite) {
    unsigned char i;
    for(i = 0; i < 16; i++) {
        if(py + i >= 64) continue;
        glcdSetAddr(px, py + i);
        glcdWriteData(sprite[i * 2]);
        glcdWriteData(sprite[i * 2 + 1]);
    }
}

void drawTrack(void) {
    unsigned char y_block, map_idx, left_border, top_border;
    
    for(y_block = 0; y_block < 4; y_block++) {
        map_idx = (track_scroll + (3 - y_block)) & 31;
        left_border = track_map[map_idx] << 4;
        top_border = track_map[(track_scroll + (3 - y_block) + 1) & 31] << 4;
        
        draw16x16(0, y_block * 16, blank_bmp);
        draw16x16(16, y_block * 16, blank_bmp);
        draw16x16(32, y_block * 16, blank_bmp);
        draw16x16(48, y_block * 16, blank_bmp);
        draw16x16(64, y_block * 16, blank_bmp);
        draw16x16(80, y_block * 16, blank_bmp); 
        draw16x16(96, y_block * 16, blank_bmp); 

        if (left_border == top_border) {
            draw16x16(left_border, y_block * 16, wall_bmp);
            draw16x16(left_border + 64, y_block * 16, wall_bmp);
        } else if (top_border > left_border) {
            draw16x16(left_border, y_block * 16, diag_right_bmp);
            draw16x16(left_border + 64, y_block * 16, diag_right_bmp);
        } else {
            draw16x16(top_border, y_block * 16, diag_left_bmp);
            draw16x16(top_border + 64, y_block * 16, diag_left_bmp);
        }
    }
}

void drawPanel(void) {
    unsigned char i, j, temp, cen, dez, y;
    unsigned char vals[3];
    
    vals[0] = game_time;
    vals[1] = kms;
    vals[2] = crashes;

    for(i = 0; i < 3; i++) {
        temp = vals[i];
        cen = 0; while (temp >= 100) { cen++; temp -= 100; }
        dez = 0; while (temp >= 10)  { dez++; temp -= 10;  }
        
        y = i * 10;
        
        for(j = 0; j < 8; j++) {
            glcdSetAddr(96, y + j);
            glcdWriteData(0x00);
            glcdWriteData(font[cen][j]);
            
            glcdSetAddr(112, y + j);
            glcdWriteData(font[dez][j]);
            glcdWriteData(font[temp][j]); 
        }
    }
}

void drawGameOver(void) {
    unsigned char i;
    code unsigned char text1[] = {12, 10, 13, 11}; 
    code unsigned char text2[] = {14, 16, 11, 15}; 
    
    for(i = 0; i < 2; i++) {
        draw16x16(32 + (i * 16), 16, blank_bmp);
        draw16x16(32 + (i * 16), 32, blank_bmp);
    }
    
    for(i = 0; i < 8; i++) {
        glcdSetAddr(32, 20 + i);
        glcdWriteData(font[text1[0]][i]);
        glcdWriteData(font[text1[1]][i]);
        
        glcdSetAddr(48, 20 + i);
        glcdWriteData(font[text1[2]][i]);
        glcdWriteData(font[text1[3]][i]);
        
        glcdSetAddr(32, 36 + i);
        glcdWriteData(font[text2[0]][i]);
        glcdWriteData(font[text2[1]][i]);
        
        glcdSetAddr(48, 36 + i);
        glcdWriteData(font[text2[2]][i]);
        glcdWriteData(font[text2[3]][i]);
    }
}

/* ================= LÓGICA DO JOGO ================= */
void scanButtons(void) {
    P0 = 0xFF; 
    P0 = 0xFD; 
    glcdDelay();

    if (btn_cooldown > 0) {
        btn_cooldown--;
        return;
    }

    if (!(P0 & 0x40)) { 
        if (car_x > 16) car_x -= 16; 
        btn_cooldown = 4;
    }
    else if (!(P0 & 0x10)) { 
        if (car_x < 80) car_x += 16; 
        btn_cooldown = 4;
    }
}

bit checkCollision(void) {
    unsigned char map_idx = track_scroll & 31; 
    unsigned char left_border = track_map[map_idx] << 4; 
    
    if (car_x <= left_border || car_x >= left_border + 64) {
        return 1; 
    }
    
    if (enemy_active) {
        if (enemy_y > 32 && enemy_y < 64) {
            unsigned char e_y_block = enemy_y >> 4;
            unsigned char e_map_idx = (track_scroll + (3 - e_y_block)) & 31;
            unsigned char e_left = track_map[e_map_idx] << 4;
            
            if (car_x == (e_left + enemy_x)) return 1; 
        }
    }
    return 0;
}

void updateGame(void) {
    track_scroll++;
    if (track_scroll >= 32) track_scroll = 0;
    
    kms++; 
    kms_tick++;
    if (kms_tick >= 10) { 
        kms_tick = 0; 
        game_time++;
    }
    
    if (enemy_active) {
        enemy_y += 8;
        if (enemy_y >= 64) enemy_active = 0;
    } else {
        spawn_timer++;
        if (kms > 5 && spawn_timer >= 7) { 
            spawn_timer = 0;
            enemy_active = 1;
            enemy_y = 0;
            
            enemy_lane_cycle++;
            if (enemy_lane_cycle > 2) enemy_lane_cycle = 0;
            
            enemy_x = 16 + (enemy_lane_cycle << 4); 
        }
    }
}

void delayMs(unsigned int ms) {
    unsigned int i;
    unsigned char j;
    for(i = 0; i < ms; i++)
        for(j = 0; j < 125; j++);
}

/* ================= LOOP PRINCIPAL ================= */
void main(void) {
    glcdInit();
    glcdClear();

    while(1) {
        if (game_over) {
            drawGameOver(); 

            if (crash_seq < 15) {
                draw16x16(car_x, 48, crash_bmp);
                crash_seq++;
            } 
            else if (crash_seq < 31) {
                unsigned char step = (crash_seq - 15) >> 1; 
                unsigned char amb_x = step << 4; 
                
                if (amb_x > car_x) amb_x = car_x;
                
                draw16x16(car_x, 48, crash_bmp); 
                
                if (amb_x >= 16) {
                    draw16x16(amb_x - 16, 48, blank_bmp);
                }
                draw16x16(amb_x, 48, amb1_bmp);
                
                if (amb_x >= car_x) crash_seq = 31;
                else crash_seq++;
            } 
            else {
                blink_timer++;
                if (blink_timer < 5) draw16x16(car_x, 48, amb1_bmp);
                else if (blink_timer < 10) draw16x16(car_x, 48, amb2_bmp);
                else blink_timer = 0;
            }
            
            P0 = 0xFF;
            P0 = 0xFD; 
            glcdDelay();
            
            if (!(P0 & 0x20)) { 
                crashes++;
                if(crashes > 99) crashes = 0;
                
                car_x = 48; 
                enemy_active = 0;
                game_time = 0;
                kms = 0;
                track_scroll = 0;
                game_over = 0;
                btn_cooldown = 0;
                
                blink_timer = 0;
                crash_seq = 0;
                kms_tick = 0;
                spawn_timer = 0;
                enemy_lane_cycle = 0;
                
                glcdClear();
                delayMs(200);
            }
            delayMs(30); 
            continue;
        }

        scanButtons();

        game_tick++;
        if (game_tick >= 4) {
            game_tick = 0;

            drawTrack();
            draw16x16(car_x, 48, player_car_bmp);
            
            if (enemy_active) {
                unsigned char e_y_block = enemy_y >> 4;
                unsigned char e_map_idx = (track_scroll + (3 - e_y_block)) & 31;
                unsigned char e_left = track_map[e_map_idx] << 4;
                
                draw16x16(e_left + enemy_x, enemy_y, enemy_car_bmp);
            }
            
            drawPanel();
            updateGame();

            if (checkCollision()) {
                game_over = 1;
            }
        }

        delayMs(20);
    }
}