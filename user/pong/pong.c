#include "api.h"
#include <string.h>
#include <math.h>

#ifndef TEX_W
#define TEX_W 400
#endif

#ifndef TEX_H
#define TEX_H 240
#endif

static uint8_t buf[TEX_H][TEX_W][3];

static uint32_t T;
static uint32_t b0, b1;
#define btn(__b)    (!!(b0 & (__b)))
#define btnp(__b)   ((b0 & (__b)) && !(b1 & (__b)))

////// Util tools ///////
static inline void pix(uint16_t x, uint16_t y, uint8_t r, uint8_t g, uint8_t b) {
     buf[y][x][0] = r;
     buf[y][x][1] = g; 
     buf[y][x][2] = b;
}

static inline void pix_alpha(uint16_t x, uint16_t y, uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
    buf[y][x][2] += (((int16_t)r - buf[y][x][2]) * a) >> 8;
    buf[y][x][1] += (((int16_t)g - buf[y][x][1]) * a) >> 8;
    buf[y][x][0] += (((int16_t)b - buf[y][x][0]) * a) >> 8;
}

static inline void rect(int16_t x, int16_t y, uint16_t w, uint16_t h, uint8_t r, uint8_t g, uint8_t b) {
     int x0, y0;
     for (int w0 = 0; w0 < w; w0++) if ((x0 = x + w0) >= 0 && x0 < TEX_W)
     for (int h0 = 0; h0 < h; h0++) if ((y0 = y + h0) >= 0 && y0 < TEX_H)
	  pix(x0, y0, r, g, b);
     
}

// use center coordinate and half of the real scale to draw a rectangle
static inline void rect_cen(int16_t x, int16_t y, uint8_t half_w, uint8_t half_h, uint8_t r, uint8_t g, uint8_t b) {
     rect(x - half_w, y - half_h, (half_w << 1) + 1, (half_h << 1) + 1, r, g, b);
}

static inline void linev(int16_t x, int16_t y1, int16_t y2, uint8_t r, uint8_t g, uint8_t b, uint8_t d) {
     rect(x, y1, d, y2 - y1 + 1, r, g, b);
}

static inline void lineh(int16_t x1, int16_t x2, int16_t y, uint8_t r, uint8_t g, uint8_t b, uint8_t d) {
     rect(x1, y, x2 - x1 + 1, d, r, g, b);
}

static inline void dashlinev(int16_t x, int16_t y1, int16_t y2, uint8_t r, uint8_t g, uint8_t b, uint8_t d, uint8_t l, uint8_t s) {
     uint16_t y_next;
     while ((y_next = y1 + l + s) <= y2) {
	  rect(x, y1, d, l, r, g, b);
	  y1 = y_next;
     }
     linev(x, y1, y2, r, g, b, d);
}

static inline void dashlineh(int16_t x1, int16_t x2, int16_t y, uint8_t r, uint8_t g, uint8_t b, uint8_t d, uint8_t l, uint8_t s) {
     uint16_t x_next;
     while ((x_next = x1 + l + s) <= x2) {
	  rect(x1, y, l, d, r, g, b);
	  x1 = x_next;
     }
     lineh(x1, x2, y, r, g, b, d);
}

static inline void rectb(int16_t x, int16_t y, uint16_t w, uint16_t h, uint8_t r, uint8_t g, uint8_t b, int8_t d) {
     lineh(x, x + w - 1, y, r, g, b, d);
     lineh(x, x + w - 1, y + h - d, r, g, b, d);
     linev(x, y + d, y + h - d - 1, r, g, b, d);
     linev(x + w - d, y + d, y + h - d - 1, r, g, b, d);
}

static inline void rectdb(int16_t x, int16_t y, uint16_t w, uint16_t h, uint8_t r, uint8_t g, uint8_t b, int8_t d, uint8_t l, uint8_t s) {
     dashlineh(x, x + w - 1, y, r, g, b, d, l, s);
     dashlineh(x, x + w - 1, y + h - d, r, g, b, d, l, s);
     dashlinev(x, y + d, y + h - d - 1, r, g, b, d, l, s);
     dashlinev(x + w - d, y + d, y + h - d - 1, r, g, b, d, l, s);
}

static inline void cls() {
     for (int x = 0; x < TEX_W; x++)
	  for (int y = 0; y < TEX_H; y++)
	       pix(x, y, 0, 0, 0);
}

#define CHAR_W 7
#define CHAR_H 14

static uint8_t font_data[CHAR_W * CHAR_H * 16 * 6];

static inline void text_char(uint16_t x, uint16_t y, signed char ch)
{
     if (ch < 32) return;
     uint8_t row = (ch - 32) / 16;
     uint8_t col = (ch - 32) % 16;
     uint16_t ptr = row * CHAR_W * 16 * CHAR_H + col * CHAR_W;
     for (uint8_t j = 0; j < CHAR_H; j++)
     for (uint8_t i = 0; i < CHAR_W; i++)
         if (font_data[ptr + (j * CHAR_W * 16 + i)]) {
	     pix(x + i, y + j, 255, 255, 255);
	     pix(x + i + 1, y + j + 1, 108, 108, 108);
	 }
}

static inline void text_str(uint16_t x, uint16_t y, const char *str)
{
     while (*str != 0) {
	  text_char(x, y, *(str++));
	  x += CHAR_W;
     }
}

static inline void text_xcen(uint16_t x, uint16_t y, const char *str)
{
     size_t n = strlen(str);
     if (n > 0) text_str(x - (n * CHAR_W / 2), y, str);
}	

static inline uint32_t mrand() {
     static uint32_t seed = 20191117;
     return (seed = ((seed * 1103515245) + 12345) & 0x7fffffff);
}


// status define
#define SCR_GAME    (1 << 0)
#define SCR_MENU    (1 << 1)
#define OVL_WIN     (1 << 2)
#define STA_RUN     (1 << 3)
#define MOD_PVP     (1 << 4)

static uint8_t status;

static uint8_t score[2];

#define PLAYGROUND_FRAME 20
#define LINE_WIDTH        3

const char *msg;

static inline void playground_draw() {
     int d = LINE_WIDTH;
     int l = 5;
     int s = 3;
     rectb(PLAYGROUND_FRAME, PLAYGROUND_FRAME, TEX_W - 2 * PLAYGROUND_FRAME, TEX_H - 2 * PLAYGROUND_FRAME, 255, 255, 255, d);
     dashlinev((TEX_W >> 1) - (d >> 1), PLAYGROUND_FRAME + d, TEX_H - PLAYGROUND_FRAME - d - 1, 255, 255, 255, d, l, s);
     char c[10] = { 0 };
     c[0] = '0' + score[0];
     for (int i = 1; i < 8; i++) c[i] = ' ';
     c[8] = '0' + score[1];
     text_xcen(TEX_W >> 1, PLAYGROUND_FRAME << 2, c);
     text_xcen(TEX_W >> 1, TEX_H - PLAYGROUND_FRAME , msg);
     
}


static struct pongball {
     float x, y;
     uint8_t r, g, b;
     float vx, vy;
     uint8_t radius;
} ball; 

#define LEFT_SERVER     -1
#define RIGHT_SERVER     1
#define MATCH_POINT      5

static inline void ball_init(int8_t server);

static inline void goal(int8_t server) {
     status &= ~STA_RUN;
     if (server == LEFT_SERVER) {
	  msg = "Left goal!";
	  score[0]++;
	  if (score[0] == MATCH_POINT) {
	       status &= ~SCR_GAME;
	       status &= ~OVL_WIN;
	  }
     } else if (server == RIGHT_SERVER) {
	  msg = "Right goal!";
	  score[1]++;
	  if (score[1] == MATCH_POINT) {
	       status &= ~SCR_GAME;
	       status |= OVL_WIN;

	  }
     }
     ball_init(server);
}


static struct board {
     float x, y;
     uint8_t half_h, half_w;
     uint8_t r, g, b;
     float v;
} boards[2];

#define BOARD_V_MAX 3

#ifndef M_PI
#define M_PI  3.1415926535897932384626433832795
#endif
#define BALL_RADIUS 5
#define BALL_V      5
#define BALL_ANG    1.0
#include <stdio.h>
static inline void ball_init(int8_t server) {
     ball.x = TEX_W >> 1;
     ball.y = TEX_H >> 1;
     
     float angle = ((float)mrand() / 0x80000000u - 0.5)  * BALL_ANG;

     ball.vx = cos(angle) * server * BALL_V;
     ball.vy = sin(angle) * BALL_V;
     ball.r = 255; ball.g = 255; ball.b = 255;
     ball.radius = BALL_RADIUS;
}

static inline void ball_update() {
     ball.x += ball.vx;
     ball.y += ball.vy;

     if (ball.y <= (PLAYGROUND_FRAME + LINE_WIDTH + ball.radius)) {
	  ball.y  = ((PLAYGROUND_FRAME + LINE_WIDTH + ball.radius) << 1) - ball.y;
	  ball.vy = -1 * ball.vy;
     } else if (ball.y >= (TEX_H - PLAYGROUND_FRAME - LINE_WIDTH - (int)ball.radius - 1)) {
	  ball.y  = ((TEX_H - PLAYGROUND_FRAME - LINE_WIDTH - (int)ball.radius - 1) << 1) - ball.y;
	  ball.vy = -1 * ball.vy;
     }

     float angle;
     float delta_y;
     if (ball.x < (boards[0].x + boards[0].half_w + ball.radius + 1)) {
	  if (((delta_y = ball.y - boards[0].y) > boards[0].half_h) || (delta_y < -boards[0].half_h))
	       goal(RIGHT_SERVER);
	  else {
	       angle = sinf((delta_y / boards[0].half_h) * BALL_ANG) * BALL_ANG;
	       ball.vy = sinf(angle)  * BALL_V;
	       ball.vx = cosf(angle) * BALL_V;
	       ball.x = 2 * (boards[0].x + boards[0].half_w + ball.radius + 1) - ball.x;
	  }
     }
     else if (ball.x > (boards[1].x - boards[1].half_w - ball.radius - 1)) {
	  if (((delta_y = ball.y - boards[1].y) > boards[1].half_h) || (delta_y < -boards[1].half_h))
	       goal(LEFT_SERVER);
	  else {
	       angle = (delta_y / boards[1].half_h) * BALL_ANG;
	       ball.vy = sinf(angle) * BALL_V;
	       ball.vx = -cosf(angle) * BALL_V;
	       ball.x = 2 * (boards[1].x - boards[1].half_w - ball.radius - 1) - ball.x;
	  }
     }
}



static inline void ball_draw() {
     int x1 = (int)(ball.x + 0.5f);
     int y1 = (int)(ball.y + 0.5f);
     int x0, y0;
     for (int xi = -ball.radius; xi <= ball.radius; xi++) if ((x0 = x1 +xi) >= 0 && x0 < TEX_W) 
     for (int yi = -ball.radius; yi <= ball.radius; yi++) if ((y0 = y1 +yi) >= 0 && y0 < TEX_H) 
         if (xi * xi + yi * yi <= ball.radius * ball.radius + 1)
	      pix(x0, y0, ball.r, ball.g, ball.b);
}


static inline void board_init() {
     boards[0].y = TEX_H >> 1;            boards[1].y = TEX_H >> 1;
     boards[0].x = PLAYGROUND_FRAME << 1; boards[1].x = TEX_W - (PLAYGROUND_FRAME << 1);
     boards[0].half_h = (TEX_H * 3) >> 5; boards[1].half_h = (TEX_H * 3) >> 5;
     boards[0].half_w = LINE_WIDTH << 1;  boards[1].half_w = LINE_WIDTH << 1;
     boards[0].r = 220;                   boards[1].r = 220;
     boards[0].g = 220;                   boards[1].g = 220;
     boards[0].b = 220;                   boards[1].b = 220;
}
     
static inline void board_update() {
     float v1[2] = { 0, 0 };
     if (status & MOD_PVP) {
	  if (btn(BUTTON_TRI)) v1[0] -= 1;
	  if (btn(BUTTON_CRO)) v1[0] += 1;
     } else {
	  if (ball.y < boards[0].y - (boards[0].half_h >> 2)) v1[0] -= 1;
	  if (ball.y > boards[0].y + (boards[0].half_h >> 2)) v1[0] += 1;
     }
     if (btn(BUTTON_UP)) v1[1] -= 1;
     if (btn(BUTTON_DOWN)) v1[1] += 1;
     
     float delta[2] = { v1[0] - boards[0].v, v1[1] - boards[1].v };
     float rate = 0.25;
     boards[0].v += delta[0] * rate;
     boards[1].v += delta[1] * rate;
     
     boards[0].y = boards[0].y + boards[0].v * BOARD_V_MAX;

     boards[1].y = boards[1].y + boards[1].v * BOARD_V_MAX;

     if (boards[0].y < (PLAYGROUND_FRAME + LINE_WIDTH + boards[0].half_h + BALL_RADIUS)) {
	  boards[0].y = PLAYGROUND_FRAME + LINE_WIDTH + boards[0].half_h + BALL_RADIUS;
	  boards[0].v = 0;
     }
     if (boards[1].y < (PLAYGROUND_FRAME + LINE_WIDTH + boards[1].half_h + BALL_RADIUS)) {
	  boards[1].y = PLAYGROUND_FRAME + LINE_WIDTH + boards[1].half_h + BALL_RADIUS;
	  boards[1].v = 0;
     }
     if (boards[0].y > (TEX_H - PLAYGROUND_FRAME - LINE_WIDTH - boards[0].half_h - 1 - BALL_RADIUS)) {
	  boards[0].y = TEX_H - PLAYGROUND_FRAME - LINE_WIDTH - boards[0].half_h -1 - BALL_RADIUS;
	  boards[0].v = 0;
     }
     if (boards[1].y > (TEX_H - PLAYGROUND_FRAME - LINE_WIDTH - boards[1].half_h - 1 - BALL_RADIUS)) {
	  boards[1].y = TEX_H - PLAYGROUND_FRAME - LINE_WIDTH - boards[1].half_h -1 - BALL_RADIUS;
	  boards[1].v = 0;
     }
	  
}

static inline void board_draw() {
     rect_cen((int)(boards[0].x + 0.5f), (int)(boards[0].y + 0.5f), boards[0].half_w, boards[0].half_h, boards[0].r, boards[0].g, boards[0].b);
     rect_cen((int)(boards[1].x + 0.5f), (int)(boards[1].y + 0.5f), boards[1].half_w, boards[1].half_h, boards[1].r, boards[1].g, boards[1].b);
}

////// MENU //////

static uint8_t menu_sel = 0;
static uint8_t last_menu_sel = 0;
static uint32_t menu_sel_time = 0;
#define MENU_TRANSITION_DUR 15

static void menu_update()
{
    T++;

    uint8_t m0 = menu_sel;

    if (btnp(BUTTON_DOWN)) menu_sel = (menu_sel + 1) % 2;
    if (btnp(BUTTON_UP)) menu_sel = (menu_sel + 1) % 2;
    if (btnp(BUTTON_CIR)) {

	 status |= SCR_GAME;
	 status &= ~(SCR_MENU);
	 status |= STA_RUN;
	 if (menu_sel == 0) status &= ~(MOD_PVP);
	 else if (menu_sel == 1) status |= MOD_PVP;

    }

    if (menu_sel != m0) {
        last_menu_sel = m0;
        menu_sel_time = T;
    }
}

static void menu_draw() {
     cls();
     text_xcen(200, 60, "= P O N G =");
     text_xcen(200, 120, "P .vs. E");
     text_xcen(200, 120 + 24, "P .vs. P");

     int16_t x = sinf((float)T * 0.06f) * 1.9;
     uint16_t y = menu_sel * 24;
     if (menu_sel_time + MENU_TRANSITION_DUR >= T) {
	  float rate = expf((float)(T - menu_sel_time) / MENU_TRANSITION_DUR * -4);
	  y -= (menu_sel - last_menu_sel) * rate * 24;
    }

    text_char(144 - CHAR_W / 2 + x, 123 + y, '~');
    text_char(256 - CHAR_W / 2 - x, 123 + y, '~');
}


// TODO
void init() {
     msg = "";

     score[0] = 0; score[1] = 0;
     board_init();
     status = SCR_MENU;
     ball_init(RIGHT_SERVER);
}



static inline void gameover_update() {
     cls();
     msg = (status & MOD_PVP) ? ((status & OVL_WIN) ? "RIGHT WIN!" : "LEFT WIN!") : ((status & OVL_WIN) ? "YOU WIN!" : "YOU LOSE!");
     if (b0) init();
}

static inline void game_update() {
     if (status & STA_RUN) {
	  board_update();
	  ball_update();
     } else {
	  board_update();
	  if (btn(BUTTON_CIR)) {
	       status |= STA_RUN;
	       msg = "";
	  }
     }
}

static inline void game_draw() {
     cls();
     playground_draw();
     board_draw();
     ball_draw();

}

static inline void gameover_draw() {
     cls();
     text_xcen(TEX_W >> 1, (TEX_H >> 1) - (CHAR_H >> 1), msg);
}

void update() {
     b1 = b0;
     b0 = buttons();
     if (status & SCR_MENU) menu_update();
     else if (status & SCR_GAME) game_update();
     else gameover_update();
}

void *draw() {
     if (status & SCR_MENU) menu_draw();
     else if (status & SCR_GAME) game_draw();
     else gameover_draw();

     return (void *)buf;
}


// print('\n'.join(map(lambda x: ''.join(map(chr, range(32 + 16 * x, 48 + 16 * x))), range(6))))
// ffmpeg -f rawvideo -pix_fmt gray - -i tamzen14.png | hexdump -ve '1/1 "%.2x"' | fold -w96 | sed -e 's/00/0,/g' | sed -e 's/ff/1,/g'

static uint8_t font_data[CHAR_W * CHAR_H * 16 * 6] = {
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,
     1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,
     0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,
     0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,1,0,1,0,0,0,0,1,0,1,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,0,1,0,1,0,0,
     0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,1,0,1,0,0,0,0,1,0,1,0,0,0,0,1,1,
     1,1,0,1,0,1,0,0,1,0,0,1,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,
     1,0,1,0,0,0,1,1,1,1,1,0,0,1,0,0,0,0,0,1,0,1,0,1,0,0,0,1,0,1,0,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,
     0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,
     0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,1,0,1,0,0,0,1,0,0,0,0,0,0,1,0,1,0,0,0,0,0,1,0,0,1,
     0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,1,0,1,0,1,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
     0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,1,0,0,0,0,1,1,
     1,0,0,0,0,1,0,1,0,0,0,1,0,1,0,1,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,0,1,1,1,0,0,0,1,1,
     1,1,1,0,0,0,0,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
     0,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,1,0,0,1,0,1,0,1,0,0,1,0,0,1,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,
     0,0,0,1,0,0,0,1,0,1,0,1,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,
     0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,1,0,1,0,0,0,0,0,0,0,1,0,1,0,0,1,0,1,0,0,1,0,0,1,1,
     0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,
     0,0,0,0,1,1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,1,0,1,0,0,0,1,1,1,
     1,0,0,0,0,0,0,1,0,0,0,0,1,1,0,0,1,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,
     0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,
     0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
     0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
     0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,1,1,1,0,0,0,0,0,1,0,0,0,0,0,
     1,1,1,0,0,0,1,1,1,1,1,0,0,0,0,0,1,0,0,0,1,1,1,1,1,0,0,0,0,1,1,0,0,0,1,1,1,1,1,0,0,0,1,1,1,0,0,0,
     0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,1,0,
     0,1,0,0,0,1,0,0,0,1,1,0,0,0,0,1,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,1,1,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,
     0,0,0,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,0,1,1,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,
     0,0,0,1,0,0,0,0,0,0,0,0,0,0,1,0,0,1,0,0,1,1,0,0,1,0,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0,
     1,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,0,1,1,0,0,0,0,0,1,
     1,0,0,0,0,0,0,0,1,0,0,0,1,1,1,1,1,0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,1,0,1,0,1,0,0,0,0,1,0,0,0,0,0,
     0,0,1,0,0,0,0,0,1,1,0,0,0,1,0,0,1,0,0,0,1,1,1,1,0,0,0,1,1,1,1,0,0,0,0,0,0,1,0,0,0,0,1,1,1,0,0,0,
     1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,
     0,1,1,0,0,1,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,1,1,1,1,1,0,0,0,0,0,0,1,0,0,1,0,0,0,1,
     0,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,
     0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,
     1,0,0,0,0,0,0,0,1,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,
     0,0,0,0,0,0,0,1,0,0,0,0,1,1,1,1,1,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,0,1,
     0,0,0,0,0,0,1,0,0,0,1,0,0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,0,1,0,0,0,1,0,0,
     0,0,0,1,0,0,0,0,1,1,0,0,0,0,0,1,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,
     0,0,1,1,1,0,0,0,1,1,1,1,1,0,0,1,1,1,1,1,0,0,0,1,1,1,0,0,0,0,0,0,1,0,0,0,0,1,1,1,0,0,0,0,1,1,1,0,
     0,0,0,1,0,0,0,0,0,0,1,1,1,0,0,0,0,1,1,0,0,0,0,0,1,1,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,
     0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
     1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,1,0,0,0,0,1,1,1,1,0,0,0,0,0,1,1,1,0,0,1,1,1,
     1,0,0,0,1,1,1,1,1,0,0,1,1,1,1,1,0,0,0,0,1,1,1,0,0,1,0,0,0,1,0,0,1,1,1,1,1,0,0,0,0,0,0,1,0,0,1,0,
     0,0,1,0,0,1,0,0,0,0,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,0,1,1,1,0,0,0,1,0,0,0,1,0,0,0,1,0,1,0,0,0,1,
     0,0,0,1,0,0,0,1,0,0,0,0,0,1,0,0,0,1,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,1,0,0,
     0,0,1,0,0,0,0,0,0,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,0,0,0,0,1,1,0,1,1,0,0,1,1,0,0,1,0,0,1,0,0,0,1,0,
     0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,0,0,0,1,0,0,0,1,0,0,1,0,0,0,0,0,0,1,0,0,0,0,
     0,0,1,0,0,0,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,1,0,1,0,0,0,0,1,0,0,0,0,0,0,1,0,1,0,
     1,0,0,1,0,1,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,1,0,0,1,0,0,0,1,0,0,1,1,1,1,0,0,0,1,0,0,0,0,0,0,1,0,0,
     0,1,0,0,1,1,1,1,0,0,0,1,1,1,1,0,0,0,1,0,0,0,0,0,0,1,1,1,1,1,0,0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,1,1,
     0,0,0,0,0,1,0,0,0,0,0,0,1,0,1,0,1,0,0,1,0,0,1,1,0,0,1,0,0,0,1,0,0,1,0,1,0,1,0,0,1,1,1,1,1,0,0,1,
     0,0,0,1,0,0,1,0,0,0,0,0,0,1,0,0,0,1,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,1,1,0,0,1,0,0,0,1,0,0,
     0,0,1,0,0,0,0,0,0,0,0,1,0,0,1,1,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,
     0,1,0,1,1,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,0,0,0,1,0,0,0,1,0,0,1,0,0,0,0,0,0,1,0,0,0,0,
     0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,1,0,1,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,
     1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,0,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,0,1,0,0,
     1,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,1,0,
     0,1,0,0,0,1,0,0,0,0,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,0,0,0,1,0,0,0,1,0,0,1,
     1,1,1,0,0,0,0,0,1,1,1,0,0,1,1,1,0,0,0,0,1,1,1,1,1,0,0,1,0,0,0,0,0,0,0,0,1,1,1,0,0,1,0,0,0,1,0,0,
     1,1,1,1,1,0,0,0,1,1,1,0,0,0,1,0,0,0,1,0,0,1,1,1,1,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,0,1,1,1,0,0,
     0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,1,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
     0,1,1,1,1,0,0,0,0,1,1,1,0,0,0,1,1,1,1,0,0,0,0,1,1,1,1,0,0,1,1,1,1,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,
     0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,1,1,1,1,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1,
     0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,0,0,0,0,0,1,
     0,0,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,0,0,0,1,0,0,0,0,1,
     0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,0,1,0,1,0,0,0,0,0,0,0,0,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,
     0,0,0,1,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,0,1,0,1,0,0,0,
     0,1,0,1,0,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,0,0,0,0,
     0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,0,1,1,0,0,0,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,1,0,0,0,1,
     0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,1,0,0,0,1,0,0,1,1,1,1,0,0,0,0,0,0,1,0,0,0,0,0,1,
     0,0,0,0,1,0,0,0,1,0,0,0,1,0,1,0,0,0,1,0,1,0,1,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,1,
     0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,1,0,0,1,
     0,1,0,0,0,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,1,0,1,0,0,0,1,0,1,0,1,0,0,0,1,0,1,0,0,0,
     0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
     0,1,0,0,0,0,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,
     0,0,1,0,1,0,1,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,1,
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,1,1,0,0,0,1,0,0,0,1,0,0,1,1,1,1,0,0,0,0,0,1,
     0,0,0,0,0,1,1,1,0,0,0,0,0,1,0,0,0,0,1,1,0,1,1,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,0,1,1,1,1,1,0,0,0,1,
     0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
     0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,1,1,1,
     0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,1,0,
     0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,1,
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,
     0,0,1,0,0,0,0,0,0,0,1,0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
     0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1,0,0,0,
     0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,1,0,1,1,0,0,0,0,1,1,1,1,0,0,0,1,1,
     1,1,0,0,0,1,1,1,0,0,0,1,1,1,1,1,0,0,0,1,1,1,1,0,0,1,0,1,1,0,0,0,1,1,1,0,0,0,0,0,1,1,1,0,0,0,1,0,
     0,1,0,0,0,0,0,1,0,0,0,0,1,1,1,1,0,0,0,1,0,1,1,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,1,
     1,0,0,1,0,0,1,0,0,0,0,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,0,1,0,0,0,1,0,0,1,1,0,0,1,0,0,
     0,0,1,0,0,0,0,0,0,0,1,0,0,0,1,0,1,0,0,0,0,0,0,1,0,0,0,0,1,0,1,0,1,0,0,1,1,0,0,1,0,0,1,0,0,0,1,0,
     0,0,0,0,0,0,0,0,0,1,1,1,1,0,0,1,0,0,0,1,0,0,1,0,0,0,0,0,0,1,0,0,0,1,0,0,1,1,1,1,1,0,0,0,1,0,0,0,
     0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,1,1,0,0,0,0,0,0,0,1,0,0,0,0,1,0,1,0,
     1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,0,0,0,1,0,0,
     0,1,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,1,0,
     1,0,0,0,0,0,0,1,0,0,0,0,1,0,1,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,1,0,0,1,
     0,0,0,1,0,0,1,0,0,0,0,0,0,1,0,0,0,1,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,
     0,0,1,0,0,0,0,0,0,0,1,0,0,0,1,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0,1,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,
     0,0,0,0,0,0,0,0,0,1,1,1,1,0,0,1,1,1,1,0,0,0,0,1,1,1,1,0,0,0,1,1,1,1,0,0,0,1,1,1,1,0,0,0,1,0,0,0,
     0,0,0,1,1,1,1,0,0,1,0,0,0,1,0,0,1,1,1,1,1,0,0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,0,1,1,0,0,1,0,1,0,
     1,0,0,1,0,0,0,1,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,
     0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,1,0,0,0,0,1,1,0,0,
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
     1,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,1,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,1,0,1,0,1,0,0,0,0,0,0,0,0,
     0,1,0,1,1,0,0,0,0,1,1,1,1,0,0,1,0,1,1,1,0,0,0,1,1,1,1,0,0,1,1,1,1,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,
     0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,1,1,1,1,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,
     0,0,0,1,0,0,1,0,0,0,0,0,0,0,0,0,0,1,1,0,0,1,0,0,1,0,0,0,1,0,0,1,1,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,
     0,0,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,0,1,0,1,0,0,0,1,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,
     1,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,
     0,0,0,0,0,0,0,1,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,1,0,0,0,1,0,1,0,0,0,1,0,1,0,1,0,0,0,0,1,0,0,0,0,
     1,0,0,0,1,0,0,0,0,1,0,0,0,0,1,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
     0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,1,0,0,0,1,0,1,0,
     0,0,1,0,1,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,1,0,0,1,0,0,1,1,0,0,1,0,0,0,0,0,0,0,0,0,0,1,0,0,0,1,0,
     0,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,1,0,1,0,0,0,1,0,1,0,0,0,1,0,0,1,1,0,0,0,1,0,0,0,0,0,0,0,
     1,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,1,1,0,1,0,0,1,
     0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,1,1,1,0,0,0,1,1,1,1,0,0,0,0,1,0,0,0,0,1,1,0,1,1,0,0,1,0,0,0,1,0,0,
     0,1,1,0,1,0,0,1,1,1,1,1,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
     0,1,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,
     0,1,1,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1,0,0,0,
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
     0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
};
