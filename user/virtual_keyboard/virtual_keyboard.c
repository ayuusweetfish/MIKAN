#include <stdint.h>
#include <stdbool.h>

#ifndef TEX_W
#define TEX_W 400
#endif

#ifndef TEX_H
#define TEX_H 240
#endif

static uint8_t buf[TEX_H][TEX_W][3];

// pix filters
bool (filter*)(uint16_t, uint16_t);

bool wholefilter(uint16_t x, uint16_t y) {
  if ((x >= 0) && (x < TEX_W) && (y >= 0) && (y < TEX_H))
    return true;
  else
    return false;
}
  
static uint16_t rectfilter_x, rectfilter_y, rectfilter_w, rectfilter_h;

bool rectfilter(uint16_t x, uint16_t y) {
  if (wholefilter(x, y) && (x >= rectfilter_x) && (x < rectfilter_x + rectfilter_w) && (y >= rectfilter_y) && (y < rectfilter_y + rectfilter_h))
    return true;
  else
    return false;
}



static uint32_t T;
static uint32_t b0, b1;
#define btn(__b)    (!!(b0 & (__b)))
#define btnp(__b)   ((b0 & (__b)) && !(b1 & (__b)))

typedef struct RGB {
  uint8_t r;
  uint8_t g;
  uint8_t b;
} RGB;

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



RGB *bcolor = palette;
RGB *fcolor = palette + 15;

static inline void pix(uint16_t x, uint16_t y, RGB *color)
{
  if (filter(x, y)) {
    buf[y][x][2] = color->r;
    buf[y][x][1] = color->g;
    buf[y][x][0] = color->b;
  }
}

static inline void pix_a(uint16_t x, uint16_t y, RGB *color, uint8_t a) {
  if (filter(x, y)) {
  buf[y][x][2] += (((int16_t)(color->r) - buf[y][x][2]) * a) >> 8;
  buf[y][x][1] += (((int16_t)(color->g) - buf[y][x][1]) * a) >> 8;
  buf[y][x][0] += (((int16_t)(color->b) - buf[y][x][0]) * a) >> 8;
  }
}

static inline void rect(int16_t x, int16_t y, uint16_t w, uint16_t h, RGB *color) {
  int x0, y0;
  for (int w0 = 0; w0 < w; w0++) if ((x0 = x + w0) >= 0 && x0 < TEX_W)
				   for (int h0 = 0; h0 < h; h0++) if ((y0 = y + h0) >= 0 && y0 < TEX_H)
								    pix(x0, y0, color);
}

static inline void rect_a(int16_t x, int16_t y, uint16_t w, uint16_t h, RGB *color, uint8_t a) {
  int x0, y0;
  for (int w0 = 0; w0 < w; w0++) if ((x0 = x + w0) >= 0 && x0 < TEX_W)
				   for (int h0 = 0; h0 < h; h0++) if ((y0 = y + h0) >= 0 && y0 < TEX_H)
								    pix_a(x0, y0, color, a);
}

static inline void rect_cen(int16_t x, int16_t y, uint8_t half_w, uint8_t half_h, RGB *color) {
     rect(x - half_w, y - half_h, (half_w << 1) + 1, (half_h << 1) + 1, color);
}

static inline void rect_cen_a(int16_t x, int16_t y, uint8_t half_w, uint8_t half_h, RGB *color, uint8_t a) {
  rect_a(x - half_w, y - half_h, (half_w << 1) + 1, (half_h << 1) + 1, color, a);
}

static inline void linev(int16_t x, int16_t y1, int16_t y2, RGB *color, uint8_t d) {
     rect(x, y1, d, y2 - y1 + 1, color);
}

static inline void linev_a(int16_t x, int16_t y1, int16_t y2, RGB *color, uint8_t d, uint8_t a) {
  rect_a(x, y1, d, y2 - y1 + 1, color, a);
}

static inline void lineh(int16_t x1, int16_t x2, int16_t y, RGB *color, uint8_t d) {
  rect(x1, y, x2 - x1 + 1, d, color);
}

static inline void lineh_a(int16_t x1, int16_t x2, int16_t y, RGB *color, uint8_t d, uint8_t a) {
  rect_a(x1, y, x2 - x1 + 1, d, color, a);
}

static inline void dashlinev(int16_t x, int16_t y1, int16_t y2, RGB *color, uint8_t d, uint8_t l, uint8_t s) {
     uint16_t y_next;
     while ((y_next = y1 + l + s) <= y2) {
       rect(x, y1, d, l, color);
	  y1 = y_next;
     }
     linev(x, y1, y2, color, d);
}

static inline void dashlinev_a(int16_t x, int16_t y1, int16_t y2, RGB *color, uint8_t d, uint8_t l, uint8_t s, uint8_t a) {
     uint16_t y_next;
     while ((y_next = y1 + l + s) <= y2) {
       rect_a(x, y1, d, l, color, a);
	  y1 = y_next;
     }
     linev_a(x, y1, y2, color, d, a);
}

static inline void dashlineh(int16_t x1, int16_t x2, int16_t y, RGB *color, uint8_t d, uint8_t l, uint8_t s) {
     uint16_t x_next;
     while ((x_next = x1 + l + s) <= x2) {
       rect(x1, y, l, d, color);
	  x1 = x_next;
     }
     lineh(x1, x2, y, color, d);
}

static inline void dashlineh_a(int16_t x1, int16_t x2, int16_t y, RGB *color, uint8_t d, uint8_t l, uint8_t s, uint8_t a) {
     uint16_t x_next;
     while ((x_next = x1 + l + s) <= x2) {
       rect_a(x1, y, l, d, color, a);
	  x1 = x_next;
     }
     lineh_a(x1, x2, y, color, d, a);
}

static inline void rectb(int16_t x, int16_t y, uint16_t w, uint16_t h, RGB *color, int8_t d) {
     lineh(x, x + w - 1, y, color, d);
     lineh(x, x + w - 1, y + h - d, color, d);
     linev(x, y + d, y + h - d - 1, color, d);
     linev(x + w - d, y + d, y + h - d - 1, color, d);
}

static inline void rectb_a(int16_t x, int16_t y, uint16_t w, uint16_t h, RGB *color, int8_t d, uint8_t a) {
  lineh_a(x, x + w - 1, y, color, d, a);
  lineh_a(x, x + w - 1, y + h - d, color, d, a);
  linev_a(x, y + d, y + h - d - 1, color, d, a);
  linev_a(x + w - d, y + d, y + h - d - 1, color, d, a);
}

static inline void rectdb(int16_t x, int16_t y, uint16_t w, uint16_t h, RGB *color, int8_t d, uint8_t l, uint8_t s) {
     dashlineh(x, x + w - 1, y, color, d, l, s);
     dashlineh(x, x + w - 1, y + h - d, color, d, l, s);
     dashlinev(x, y + d, y + h - d - 1, color, d, l, s);
     dashlinev(x + w - d, y + d, y + h - d - 1, color, d, l, s);
}

static inline void rectdb_a(int16_t x, int16_t y, uint16_t w, uint16_t h, RGB *color, int8_t d, uint8_t l, uint8_t s, uint8_t a) {
  dashlineh_a(x, x + w - 1, y, color, d, l, s, a);
  dashlineh_a(x, x + w - 1, y + h - d, color, d, l, s, a);
  dashlinev_a(x, y + d, y + h - d - 1, color, d, l, s, a);
  dashlinev_a(x + w - d, y + d, y + h - d - 1, color, d, l, s, a);
}

static inline void circ(int16_t x, int16_t y, uint16_t r, RGB* color) {
  int x0, y0;
  for (int xi = -r; xi <= r; xi++) if ((x0 = x +xi) >= 0 && x0 < TEX_W) 
  for (int yi = -r; yi <= r; yi++) if ((y0 = y +yi) >= 0 && y0 < TEX_H) 
				     if (xi * xi + yi * yi <= r * (r + 1))
	      pix(x0, y0, color);
}

static inline void circ_a(int16_t x, int16_t y, uint16_t r, RGB* color, uint8_t a) {
  int x0, y0;
  for (int xi = -r; xi <= r; xi++) if ((x0 = x +xi) >= 0 && x0 < TEX_W) 
  for (int yi = -r; yi <= r; yi++) if ((y0 = y +yi) >= 0 && y0 < TEX_H) 
				     if (xi * xi + yi * yi <= r * (r + 1))
				       pix_a(x0, y0, color, a);
}

static inline void circb(int16_t x, int16_t y, uint16_t r, RGB* color, uint8_t d) {
  int x0, y0;
  for (int xi = -r; xi <= r; xi++) if ((x0 = x +xi) >= 0 && x0 < TEX_W) 
  for (int yi = -r; yi <= r; yi++) if ((y0 = y +yi) >= 0 && y0 < TEX_H) 
				     if ((xi * xi + yi * yi <= r * (r + 1)) && (xi * xi + yi * yi >= (r - d) * (r - d + 1)))
	      pix(x0, y0, color);
}

static inline void circb_a(int16_t x, int16_t y, uint16_t r, RGB* color, uint8_t d, uint8_t a) {
  int x0, y0;
  for (int xi = -r; xi <= r; xi++) if ((x0 = x +xi) >= 0 && x0 < TEX_W) 
  for (int yi = -r; yi <= r; yi++) if ((y0 = y +yi) >= 0 && y0 < TEX_H) 
				     if ((xi * xi + yi * yi <= r * (r + 1)) && (xi * xi + yi * yi >= (r - d) * (r - d + 1)))
				       pix_a(x0, y0, color, a);
}

static inline void ellipse(int16_t x, int16_t y, uint16_t a, uint16_t b, RGB* color) {
  int x0, y0;
  for (int xi = -a; xi <= a; xi++) if ((x0 = x +xi) >= 0 && x0 < TEX_W) 
				     for (int yi = -b; yi <= b; yi++) if ((y0 = y +yi) >= 0 && y0 < TEX_H) {
				     int a_1 = a * (a + 1);
  				     int b_1 = b * (b + 1);
  				     if (xi * xi * b_1 + yi * yi * a_1 <= a_1 * b_1)
				       pix(x0, y0, color);
				       }
}

static inline void ellipse_a(int16_t x, int16_t y, uint16_t a, uint16_t b, RGB* color, uint8_t alpha) {
  int x0, y0;
  for (int xi = -a; xi <= a; xi++) if ((x0 = x +xi) >= 0 && x0 < TEX_W) 
				     for (int yi = -b; yi <= b; yi++) if ((y0 = y +yi) >= 0 && y0 < TEX_H) {
				     int a_1 = a * (a + 1);
  				     int b_1 = b * (b + 1);
  				     if (xi * xi * b_1 + yi * yi * a_1 <= a_1 * b_1)
				       pix_a(x0, y0, color, alpha);
				       }
}
static inline void ellipseb(int16_t x, int16_t y, uint16_t a, uint16_t b, RGB* color, uint8_t d) {
  int x0, y0;
  for (int xi = -a; xi <= a; xi++) if ((x0 = x +xi) >= 0 && x0 < TEX_W) 
				     for (int yi = -b; yi <= b; yi++) if ((y0 = y +yi) >= 0 && y0 < TEX_H) {
				     int a_1 = a * (a + 1);
  				     int b_1 = b * (b + 1);
				     int a_2 = (a - d) * (a - d + 1);
  				     int b_2 = (b - d) * (b - d + 1);
  				     if ((xi * xi * b_1 + yi * yi * a_1 <= a_1 * b_1) && (xi * xi * b_2 + yi * yi * a_2 >= a_2 * b_2))
				       pix(x0, y0, color);
				       }
}

static inline void ellipseb_a(int16_t x, int16_t y, uint16_t a, uint16_t b, RGB* color, uint8_t d, uint8_t alpha) {
  int x0, y0;
  for (int xi = -a; xi <= a; xi++) if ((x0 = x +xi) >= 0 && x0 < TEX_W) 
				     for (int yi = -b; yi <= b; yi++) if ((y0 = y +yi) >= 0 && y0 < TEX_H) {
				     int a_1 = a * (a + 1);
  				     int b_1 = b * (b + 1);
				     int a_2 = (a - d) * (a - d + 1);
  				     int b_2 = (b - d) * (b - d + 1);
  				     if ((xi * xi * b_1 + yi * yi * a_1 <= a_1 * b_1) && (xi * xi * b_2 + yi * yi * a_2 >= a_2 * b_2))
				       pix_a(x0, y0, color, alpha);
				       }
}

static inline void cls(RGB* color) {
     for (int x = 0; x < TEX_W; x++)
	  for (int y = 0; y < TEX_H; y++)
	       pix(x, y, color);
}

void init() {
}

void update() {
}

void *draw() {
  
  cls(palette+6);
  rectdb(50, 50, 60, 40, palette+8, 2, 3, 5);

  RGB m = {200,200,200};
  rectdb_a(130, 50, 60, 40, &m, 2, 3, 5, 100);

  ellipseb_a(200, 130, 28, 60, &m, 4, 120);
  
  return (void *)buf;
}

