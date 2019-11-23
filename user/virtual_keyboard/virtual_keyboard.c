#include "graphics2d_api.h"

// KEYBOARD
/*

q w e r t y u i o p
 a s d f g h j k l 
   z x c v b n m 
*/
// KEYBOARD_END




#define KEYBOARD_FRAME 5
#define KEY_WIDTH 29


static const char *keyboard_alphabeta[3] = {
					  "qwertyuiop",
					  "asdfghjkl",
					  "zxcvbnm"
};

static uint8_t graph[TEX_H][TEX_W][3]; 

static inline void keyboard_init() {
  uint8_t (*temp_buf)[TEX_W][3];
  temp_buf = buffer;
  buffer = graph;
  cls();
  color = palette + 15;
  alpha = 255;
  round_rect(0, 0, KEY_WIDTH, 30, 4);
  color = palette + 7;
  alpha = 100;
  round_rectb(0, 0, KEY_WIDTH, 30, 4, 2);
  buffer = temp_buf;
  
}

static inline void keyboard_draw() {
  RGB* color_store = color;
  uint8_t alpha_store = alpha;
  uint8_t i;
  int16_t xi, yi;
  int16_t x_i, y_i;

  xi = (TEX_W - (10 * (KEY_WIDTH + KEYBOARD_FRAME)) + KEYBOARD_FRAME) >> 1;
  x_i = xi;
  yi = 100;
  
  for (i = 0; i < 10; i++) {

    direct_copy(0, 0, graph, xi, yi, buffer, KEY_WIDTH, 30);
    text_char(xi + 11, yi + 8, keyboard_alphabeta[0][i]);
    xi = xi + KEY_WIDTH + KEYBOARD_FRAME;
  }
  
  yi = 135;
  xi = x_i + (KEY_WIDTH >> 1);
  for (i = 0; i < 9; i++) {
    direct_copy(0, 0, graph, xi, yi, buffer, KEY_WIDTH, 30);
    text_char(xi + 11, yi + 8, keyboard_alphabeta[1][i]);
    xi = xi + KEY_WIDTH + KEYBOARD_FRAME;

  }
  yi = 170;
  xi = x_i + (KEY_WIDTH >> 1);
  for (i = 0; i < 7; i++) {
    xi = xi + KEY_WIDTH + KEYBOARD_FRAME;
    direct_copy(0, 0, graph, xi, yi, buf, KEY_WIDTH, 30);
    text_char(xi + 11, yi + 8, keyboard_alphabeta[2][i]);
  }

  
  alpha = alpha_store;
  color = color_store;

  
}

// Main Loop

void init() {

  clip_r();
  writemask(false);
  usemask(false);
  keyboard_init();
}

void update() {
  T = (T+1)%300;
  
}

static inline void test() {
  alpha = 255;
  bcolor = palette + 6;
  cls();
  color = palette + 8;
  rectdb(50, 50, 60, 40, 2, 3, 5);
  RGB m = {200,200,200};
  color = &m;
  alpha = 100;
  rectdb(130, 50, 60, 40,  2, 3, 5);
  ellipseb(T-100, 130, 28, 60, 1);
  alpha = 255;
  text_char(100, 100, 'a');
  color = palette+14;
  alpha = 100;
  line_aa(40, 40, 100, 120);
  line(40, 60, 100, 140);
  circr_aa(100, 80, 50);
  circr(100+10, 80, 50);

}

void *draw() {
  
  cls();
  keyboard_draw();
  return (void *)buffer;
}

