#include "api.h"
#include "graphics2d_api.h"
#include <stdbool.h>
// KEYBOARD
/*

q w e r t y u i o p
 a s d f g h j k l 
   z x c v b n m 


handler -> keyhold? -> command -> direct or keyaction
*/
// KEYBOARD_END




#define KEYBOARD_FRAME 5
#define KEY_WIDTH 29


static uint8_t graph[TEX_H][TEX_W][3]; 
static uint8_t symbols[] = {
#include "symbols.h"
};
static inline void text_symbol(int16_t x, int16_t y, uint8_t n) {
  uint16_t ptr = n * CHAR_W;
  for (uint8_t j = 0; j < CHAR_H; j++)
    for (uint8_t i = 0; i < CHAR_W; i++)
      if (symbols[ptr + (j * CHAR_W * 16 + i)])
	pix(x + i, y + j);
}


#define INPUTLENGTH 32
static char inputbox[INPUTLENGTH];
uint8_t cursor = 0;

typedef struct KEYBOARD_ELEMENT {
  int16_t x,  y;
  uint8_t row, col;
  void (*draw)(struct KEYBOARD_ELEMENT *self);
  void (*action)(struct KEYBOARD_ELEMENT *self, uint8_t action_type);

} KBD_ELE;

static const char *keyboard_alphabeta[4] = { "qwertyuiop",
					     "asdfghjkl<",
					     "^zxcvbnm,.",
					     "CMN____|{}", };
static KBD_ELE keyboard_1[4][10];
static uint8_t kbd_focus_position[2] = { 0, 0 };
static uint32_t key_hold_time[8] = { 0 };
static uint8_t key_trigger_status = 0;
static bool framefocus = true; //


static inline bool keyhold(uint8_t i, uint16_t dur, uint8_t peri) {
  bool result = false;
  if (btnp(1 << i)) {
    key_trigger_status = 0;
    key_trigger_status |= (1 << i);
    for (int j = 0; j < 8; j++) {
      key_hold_time[j] = 0; 
    }
  }
  if (btnr(1 << i)) {
    key_hold_time[i] = 0;
    key_trigger_status &= ~(1 << i);
  }
  if (key_trigger_status & (1 << i)) {
    result = ((key_hold_time[i] == 0) || (key_hold_time[i] > dur && ((key_hold_time[i] % peri) == 0)));
    key_hold_time[i]++;
  }
  return result;
}

static void letterkey_draw (struct KEYBOARD_ELEMENT *self) {
  alpha = 100;
  if (framefocus && (kbd_focus_position[0] == self->row) && (kbd_focus_position[1] == self->col)) {
    transparent_copy(KEY_WIDTH, 0, graph, self->x, self->y, buffer, KEY_WIDTH, 30, bcolor);
    color = palette + 8;
  } else {
    transparent_copy(0, 0, graph, self->x, self->y, buffer, KEY_WIDTH, 30, bcolor);
    color = palette + 7;
  }
  text_char(self->x + 11, self->y + 8, keyboard_alphabeta[self->row][self->col]);
  alpha = 255;
}

static void backspace_draw (struct KEYBOARD_ELEMENT *self) {
  alpha = 100;
  if (framefocus && (kbd_focus_position[0] == self->row) && (kbd_focus_position[1] == self->col)) {
    transparent_copy(KEY_WIDTH, 0, graph, self->x, self->y, buffer, KEY_WIDTH, 30, bcolor);
    color = palette + 8;
  } else {
    transparent_copy(0, 0, graph, self->x, self->y, buffer, KEY_WIDTH, 30, bcolor);
    color = palette + 7;
  }
  text_symbol(self->x + 7, self->y + 8, 0);
  text_symbol(self->x + 7 + CHAR_W, self->y + 8, 1);
  alpha = 255;
}

static void space_draw (struct KEYBOARD_ELEMENT *self) {
  if (self->col == 3) {
    alpha = 100;
    static int xend = 3 * (KEY_WIDTH + KEYBOARD_FRAME);
    int x = 0;
    if (framefocus && (kbd_focus_position[0] == self->row) && (kbd_focus_position[1] == self->col)) {
      transparent_copy(KEY_WIDTH, 0, graph, self->x, self->y, buffer, KEY_WIDTH >> 1, 30, bcolor);
      for(x = 0; x < xend; x++)
	direct_copy(self->x + (KEY_WIDTH >> 1) - 1, self->y, buffer, self->x + (KEY_WIDTH >> 1) + x, self->y, buffer, 1, 30);
      color = palette + 8;
    } else {
      transparent_copy(0, 0, graph, self->x, self->y, buffer, KEY_WIDTH >> 1, 30, bcolor);
      for(x = 0; x < xend; x++)
	direct_copy(self->x + (KEY_WIDTH >> 1) - 1, self->y, buffer, self->x + (KEY_WIDTH >> 1) + x, self->y, buffer, 1, 30);
      transparent_copy(KEY_WIDTH >> 1, 0, graph, self->x + xend + (KEY_WIDTH >> 1), self->y, buffer, KEY_WIDTH - (KEY_WIDTH >> 1), 30, bcolor);
      color = palette + 7;
    }
    text_symbol(self->x + 4, self->y + 8, 3);
    text_symbol(self->x + 11, self->y + 8, 4);
    text_symbol(self->x + 18, self->y + 8, 4);
    text_symbol(self->x + 25, self->y + 8, 5);
    alpha = 255;
  }
}


static inline void key_position_fix() {
  if (kbd_focus_position[0] == 3) {
    if (kbd_focus_position[1] > 3 && kbd_focus_position[1] < 7 ) kbd_focus_position[1] = 3;
  }
}


 #define ACTION_CLICK 1
 static void letterkey_action(struct KEYBOARD_ELEMENT *self, uint8_t action_type) {
   if (cursor < INPUTLENGTH) {
     inputbox[cursor] = keyboard_alphabeta[kbd_focus_position[0]][kbd_focus_position[1]];
     cursor++;
   }
 }

static void backspace_action(struct KEYBOARD_ELEMENT *self, uint8_t action_type) {
  if (cursor > 0) cursor--;
  inputbox[cursor] = 0;
}



static void command_up(void) {
  if (keyhold(0, 20, 8)) {
    if (framefocus) {
      if (kbd_focus_position[0] > 0) {
	kbd_focus_position[0] = kbd_focus_position[0] - 1;
      }
    } else {
      // back to the prev command
    }
  }
}

static void command_down(void) {
  if (keyhold(1, 20, 8)) {
    if (framefocus) {
      if (kbd_focus_position[0] < 3) {
	kbd_focus_position[0] = kbd_focus_position[0] + 1;
	key_position_fix();
      }
    } else {
      // go to the next command 
    }
  }
}

static void command_left(void) {
  if (keyhold(2, 20, 8)) {
    if (framefocus) {
      if (kbd_focus_position[1] > 0) {
	kbd_focus_position[1] = kbd_focus_position[1] - 1;
	key_position_fix();
      }
    } else {
      // cursor left
    }
  }
}

static void command_right(void) {
  if (keyhold(3, 20, 8)) {
    if (framefocus) {
      if (kbd_focus_position[1] < 9) {
	kbd_focus_position[1] = kbd_focus_position[1] + 1;
	if (kbd_focus_position[0] == 3) {
	  if (kbd_focus_position[1] == 4) kbd_focus_position[1] = 7;
	}
      }
    } else {
      // cursor right
    }
  }
}

static void command_cro(void) {
  if (keyhold(4, 20, 8)) {
    backspace_action(&keyboard_1[1][9], ACTION_CLICK);
  }
}

static void command_cir(void) {
  if (keyhold(5, 20, 8)) {
    keyboard_1[kbd_focus_position[0]][kbd_focus_position[1]].action(&keyboard_1[kbd_focus_position[0]][kbd_focus_position[1]], ACTION_CLICK); 
  }
}

static void command_sqr(void) {
  // switch frame focus
}

static void command_tri(void) {
  // confirm
}

static void (*command[8])(void) = { command_up, command_down, command_left, command_right,
				    command_cro, command_cir, command_sqr, command_tri
};


 
static inline void kbd_handler () {
  for(int i = 0; i < 8; i++) {
    if (btn(1 << i)) command[i]();
 }
}


static inline void keyboard_init() {
  uint8_t i;
  int16_t xi, yi;
  int16_t x_i, y_i;
  xi = (TEX_W - (10 * (KEY_WIDTH + KEYBOARD_FRAME)) + KEYBOARD_FRAME) >> 1;
  x_i = xi;
  yi = 90;
  for (i = 0; i < 10; i++) {
    keyboard_1[0][i].x = xi;
    keyboard_1[0][i].y = yi;
    keyboard_1[0][i].row = 0;
    keyboard_1[0][i].col = i;
    keyboard_1[0][i].draw = letterkey_draw;
    keyboard_1[0][i].action = letterkey_action;
    xi = xi + KEY_WIDTH + KEYBOARD_FRAME;
  }
  yi = 125;
  xi = x_i;
  for (i = 0; i < 10; i++) {
    keyboard_1[1][i].x = xi;
    keyboard_1[1][i].y = yi;
    keyboard_1[1][i].row = 1;
    keyboard_1[1][i].col = i;
    keyboard_1[1][i].draw = letterkey_draw;
    keyboard_1[1][i].action = letterkey_action;
    xi = xi + KEY_WIDTH + KEYBOARD_FRAME;
  }
  keyboard_1[1][9].draw = backspace_draw;
  keyboard_1[1][9].action = backspace_action;
  yi = 160;
  xi = x_i;
  for (i = 0; i < 10; i++) {
    keyboard_1[2][i].x = xi;
    keyboard_1[2][i].y = yi;
    keyboard_1[2][i].row = 2;
    keyboard_1[2][i].col = i;
    keyboard_1[2][i].draw = letterkey_draw;
    keyboard_1[2][i].action = letterkey_action;    
    xi = xi + KEY_WIDTH + KEYBOARD_FRAME;
  }
  yi = 195;
  xi = x_i;
  for (i = 0; i < 10; i++) {
    keyboard_1[3][i].x = xi;
    keyboard_1[3][i].y = yi;
    keyboard_1[3][i].row = 3;
    keyboard_1[3][i].col = i;
    keyboard_1[3][i].draw = letterkey_draw;
    keyboard_1[3][i].action = letterkey_action;
    xi = xi + KEY_WIDTH + KEYBOARD_FRAME;
  }
  keyboard_1[3][3].draw = space_draw;
  keyboard_1[3][4].draw = space_draw;
  keyboard_1[3][5].draw = space_draw;
  keyboard_1[3][6].draw = space_draw;

  // DRAW GRAPH
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
  RGB temp  = { 0xe1, 0xde, 0xd6 };
  color = &temp;
  alpha = 255;
  round_rect(KEY_WIDTH, 0, KEY_WIDTH, 30, 4);
  color = palette + 8;
  alpha = 100;
  round_rectb(KEY_WIDTH, 0, KEY_WIDTH, 30, 4, 2);
  buffer = temp_buf;
  // DRAW GRAPH END
}

static inline void keyboard_update() {
  kbd_handler();  
}

static inline void keyboard_draw() {
  RGB* color_store = color;
  uint8_t alpha_store = alpha;
  uint8_t i;
  
  for (i = 0; i < 10; i++) {
    keyboard_1[0][i].draw(&keyboard_1[0][i]);
  }
  for (i = 0; i < 10; i++) {
    keyboard_1[1][i].draw(&keyboard_1[1][i]);
  }
  for (i = 0; i < 10; i++) {
    keyboard_1[2][i].draw(&keyboard_1[2][i]);
  }
  for (i = 0; i < 10; i++) {
    keyboard_1[3][i].draw(&keyboard_1[3][i]);
  }
  alpha = alpha_store;
  color = color_store;
}



// Main Loop

void init() {
  b0 = b1 = 0;
  clip_r();
  writemask(false);
  usemask(false);
  keyboard_init();
}

void update() {
  b1 = b0;
  b0 = buttons();
  keyboard_update();
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
  
  text_char(30, 30, '0' + kbd_focus_position[0]);
  text_char(60, 30, '0' + kbd_focus_position[1]);
  text_str(90, 30, inputbox);
  return (void *)buffer;
}

