#include "api.h"
#include "graphics2d_api.h"
#include <stdbool.h>

// Start: Draw symbols
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
// End: Draw symbols 

// Start: Input box
#define INPUTMAXLENGTH 32
static char inputbox[INPUTMAXLENGTH + 1] = { 0 }; // last one is for \0 end signal;
static uint8_t cursor = 0;
static uint8_t inputlength = 0;
static uint32_t cursor_t = 0;
static inline void cursor_draw() {
  int delta_t = T - cursor_t;
  if (delta_t > 600 || delta_t % 60 < 30) rect(90 + cursor * CHAR_W, 30, CHAR_W, CHAR_H);

}

static inline void inputbox_draw() {
  rectb(84, 24, 240, 25, 2);
  text_str(90, 30, inputbox);
  cursor_draw();
}
// End: Input box


// Start: Basic structure of keyboard

static uint8_t graph[TEX_H][TEX_W][3];

#define KEYBOARD_FRAME 5
#define KEY_WIDTH 29

typedef struct KEYBOARD_ELEMENT {
  int16_t x,  y;
  uint8_t row, col;
  void (*draw)(struct KEYBOARD_ELEMENT *self);
  void (*action)(void);
} KBD_ELE;

// 4 * 10 alphabeta keyboard
static KBD_ELE keyboard_1[4][10];
static const char *keyboard_alphabeta[2][4] = {{ "qwertyuiop",
						 "asdfghjkl<",
						 "^zxcvbnm,.",
						 "CMN    |{}", },
					       { "QWERTYUIOP",
						 "ASDFGHJKL<",
						 "^ZXCVBNM?!",
						 "CMN    |{}", }};
// 4 * 8 number keyboard
static KBD_ELE keyboard_2[4][8];
static const char *keyboard_number[4] = { "()$~+123",
					  "{}%!-456",
					  "[]^@*789",
					  "<>&#/0<|"};

static uint8_t kbd_focus_position[2] = { 0, 0 };
static bool framefocus = true;
static uint8_t toggle_key = 0;
#define SHIFT          (1 << 0)
#define CMD            (1 << 1)
#define OPTION         (1 << 2)
#define SWITCH         (1 << 3)
// End: Basic structure of keyboard


// Start: Key Hold
#define HOLD_DURATION  12
#define HOLD_PERI       6
static uint32_t key_hold_time[8] = { 0 };
static uint8_t key_trigger_status = 0;
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
// End: Key Hold

// Start: Basic function
static uint8_t remember_pre_space_position = 0;
static inline void key_position_fix() {
  if (kbd_focus_position[0] == 3 && kbd_focus_position[1] >= 3 && kbd_focus_position[1] < 7 ) {
    remember_pre_space_position = kbd_focus_position[1];
    kbd_focus_position[1] = 3;
  }
}

static inline void leftcursor() {
  if (cursor > 0) {
    cursor--;
    cursor_t = T;
  }
}

static inline void rightcursor() {
  if (cursor < inputlength) {
    cursor++;
    cursor_t = T;
  }
}

// !! be careful, this function don't judge the range of m
static inline void strmove(int m) {
  int i;
  if (m > 0) {
    i = inputlength - 1;
    while(i >= cursor) {
      inputbox[m + i] = inputbox[i];
      inputbox[i] = ' ';
      i--;
    }
  } else if (m < 0) {
    i = cursor;
    while(i < inputlength + 1) {
      inputbox[m + i] = inputbox[i];
      inputbox[i] = 0;
      i++;
    }
  }
}

static char killbuffer[INPUTMAXLENGTH + 1];
static uint8_t killlength = 0;
static inline void kill() {
  char c;
  int i = 0;
  while((c = inputbox[cursor + i]) != 0) {
    killbuffer[i] = c;
    i++;
  }
  killlength = i;
  while(i != INPUTMAXLENGTH + 1) killbuffer[i] = 0;
  cursor_t = T;
}

static inline void yield() {
  if (inputlength + killlength < INPUTMAXLENGTH) {
    strmove(killlength);
    for(int i = 0; i < killlength; i++) {
      inputbox[cursor + i] = killbuffer[i];
    }
    cursor = cursor + killlength;
  }
  cursor_t = T;
}

static inline void typeletter(char c) {
  if (inputlength < INPUTMAXLENGTH) {
    strmove(1);
    inputbox[cursor] = c;
    cursor++;
    inputlength++;
  }
  cursor_t = T;
}

static inline void backdelete() {
  if (cursor > 0) {
    strmove(-1);
    cursor--;
    inputlength--;
  }
  cursor_t = T;
}

static inline void upkey() {
  if (kbd_focus_position[0] > 0) {
    kbd_focus_position[0] = kbd_focus_position[0] - 1;
    if (kbd_focus_position[1] == 3) kbd_focus_position[1] = remember_pre_space_position;
  }
}

static inline void downkey() {
  if (kbd_focus_position[0] < 3) {
    kbd_focus_position[0] = kbd_focus_position[0] + 1;
    key_position_fix();
  }
}

static inline void leftkey() {
  if (kbd_focus_position[1] > 0) {
    kbd_focus_position[1] = kbd_focus_position[1] - 1;
    key_position_fix();
  }
}

static inline void rightkey() {
  if (kbd_focus_position[1] < 9) {
    kbd_focus_position[1] = kbd_focus_position[1] + 1;
    if (kbd_focus_position[0] == 3) {
      if (kbd_focus_position[1] == 3) {
	remember_pre_space_position = 3;
      }
      if (kbd_focus_position[1] == 4) {
	kbd_focus_position[1] = 7;
      }
    }
  }
  
}
// End: Basic function


// Start: Key draw
static void letterkey_draw (struct KEYBOARD_ELEMENT *self) {
  alpha = 100;
  if (framefocus && (kbd_focus_position[0] == self->row) && (kbd_focus_position[1] == self->col)) {
    transparent_copy(KEY_WIDTH, 0, graph, self->x, self->y, buffer, KEY_WIDTH, 30, bcolor);
    color = palette + 8;
    text_char(self->x + 12, self->y + 9, keyboard_alphabeta[toggle_key & SHIFT][self->row][self->col]);
    alpha = 150;
  } else {
    transparent_copy(0, 0, graph, self->x, self->y, buffer, KEY_WIDTH, 30, bcolor);
    color = palette + 7;
  }
  text_char(self->x + 11, self->y + 8, keyboard_alphabeta[toggle_key & SHIFT][self->row][self->col]);
  alpha = 255;
}

static void backspace_draw (struct KEYBOARD_ELEMENT *self) {
  alpha = 100;
  if (framefocus && (kbd_focus_position[0] == self->row) && (kbd_focus_position[1] == self->col)) {
    transparent_copy(KEY_WIDTH, 0, graph, self->x, self->y, buffer, KEY_WIDTH, 30, bcolor);
    color = palette + 8;
    text_symbol(self->x + 8, self->y + 10, 0);
    text_symbol(self->x + 8 + CHAR_W, self->y + 10, 1);
    alpha = 150;
  } else {
    transparent_copy(0, 0, graph, self->x, self->y, buffer, KEY_WIDTH, 30, bcolor);
    color = palette + 7;
  }
  text_symbol(self->x + 7, self->y + 9, 0);
  text_symbol(self->x + 7 + CHAR_W, self->y + 9, 1);
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
      transparent_copy(KEY_WIDTH + (KEY_WIDTH >> 1), 0, graph, self->x + xend + (KEY_WIDTH >> 1), self->y, buffer, KEY_WIDTH - (KEY_WIDTH >> 1), 30, bcolor);
      color = palette + 8;
      x = self->x + 11;
      text_symbol(x, self->y + 11, 3);
      for (x += CHAR_W; x < self->x + (KEY_WIDTH << 2) - KEYBOARD_FRAME + 1; x += CHAR_W) {
	text_symbol(x, self->y + 11, 4);
      }
      text_symbol(x, self->y + 11, 5);
      alpha = 150;
    } else {
      transparent_copy(0, 0, graph, self->x, self->y, buffer, KEY_WIDTH >> 1, 30, bcolor);
      for(x = 0; x < xend; x++)
	direct_copy(self->x + (KEY_WIDTH >> 1) - 1, self->y, buffer, self->x + (KEY_WIDTH >> 1) + x, self->y, buffer, 1, 30);
      transparent_copy(KEY_WIDTH >> 1, 0, graph, self->x + xend + (KEY_WIDTH >> 1), self->y, buffer, KEY_WIDTH - (KEY_WIDTH >> 1), 30, bcolor);
      color = palette + 7;
    }
    x = self->x + 10;
    text_symbol(x, self->y + 10, 3);
    for (x += CHAR_W; x < self->x + (KEY_WIDTH << 2) - KEYBOARD_FRAME; x += CHAR_W) {
      text_symbol(x, self->y + 10, 4);
    }
    text_symbol(x, self->y + 10, 5);
    alpha = 255;
  }
}

static void switch_draw (struct KEYBOARD_ELEMENT *self) {
  alpha = 100;
  if ((toggle_key & SWITCH) || (framefocus && (kbd_focus_position[0] == self->row) && (kbd_focus_position[1] == self->col))) {
    transparent_copy(KEY_WIDTH, 0, graph, self->x, self->y, buffer, KEY_WIDTH, 30, bcolor);
    color = palette + 8;
    text_symbol(self->x + 8, self->y + 9, 8);
    text_symbol(self->x + 8 + CHAR_W, self->y + 9, 9);
    alpha = 150;
  } else {
    transparent_copy(0, 0, graph, self->x, self->y, buffer, KEY_WIDTH, 30, bcolor);
    color = palette + 7;
  }
  text_symbol(self->x + 7, self->y + 8, 8);
  text_symbol(self->x + 7 + CHAR_W, self->y + 8, 9);
  alpha = 255;
}

static void option_draw (struct KEYBOARD_ELEMENT *self) {
  alpha = 100;
  if ((toggle_key & OPTION) || (framefocus && (kbd_focus_position[0] == self->row) && (kbd_focus_position[1] == self->col))) {
    transparent_copy(KEY_WIDTH, 0, graph, self->x, self->y, buffer, KEY_WIDTH, 30, bcolor);
    color = palette + 8;
    text_symbol(self->x + 8, self->y + 9, 10);
    text_symbol(self->x + 8 + CHAR_W, self->y + 9, 11);
    alpha = 150;
  } else {
    transparent_copy(0, 0, graph, self->x, self->y, buffer, KEY_WIDTH, 30, bcolor);
    color = palette + 7;
  }
  text_symbol(self->x + 7, self->y + 8, 10);
  text_symbol(self->x + 7 + CHAR_W, self->y + 8, 11);
  alpha = 255;
}

static void cmd_draw (struct KEYBOARD_ELEMENT *self) {
  alpha = 100;
  if ((toggle_key & CMD)|| (framefocus && (kbd_focus_position[0] == self->row) && (kbd_focus_position[1] == self->col))) {
    transparent_copy(KEY_WIDTH, 0, graph, self->x, self->y, buffer, KEY_WIDTH, 30, bcolor);
    color = palette + 8;
    text_symbol(self->x + 8, self->y + 9, 12);
    text_symbol(self->x + 8 + CHAR_W, self->y + 9, 13);
    alpha = 150;
  } else {
    transparent_copy(0, 0, graph, self->x, self->y, buffer, KEY_WIDTH, 30, bcolor);
    color = palette + 7;
  }
  text_symbol(self->x + 7, self->y + 8, 12);
  text_symbol(self->x + 7 + CHAR_W, self->y + 8, 13);
  alpha = 255;
}

static void shift_draw (struct KEYBOARD_ELEMENT *self) {
  alpha = 100;
  if ((toggle_key & SHIFT) || (framefocus && (kbd_focus_position[0] == self->row) && (kbd_focus_position[1] == self->col))) {
    transparent_copy(KEY_WIDTH, 0, graph, self->x, self->y, buffer, KEY_WIDTH, 30, bcolor);
    color = palette + 8;
    text_symbol(self->x + 12, self->y + 9, 14);
    alpha = 150;
  } else {
    transparent_copy(0, 0, graph, self->x, self->y, buffer, KEY_WIDTH, 30, bcolor);
    color = palette + 7;
  }
  text_symbol(self->x + 11, self->y + 8, 14);
  alpha = 255;
}

static void return_draw (struct KEYBOARD_ELEMENT *self) {
  alpha = 100;
  if (framefocus && (kbd_focus_position[0] == self->row) && (kbd_focus_position[1] == self->col)) {
    transparent_copy(KEY_WIDTH, 0, graph, self->x, self->y, buffer, KEY_WIDTH, 30, bcolor);
    color = palette + 8;
    text_symbol(self->x + 12, self->y + 9, 2);
    alpha = 150;
  } else {
    transparent_copy(0, 0, graph, self->x, self->y, buffer, KEY_WIDTH, 30, bcolor);
    color = palette + 7;
  }
  text_symbol(self->x + 11, self->y + 8, 2);
  alpha = 255;
}

static void left_draw (struct KEYBOARD_ELEMENT *self) {
  alpha = 100;
  if (framefocus && (kbd_focus_position[0] == self->row) && (kbd_focus_position[1] == self->col)) {
    transparent_copy(KEY_WIDTH, 0, graph, self->x, self->y, buffer, KEY_WIDTH, 30, bcolor);
    color = palette + 8;
    text_symbol(self->x + 12, self->y + 9, 6);
    alpha = 150;
  } else {
    transparent_copy(0, 0, graph, self->x, self->y, buffer, KEY_WIDTH, 30, bcolor);
    color = palette + 7;
  }
  text_symbol(self->x + 11, self->y + 8, 6);
  alpha = 255;
}

static void right_draw (struct KEYBOARD_ELEMENT *self) {
  alpha = 100;
  if (framefocus && (kbd_focus_position[0] == self->row) && (kbd_focus_position[1] == self->col)) {
    transparent_copy(KEY_WIDTH, 0, graph, self->x, self->y, buffer, KEY_WIDTH, 30, bcolor);
    color = palette + 8;
    text_symbol(self->x + 12, self->y + 9, 7);
    alpha = 150;
  } else {
    transparent_copy(0, 0, graph, self->x, self->y, buffer, KEY_WIDTH, 30, bcolor);
    color = palette + 7;
  }
  text_symbol(self->x + 11, self->y + 8, 7);
  alpha = 255;
}
// End: Key draw


// Start: Key action

static void letterkey_action() {
  typeletter(keyboard_alphabeta[toggle_key & SHIFT][kbd_focus_position[0]][kbd_focus_position[1]]);
}

static void backspace_action() {
  backdelete();
}

static void space_action() {
  typeletter(' ');
}

static void right_action() {
  rightcursor();
}

static void left_action() {
  leftcursor();
}

static void shift_action() {
  toggle_key ^= SHIFT;
}

static void cmd_action() {
  toggle_key ^= CMD;
}

static void switch_action() {
  toggle_key ^= SWITCH;
}

static void option_action() {
  toggle_key ^= OPTION;
}
// End: Key action



// Start: Command
static void command_up(void) {
  if (keyhold(0, HOLD_DURATION, HOLD_PERI)) {
    if (framefocus) {
      upkey();
    } else {
      // back to the prev command
    }
  }
}

static void command_down(void) {
  if (keyhold(1, HOLD_DURATION, HOLD_PERI)) {
    if (framefocus) {
      downkey();
    } else {
      // go to the next command 
    }
  }
}

static void command_left(void) {
  if (keyhold(2, HOLD_DURATION, HOLD_PERI)) {
    if (framefocus) {
      leftkey();
    } else {
      // cursor left
    }
  }
}

static void command_right(void) {
  if (keyhold(3, HOLD_DURATION, HOLD_PERI)) {
    if (framefocus) {
      rightkey();
    } else {
      // cursor right
    }
  }
}

static void command_cro(void) {
  if (keyhold(4, HOLD_DURATION, HOLD_PERI)) {
    backdelete();
  }
}

static void command_cir(void) {
  if (keyhold(5, HOLD_DURATION, HOLD_PERI)) {
    keyboard_1[kbd_focus_position[0]][kbd_focus_position[1]].action();
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

// End: Command

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
  keyboard_1[2][0].draw = shift_draw;
  keyboard_1[2][0].action = shift_action;
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
  keyboard_1[3][0].draw = cmd_draw;
  keyboard_1[3][0].action = cmd_action;
  keyboard_1[3][1].draw = option_draw;
  keyboard_1[3][1].action = option_action;
  keyboard_1[3][2].draw = switch_draw;
  keyboard_1[3][2].action = switch_action;
  keyboard_1[3][3].draw = space_draw;
  keyboard_1[3][3].action = space_action;
  keyboard_1[3][4].draw = space_draw;
  keyboard_1[3][5].draw = space_draw;
  keyboard_1[3][6].draw = space_draw;
  keyboard_1[3][7].draw = return_draw;
  keyboard_1[3][8].draw = left_draw;
  keyboard_1[3][8].action = left_action;
  keyboard_1[3][9].draw = right_draw;
  keyboard_1[3][9].action = right_action;
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

  if (toggle_key & SWITCH) {
  } else {
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
  }
  alpha = alpha_store;
  color = color_store;
}

// Main Loop

void init() {
  b0 = b1 = T = 0;
  
  clip_r();
  writemask(false);
  usemask(false);
  keyboard_init();
}

void update() {
  b1 = b0;
  b0 = buttons();
  keyboard_update();
  T = (T + 1) % 5184000;
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
  inputbox_draw();
  return (void *)buffer;
}
