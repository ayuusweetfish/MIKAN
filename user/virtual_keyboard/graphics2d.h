#ifndef __GRAPHICS2d_H__
#define __GRAPHICS2d_H__
#include <stdint.h>
#include <stdbool.h>

#ifndef TEX_W
#define TEX_W 400
#endif

#ifndef TEX_H
#define TEX_H 240
#endif

extern uint8_t buf[TEX_H][TEX_W][3];

// filter
extern bool (*filter) (int16_t, int16_t);

extern bool wholefilter(int16_t x, int16_t y);

extern int16_t rectfilter_x, rectfilter_y;
extern uint16_t rectfilter_w, rectfilter_h;
extern bool rectfilter(int16_t x, int16_t y);

inline void clip_r() { filter = wholefilter; }
inline void clip(int16_t x, int16_t y, uint16_t w, uint16_t h) {
  rectfilter_x = x; rectfilter_y = y; rectfilter_w = w; rectfilter_h = h;
  filter = rectfilter;
}

// mask
#define WRITEMASK   (1 << 0)
#define USEMASK     (1 << 1)

extern uint8_t mask;
extern uint8_t maskbuf[TEX_H][TEX_W];

inline void writemask(bool b) {
  if (b) mask |= WRITEMASK;
  else mask &= ~WRITEMASK;
}

inline void usemask(bool b) {
  if (b) mask |= USEMASK;
  else mask &= ~USEMASK;
}

// button
extern uint32_t T;
extern uint32_t b0, b1;
#define btn(__b)    (!!(b0 & (__b)))
#define btnp(__b)   ((b0 & (__b)) && !(b1 & (__b)))

// color
typedef struct RGB {
  uint8_t r;
  uint8_t g;
  uint8_t b;
} RGB;

extern RGB palette[16];
extern RGB *bcolor;
extern RGB *fcolor;

inline void pix(uint16_t x, uint16_t y, RGB *color)
{
  if (filter(x, y)) {
    if (mask & WRITEMASK) {
      maskbuf[y][x] = 255;

    } else if (mask & USEMASK) {
      buf[y][x][2] += (((int16_t)(color->r) - buf[y][x][2]) * maskbuf[y][x]) >> 8;
      buf[y][x][1] += (((int16_t)(color->g) - buf[y][x][1]) * maskbuf[y][x]) >> 8;
      buf[y][x][0] += (((int16_t)(color->b) - buf[y][x][0]) * maskbuf[y][x]) >> 8;
    } else {
      buf[y][x][2] = color->r;
      buf[y][x][1] = color->g;
      buf[y][x][0] = color->b;
    }
  }
  
}

inline void pix_a(uint16_t x, uint16_t y, RGB *color, uint8_t a) {
  if (filter(x, y)) {
    if (mask & WRITEMASK) {
      maskbuf[y][x] = a;
    } else if (mask & USEMASK) {
      buf[y][x][2] += (((((int16_t)(color->r) - buf[y][x][2]) * a) >> 8) * maskbuf[y][x]) >> 8;
      buf[y][x][1] += (((((int16_t)(color->g) - buf[y][x][1]) * a) >> 8) * maskbuf[y][x]) >> 8;
      buf[y][x][0] += (((((int16_t)(color->b) - buf[y][x][0]) * a) >> 8) * maskbuf[y][x]) >> 8;
    } else {
      buf[y][x][2] += (((int16_t)(color->r) - buf[y][x][2]) * a) >> 8;
      buf[y][x][1] += (((int16_t)(color->g) - buf[y][x][1]) * a) >> 8;
      buf[y][x][0] += (((int16_t)(color->b) - buf[y][x][0]) * a) >> 8;
    }
  }
}

inline void rect(int16_t x, int16_t y, uint16_t w, uint16_t h, RGB *color) {
  int x0, y0;
  for (int w0 = 0; w0 < w; w0++) if ((x0 = x + w0) >= 0 && x0 < TEX_W)
				   for (int h0 = 0; h0 < h; h0++) if ((y0 = y + h0) >= 0 && y0 < TEX_H)
								    pix(x0, y0, color);
}

inline void rect_a(int16_t x, int16_t y, uint16_t w, uint16_t h, RGB *color, uint8_t a) {
  int x0, y0;
  for (int w0 = 0; w0 < w; w0++) if ((x0 = x + w0) >= 0 && x0 < TEX_W)
				   for (int h0 = 0; h0 < h; h0++) if ((y0 = y + h0) >= 0 && y0 < TEX_H)
								    pix_a(x0, y0, color, a);
}

inline void rect_cen(int16_t x, int16_t y, uint8_t half_w, uint8_t half_h, RGB *color) {
     rect(x - half_w, y - half_h, (half_w << 1) + 1, (half_h << 1) + 1, color);
}

inline void rect_cen_a(int16_t x, int16_t y, uint8_t half_w, uint8_t half_h, RGB *color, uint8_t a) {
  rect_a(x - half_w, y - half_h, (half_w << 1) + 1, (half_h << 1) + 1, color, a);
}

inline void linev(int16_t x, int16_t y1, int16_t y2, RGB *color, uint8_t d) {
     rect(x, y1, d, y2 - y1 + 1, color);
}

inline void linev_a(int16_t x, int16_t y1, int16_t y2, RGB *color, uint8_t d, uint8_t a) {
  rect_a(x, y1, d, y2 - y1 + 1, color, a);
}

inline void lineh(int16_t x1, int16_t x2, int16_t y, RGB *color, uint8_t d) {
  rect(x1, y, x2 - x1 + 1, d, color);
}

inline void lineh_a(int16_t x1, int16_t x2, int16_t y, RGB *color, uint8_t d, uint8_t a) {
  rect_a(x1, y, x2 - x1 + 1, d, color, a);
}

inline void dashlinev(int16_t x, int16_t y1, int16_t y2, RGB *color, uint8_t d, uint8_t l, uint8_t s) {
     uint16_t y_next;
     while ((y_next = y1 + l + s) <= y2) {
       rect(x, y1, d, l, color);
	  y1 = y_next;
     }
     linev(x, y1, y2, color, d);
}

inline void dashlinev_a(int16_t x, int16_t y1, int16_t y2, RGB *color, uint8_t d, uint8_t l, uint8_t s, uint8_t a) {
     uint16_t y_next;
     while ((y_next = y1 + l + s) <= y2) {
       rect_a(x, y1, d, l, color, a);
	  y1 = y_next;
     }
     linev_a(x, y1, y2, color, d, a);
}

inline void dashlineh(int16_t x1, int16_t x2, int16_t y, RGB *color, uint8_t d, uint8_t l, uint8_t s) {
     uint16_t x_next;
     while ((x_next = x1 + l + s) <= x2) {
       rect(x1, y, l, d, color);
	  x1 = x_next;
     }
     lineh(x1, x2, y, color, d);
}

inline void dashlineh_a(int16_t x1, int16_t x2, int16_t y, RGB *color, uint8_t d, uint8_t l, uint8_t s, uint8_t a) {
     uint16_t x_next;
     while ((x_next = x1 + l + s) <= x2) {
       rect_a(x1, y, l, d, color, a);
	  x1 = x_next;
     }
     lineh_a(x1, x2, y, color, d, a);
}

inline void rectb(int16_t x, int16_t y, uint16_t w, uint16_t h, RGB *color, int8_t d) {
     lineh(x, x + w - 1, y, color, d);
     lineh(x, x + w - 1, y + h - d, color, d);
     linev(x, y + d, y + h - d - 1, color, d);
     linev(x + w - d, y + d, y + h - d - 1, color, d);
}

inline void rectb_a(int16_t x, int16_t y, uint16_t w, uint16_t h, RGB *color, int8_t d, uint8_t a) {
  lineh_a(x, x + w - 1, y, color, d, a);
  lineh_a(x, x + w - 1, y + h - d, color, d, a);
  linev_a(x, y + d, y + h - d - 1, color, d, a);
  linev_a(x + w - d, y + d, y + h - d - 1, color, d, a);
}

inline void rectdb(int16_t x, int16_t y, uint16_t w, uint16_t h, RGB *color, int8_t d, uint8_t l, uint8_t s) {
     dashlineh(x, x + w - 1, y, color, d, l, s);
     dashlineh(x, x + w - 1, y + h - d, color, d, l, s);
     dashlinev(x, y + d, y + h - d - 1, color, d, l, s);
     dashlinev(x + w - d, y + d, y + h - d - 1, color, d, l, s);
}

inline void rectdb_a(int16_t x, int16_t y, uint16_t w, uint16_t h, RGB *color, int8_t d, uint8_t l, uint8_t s, uint8_t a) {
  dashlineh_a(x, x + w - 1, y, color, d, l, s, a);
  dashlineh_a(x, x + w - 1, y + h - d, color, d, l, s, a);
  dashlinev_a(x, y + d, y + h - d - 1, color, d, l, s, a);
  dashlinev_a(x + w - d, y + d, y + h - d - 1, color, d, l, s, a);
}

inline void circ(int16_t x, int16_t y, uint16_t r, RGB* color) {
  int x0, y0;
  for (int xi = -r; xi <= r; xi++) if ((x0 = x +xi) >= 0 && x0 < TEX_W) 
  for (int yi = -r; yi <= r; yi++) if ((y0 = y +yi) >= 0 && y0 < TEX_H) 
				     if (xi * xi + yi * yi <= r * (r + 1))
	      pix(x0, y0, color);
}

inline void circ_a(int16_t x, int16_t y, uint16_t r, RGB* color, uint8_t a) {
  int x0, y0;
  for (int xi = -r; xi <= r; xi++) if ((x0 = x +xi) >= 0 && x0 < TEX_W) 
  for (int yi = -r; yi <= r; yi++) if ((y0 = y +yi) >= 0 && y0 < TEX_H) 
				     if (xi * xi + yi * yi <= r * (r + 1))
				       pix_a(x0, y0, color, a);
}

inline void circb(int16_t x, int16_t y, uint16_t r, RGB* color, uint8_t d) {
  int x0, y0;
  for (int xi = -r; xi <= r; xi++) if ((x0 = x +xi) >= 0 && x0 < TEX_W) 
  for (int yi = -r; yi <= r; yi++) if ((y0 = y +yi) >= 0 && y0 < TEX_H) 
				     if ((xi * xi + yi * yi <= r * (r + 1)) && (xi * xi + yi * yi >= (r - d) * (r - d + 1)))
	      pix(x0, y0, color);
}

inline void circb_a(int16_t x, int16_t y, uint16_t r, RGB* color, uint8_t d, uint8_t a) {
  int x0, y0;
  for (int xi = -r; xi <= r; xi++) if ((x0 = x +xi) >= 0 && x0 < TEX_W) 
  for (int yi = -r; yi <= r; yi++) if ((y0 = y +yi) >= 0 && y0 < TEX_H) 
				     if ((xi * xi + yi * yi <= r * (r + 1)) && (xi * xi + yi * yi >= (r - d) * (r - d + 1)))
				       pix_a(x0, y0, color, a);
}

inline void ellipse(int16_t x, int16_t y, uint16_t a, uint16_t b, RGB* color) {
  int x0, y0;
  for (int xi = -a; xi <= a; xi++) if ((x0 = x +xi) >= 0 && x0 < TEX_W) 
				     for (int yi = -b; yi <= b; yi++) if ((y0 = y +yi) >= 0 && y0 < TEX_H) {
				     int a_1 = a * (a + 1);
  				     int b_1 = b * (b + 1);
  				     if (xi * xi * b_1 + yi * yi * a_1 <= a_1 * b_1)
				       pix(x0, y0, color);
				       }
}

inline void ellipse_a(int16_t x, int16_t y, uint16_t a, uint16_t b, RGB* color, uint8_t alpha) {
  int x0, y0;
  for (int xi = -a; xi <= a; xi++) if ((x0 = x +xi) >= 0 && x0 < TEX_W) 
				     for (int yi = -b; yi <= b; yi++) if ((y0 = y +yi) >= 0 && y0 < TEX_H) {
				     int a_1 = a * (a + 1);
  				     int b_1 = b * (b + 1);
  				     if (xi * xi * b_1 + yi * yi * a_1 <= a_1 * b_1)
				       pix_a(x0, y0, color, alpha);
				       }
}

inline void ellipseb(int16_t x, int16_t y, uint16_t a, uint16_t b, RGB* color, uint8_t d) {
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

inline void ellipseb_a(int16_t x, int16_t y, uint16_t a, uint16_t b, RGB* color, uint8_t d, uint8_t alpha) {
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

inline void round_rect(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t r, RGB* color) {

  clip(x, y, r, r);
  circ(x + r, y + r, r, color);
  clip(x + w - r, y, r, r);
  circ(x + w - r - 1, y + r, r, color);
  clip(x, y + h - r, r, r);
  circ(x + r, y + h - r - 1, r, color);
  clip(x + w - r, y + h - r, r, r);
  circ(x + w - r - 1, y + h - r - 1, r, color);
  clip_r();

  rect(x + r, y, w - 2 * r, h, color);
  rect(x, y + r, r , h - 2 * r, color);
  rect(x + w - r, y + r, r, h - 2 * r, color);

}

inline void round_rect_a(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t r, RGB* color, uint8_t a) {

  clip(x, y, r, r);
  circ_a(x + r, y + r, r, color, a);
  clip(x + w - r, y, r, r);
  circ_a(x + w - r - 1, y + r, r, color, a);
  clip(x, y + h - r, r, r);
  circ_a(x + r, y + h - r - 1, r, color, a);
  clip(x + w - r, y + h - r, r, r);
  circ_a(x + w - r - 1, y + h - r - 1, r, color, a);
  clip_r();

  rect_a(x + r, y, w - 2 * r, h, color, a);
  rect_a(x, y + r, r , h - 2 * r, color, a);
  rect_a(x + w - r, y + r, r, h - 2 * r, color, a);

}

inline void round_rectb(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t r, RGB* color, uint8_t d) {
  clip(x, y, r, r);
  circb(x + r, y + r, r, color, d);
  clip(x + w - r, y, r, r);
  circb(x + w - r - 1, y + r, r, color, d);
  clip(x, y + h - r, r, r);
  circb(x + r, y + h - r - 1, r, color, d);
  clip(x + w - r, y + h - r, r, r);
  circb(x + w - r - 1, y + h - r - 1, r, color, d);
  clip_r();

  rect(x + r, y, w - 2 * r, d, color);
  rect(x, y + r, d, h - 2 * r, color);
  rect(x + r, y + h - d, w - 2 * r, d, color);
  rect(x + w - d, y + r, d, h - 2 * r, color); 
}

inline void cls(RGB* color) {
     for (int x = 0; x < TEX_W; x++)
	  for (int y = 0; y < TEX_H; y++)
	       pix(x, y, color);
}



#endif
