#include "graphics2d.h"

#include <stdint.h>
#include <stdbool.h>


#ifndef TEX_W
#define TEX_W 400
#endif

#ifndef TEX_H
#define TEX_H 240
#endif

uint8_t buf[TEX_H][TEX_W][3];

// pix filters
bool (*filter) (int16_t, int16_t);

bool wholefilter(int16_t x, int16_t y) {
  if ((x >= 0) && (x < TEX_W) && (y >= 0) && (y < TEX_H))
    return true;
  else
    return false;
}
  
int16_t rectfilter_x, rectfilter_y;
uint16_t rectfilter_w, rectfilter_h;

bool rectfilter(int16_t x, int16_t y) {
  if (wholefilter(x, y) && (x >= rectfilter_x) && (x < rectfilter_x + rectfilter_w) && (y >= rectfilter_y) && (y < rectfilter_y + rectfilter_h))
    return true;
  else
    return false;
}



// pix mask

uint8_t mask;

uint8_t maskbuf[TEX_H][TEX_W];

uint32_t T;
uint32_t b0, b1;

RGB palette[16] = {
		   // 0 black
		   { 0x14, 0x0c, 0x1c },
		   // 1 Coconut Brown 
		   { 0x44, 0x24, 0x34 },
		   // 2 Marine Blue
		   { 0x30, 0x34, 0x6d },
		   // 3 dark grey
		   { 0x4e, 0x4a, 0x4e },
		   // 4 light brown
		   { 0x85, 0x4c, 0x30 },
		   // 5 dark green
		   { 0x34, 0x65, 0x25 },
		   // 6 red
		   { 0xd0, 0x46, 0x48 },
		   // 7 city grey
		   { 0x75, 0x71, 0x61 },
		   // 8 blue
		   { 0x59, 0x7d, 0xce },
		   // 9 orange
		   { 0xd2, 0x7d, 0x2c },
		   // 10 grey
		   { 0x85, 0x95, 0xa1 },
		   // 11 green
		   { 0x6d, 0xaa, 0x2c },
		   // 12 pink
		   { 0xd2, 0xaa, 0x99 },
		   // 13 cyan
		   { 0x6d, 0xc2, 0xca },
		   // 14 yellow
		   { 0xda, 0xd4, 0x5e },
		   // 15 white
		   { 0xde, 0xee, 0xd6 }
};

RGB *color = palette + 15;
RGB *bcolor = palette;

uint8_t alpha = 255;
uint8_t balpha = 255;

uint8_t font_data[CHAR_W * CHAR_H * 16 * 6] = {
#include "font.h"
};
