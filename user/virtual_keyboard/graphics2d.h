#ifndef __GRAPHICS2D_H__
#define __GRAPHICS2D_H__
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>

#ifndef TEX_W
#define TEX_W 400
#endif

#ifndef TEX_H
#define TEX_H 240
#endif

#ifndef M_PI
#define M_PI  3.1415926535897932384626433832795
#endif
#ifndef M_SQRT1_2
#define M_SQRT1_2   0.7071067811865476
#endif

#define sgn(__s)       ((__s == 0) ? 0 : ((__s < 0) ? -1 : 1))
#define abs(__s)       ((__s < 0) ? -__s : __s)
#define max(__a, __b)  ((__a > __b) ? __a : __b)
#define min(__a, __b)  ((__a < __b) ? __a : __b)
#define btn(__b)       (!!(b0 & (__b)))

extern uint8_t buf[TEX_H][TEX_W][3];

extern uint8_t (*buffer)[TEX_W][3];

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

extern RGB *color;
extern RGB *bcolor;
extern uint8_t alpha;
extern uint8_t balpha;

inline void switch_color() {
  RGB* temp = bcolor;
  bcolor = color;
  color = temp;
}

inline void switch_alpha() {
  uint8_t temp = alpha;
  alpha = balpha;
  balpha = temp;
}

inline void pix_rgba(int16_t x, int16_t y, uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
  if (filter(x, y)) {
    if (a == 255) {
      if (mask & WRITEMASK) {
	maskbuf[y][x] = 255;
      } else if (mask & USEMASK) {
	buffer[y][x][2] += (((int16_t)(r) - buffer[y][x][2]) * maskbuf[y][x]) >> 8;
	buffer[y][x][1] += (((int16_t)(g) - buffer[y][x][1]) * maskbuf[y][x]) >> 8;
	buffer[y][x][0] += (((int16_t)(b) - buffer[y][x][0]) * maskbuf[y][x]) >> 8;
      } else {
	buffer[y][x][2] = r;
	buffer[y][x][1] = g;
	buffer[y][x][0] = b;
      }
    } else {
      if (mask & WRITEMASK) {
	maskbuf[y][x] = alpha;
      } else if (mask & USEMASK) {
	buffer[y][x][2] += (((((int16_t)(r) - buffer[y][x][2]) * a) >> 8) * maskbuf[y][x]) >> 8;
	buffer[y][x][1] += (((((int16_t)(g) - buffer[y][x][1]) * a) >> 8) * maskbuf[y][x]) >> 8;
	buffer[y][x][0] += (((((int16_t)(b) - buffer[y][x][0]) * a) >> 8) * maskbuf[y][x]) >> 8;
      } else {
	buffer[y][x][2] += (((int16_t)(r) - buffer[y][x][2]) * a) >> 8;
	buffer[y][x][1] += (((int16_t)(g) - buffer[y][x][1]) * a) >> 8;
	buffer[y][x][0] += (((int16_t)(b) - buffer[y][x][0]) * a) >> 8;
      }
    }
  }
  
}

inline void pix_rgb(int16_t x, int16_t y, uint8_t r, uint8_t g, uint8_t b)
{
  pix_rgba(x, y, r, g, b, alpha);
}

inline void pix(int16_t x, int16_t y)
{
  pix_rgba(x, y, color->r, color->g, color->b, alpha);
}

inline void pix_polar(float theta, float r, int16_t x, int16_t y) {
  pix((int)(r * cosf(theta) + 0.5f) + x, (int)(r * sinf(theta) + 0.5f) + y);
}



inline void rect(int16_t x, int16_t y, uint16_t w, uint16_t h) {
  int x0, y0;
  for (int w0 = 0; w0 < w; w0++) if ((x0 = x + w0) >= 0 && x0 < TEX_W)
				   for (int h0 = 0; h0 < h; h0++) if ((y0 = y + h0) >= 0 && y0 < TEX_H)
								    pix(x0, y0);
}

inline void rect_cen(int16_t x, int16_t y, uint8_t half_w, uint8_t half_h) {
  rect(x - half_w, y - half_h, (half_w << 1) + 1, (half_h << 1) + 1);
}

inline void linev(int16_t x, int16_t y1, int16_t y2, uint8_t d) {
     rect(x, y1, d, y2 - y1 + 1);
}
                              
inline void lineh(int16_t x1, int16_t x2, int16_t y, uint8_t d) {
  rect(x1, y, x2 - x1 + 1, d);
}

inline void line(int16_t x1, int16_t y1, int16_t x2, int16_t y2) {
  int16_t rx1, ry1, rx2, ry2;
  int16_t dx = x2 - x1; int16_t dy = y2 - y1;
  float k, di;
  int8_t flag;
  if (dx < 0) {
    rx1 = x2;    rx2 = x1;
    ry1 = y2;    ry2 = y1;
    dx = -dx;    dy = -dy;
  } else {
    rx1 = x1;    rx2 = x2;
    ry1 = y1;    ry2 = y2;
  }
  if (abs(dx) > abs(dy)) {
    // x axis is the main direction
    k = ((float)dy) / ((float)dx);
    flag = sgn(k);
    k = abs(k);
    di = 0.5 - k;
    while (rx1 != rx2) {
      pix(rx1, ry1);
      rx1++;
      if (di > 0) {
	di = di - k;
      } else {
	di = di + 1 - k;
	ry1+=flag;
      }
    }
  } else {
    // y axis is the main direction
    k = ((float)dx) / ((float)dy);
    flag = sgn(k);
    k = abs(k);

    di = 0.5 - k;
    while (ry1 != ry2) {
      pix(rx1, ry1);
      ry1++;
      if (di > 0) {
	di = di - k;
      } else {
	di = di + 1 - k;
	rx1+=flag;
      }
    }
  }
}


inline void line_aa(int16_t x1, int16_t y1, int16_t x2, int16_t y2) {
  int16_t rx1, ry1, rx2, ry2;
  uint8_t alpha_store = alpha;
  int16_t dx = x2 - x1; int16_t dy = y2 - y1;
  float k, e;
  int8_t flag;
  if (dx < 0) {
    rx1 = x2;    rx2 = x1;
    ry1 = y2;    ry2 = y1;
    dx = -dx;    dy = -dy;
  } else {
    rx1 = x1;    rx2 = x2;
    ry1 = y1;    ry2 = y2;
  }
  if (abs(dx) > abs(dy)) {
    // x axis is the main direction
    k = ((float)dy) / ((float)dx);
    flag = sgn(k);
    k = abs(k);
    e = 0;
    while (rx1 != rx2) {
      e += k;
      if (e > 1) {
	e = e - 1;
	ry1 += flag;
      }
      alpha = (int)((1 - e) * alpha_store + 0.5f);
      pix(rx1, ry1);
      alpha = (int)(e * alpha_store + 0.5f);
      pix(rx1, ry1 + flag);
      rx1++;
    }
    alpha = alpha_store;
  } else {
    // y axis is the main direction
    k = ((float)dx) / ((float)dy);
    flag = sgn(k);
    k = abs(k);
    e = 0;
    while (ry1 != ry2) {
      e += k;
      if (e > 1) {
	e = e - 1;
	rx1 += flag;
      }
      alpha = (int)((1 - e) * alpha_store + 0.5f);
      pix(rx1, ry1);
      alpha = (int)(e * alpha_store + 0.5f);
      pix(rx1 + flag, ry1);
      ry1++;
    }
    alpha = alpha_store;
  }
}

inline void dashlinev(int16_t x, int16_t y1, int16_t y2,  uint8_t d, uint8_t l, uint8_t s) {
     uint16_t y_next;
     while ((y_next = y1 + l + s) <= y2) {
       rect(x, y1, d, l);
	  y1 = y_next;
     }
     linev(x, y1, y2, d);
}

inline void dashlineh(int16_t x1, int16_t x2, int16_t y, uint8_t d, uint8_t l, uint8_t s) {
     uint16_t x_next;
     while ((x_next = x1 + l + s) <= x2) {
       rect(x1, y, l, d);
	  x1 = x_next;
     }
     lineh(x1, x2, y, d);
}

inline void rectb(int16_t x, int16_t y, uint16_t w, uint16_t h, int8_t d) {
     lineh(x, x + w - 1, y, d);
     lineh(x, x + w - 1, y + h - d, d);
     linev(x, y + d, y + h - d - 1, d);
     linev(x + w - d, y + d, y + h - d - 1, d);
}

inline void rectdb(int16_t x, int16_t y, uint16_t w, uint16_t h, int8_t d, uint8_t l, uint8_t s) {
     dashlineh(x, x + w - 1, y, d, l, s);
     dashlineh(x, x + w - 1, y + h - d, d, l, s);
     dashlinev(x, y + d, y + h - d - 1, d, l, s);
     dashlinev(x + w - d, y + d, y + h - d - 1, d, l, s);
}

inline void circ(int16_t x, int16_t y, uint16_t r) {
  int x0, y0;
  for (int xi = -r; xi <= r; xi++) if ((x0 = x +xi) >= 0 && x0 < TEX_W) 
  for (int yi = -r; yi <= r; yi++) if ((y0 = y +yi) >= 0 && y0 < TEX_H) 
				     if (xi * xi + yi * yi <= r * (r + 1))
	      pix(x0, y0);
}

inline void circb(int16_t x, int16_t y, uint16_t r, uint8_t d) {
  int x0, y0;
  for (int xi = -r; xi <= r; xi++) if ((x0 = x +xi) >= 0 && x0 < TEX_W) 
  for (int yi = -r; yi <= r; yi++) if ((y0 = y +yi) >= 0 && y0 < TEX_H) 
				     if ((xi * xi + yi * yi <= r * (r + 1)) && (xi * xi + yi * yi >= (r - d) * (r - d + 1)))
	      pix(x0, y0);
}

inline void circr(int16_t x, int16_t y, uint16_t r) {
  int16_t xi = 2;
  int16_t yi = r;
  int16_t xend = (int)(r * M_SQRT1_2) + 1;
  float d = 1.25 - r;
  if (r != 1) {
    pix(x, y + r - 1);
    pix(x, y - r + 1);
    pix(x + r - 1, y);
    pix(x - r + 1, y);
  } else {
    pix(x, y);
  }
  while (xi < xend) {
    pix(x + xi - 1, y + yi - 1);
    pix(x + yi - 1, y + xi - 1);
    pix(x - xi + 1, y - yi + 1);
    pix(x - yi + 1, y - xi + 1);
    pix(x + xi - 1, y - yi + 1);
    pix(x + yi - 1, y - xi + 1);
    pix(x - xi + 1, y + yi - 1);
    pix(x - yi + 1, y + xi - 1);
    if (d < 0) {
      d = d + ((int)xi << 1) + 3;
    }
    else {
      d = d + ((int)(xi - yi) << 1) + 5;
      yi--;
    }
    xi++;
  }
}

inline void circr_aa(int16_t x, int16_t y, uint16_t r) {
  int16_t xi = 2;
  int16_t yi = r;
  int16_t xend = (int)(r * M_SQRT1_2) + 1;
  float d = 1.25 - r;
  float e = 0;
  float r2r = 1.0f / (r << 1);
  uint8_t alpha_store = alpha;
  if (r != 1) {
    pix(x, y + r - 1);
    pix(x, y - r + 1);
    pix(x + r - 1, y);
    pix(x - r + 1, y);
  } else {
    pix(x, y);
  }
  while (xi < xend - 1) {
    alpha = (int)(alpha_store * (1 - e) + 0.5f);
    pix(x + xi - 1, y + yi - 1);
    pix(x + yi - 1, y + xi - 1);
    pix(x - xi + 1, y - yi + 1);
    pix(x - yi + 1, y - xi + 1);
    pix(x + xi - 1, y - yi + 1);
    pix(x + yi - 1, y - xi + 1);
    pix(x - xi + 1, y + yi - 1);
    pix(x - yi + 1, y + xi - 1);
    alpha = (int)(alpha_store * e + 0.5f);
    pix(x + xi - 1, y + yi - 2);
    pix(x + yi - 2, y + xi - 1);
    pix(x - xi + 1, y - yi + 2);
    pix(x - yi + 2, y - xi + 1);
    pix(x + xi - 1, y - yi + 2);
    pix(x + yi - 2, y - xi + 1);
    pix(x - xi + 1, y + yi - 2);
    pix(x - yi + 2, y + xi - 1);
    if (d < 0) {
      d = d + ((int)xi << 1) + 3;
    }
    else {
      d = d + ((int)(xi - yi) << 1) + 5;
      yi--;
    }
    xi++;
    e = ((xi << 1) + 1) * r2r;
  }
  alpha = (int)(alpha_store * (1 - e) + 0.5f);
  pix(x + xi - 1, y + yi - 1);
  pix(x + yi - 1, y + xi - 1);
  pix(x - xi + 1, y - yi + 1);
  pix(x - yi + 1, y - xi + 1);
  pix(x + xi - 1, y - yi + 1);
  pix(x + yi - 1, y - xi + 1);
  pix(x - xi + 1, y + yi - 1);
  pix(x - yi + 1, y + xi - 1);
  alpha = (int)(alpha_store * e + 0.5f);
  pix(x + xi - 1, y + yi - 2);
  
  pix(x - xi + 1, y - yi + 2);
  
  pix(x + xi - 1, y - yi + 2);
  
  pix(x - xi + 1, y + yi - 2);
  alpha = alpha_store;
}

inline void ellipse(int16_t x, int16_t y, uint16_t a, uint16_t b) {
  int x0, y0;
  for (int xi = -a; xi <= a; xi++) if ((x0 = x +xi) >= 0 && x0 < TEX_W) 
				     for (int yi = -b; yi <= b; yi++) if ((y0 = y +yi) >= 0 && y0 < TEX_H) {
				     int a_1 = a * (a + 1);
  				     int b_1 = b * (b + 1);
  				     if (xi * xi * b_1 + yi * yi * a_1 <= a_1 * b_1)
				       pix(x0, y0);
				       }
}



inline void ellipseb(int16_t x, int16_t y, uint16_t a, uint16_t b, uint8_t d) {
  int x0, y0;
  for (int xi = -a; xi <= a; xi++) if ((x0 = x +xi) >= 0 && x0 < TEX_W) 
				     for (int yi = -b; yi <= b; yi++) if ((y0 = y +yi) >= 0 && y0 < TEX_H) {
				     int a_1 = a * (a + 1);
  				     int b_1 = b * (b + 1);
				     int a_2 = (a - d) * (a - d + 1);
  				     int b_2 = (b - d) * (b - d + 1);
  				     if ((xi * xi * b_1 + yi * yi * a_1 <= a_1 * b_1) && (xi * xi * b_2 + yi * yi * a_2 >= a_2 * b_2))
				       pix(x0, y0);
				       }
}

inline void ellipser(int16_t x, int16_t y, uint16_t a, uint16_t b) {
  //    x = aa/sqrt(aa+bb)
  //    y = bb/sqrt(aa+bb)
  uint16_t xi, yi;
  uint16_t aa, bb;
  int bbx, aay;
  float d;
  if (a > b) {
    d = b * b + a * a * (0.25f - b);
    xi = 2; yi = b;
    bbx = ((b * b) << 1);
    aay = a * a * b;
    pix(x, y + b - 1);
    pix(x + a - 1, y);
    pix(x - a + 1, y);
    pix(x, y - b + 1);

    while(bbx < aay) {
      pix(x + xi - 1, y + yi - 1);
      pix(x - xi + 1, y - yi + 1);
      pix(x + xi - 1, y - yi + 1);
      pix(x - xi + 1, y + yi - 1);
      aa = a * a;
      bb = b * b;
      bbx = bb * xi;
      aay = aa * yi;
      if (d < 0) {
	d = d + (bbx << 1) + (bb << 1) + bb;
      } else {
	d = d + (bbx << 1) + (bb << 1) + bb + ((-aay + aa) << 1);
	yi--;
      }
      xi++;
    }
    d = bbx * xi + bbx + bb * 0.5f + aay * yi - (aay << 1) + aa - aa * bb;
    while(yi > 1) {
      pix(x + xi - 1, y + yi - 1);
      pix(x - xi + 1, y - yi + 1);
      pix(x + xi - 1, y - yi + 1);
      pix(x - xi + 1, y + yi - 1);

      aa = a * a;
      bb = b * b;
      bbx = bb * xi;
      aay = aa * yi;
      if (d < 0) {
	d = d + ((bbx + bb) << 1) + ((-aay + aa) << 1) + aa;
	xi++ ;
      } else {
	d = d + ((-aay + aa) << 1) + aa;
      }
      yi--;
    }
  } else {
    pix(x, y + b - 1);
    pix(x + a - 1, y);
    pix(x - a + 1, y);
    pix(x, y - b + 1);

    uint16_t temp = a;
    a = b;
    b = temp;
    
    d = b * b + a * a * (0.25f - b);
    xi = 2; yi = b;
    bbx = ((b * b) << 1);
    aay = a * a * b;

    while(bbx < aay) {
      pix(x + yi - 1, y + xi - 1);
      pix(x - yi + 1, y - xi + 1);
      pix(x + yi - 1, y - xi + 1);
      pix(x - yi + 1, y + xi - 1);
      aa = a * a;
      bb = b * b;
      bbx = bb * xi;
      aay = aa * yi;
      if (d < 0) {
	d = d + (bbx << 1) + (bb << 1) + bb;
      } else {
	d = d + (bbx << 1) + (bb << 1) + bb + ((-aay + aa) << 1);
	yi--;
      }
      xi++;
    }
    d = bbx * xi + bbx + bb * 0.5f + aay * yi - (aay << 1) + aa - aa * bb;
    while(yi > 1) {
      pix(x + yi - 1, y + xi - 1);
      pix(x - yi + 1, y - xi + 1);
      pix(x + yi - 1, y - xi + 1);
      pix(x - yi + 1, y + xi - 1);

      aa = a * a;
      bb = b * b;
      bbx = bb * xi;
      aay = aa * yi;
      if (d < 0) {
	d = d + ((bbx + bb) << 1) + ((-aay + aa) << 1) + aa;
	xi++ ;
      } else {
	d = d + ((-aay + aa) << 1) + aa;
      }
      yi--;
    }
  }
}

inline void round_rect(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t r) {

  clip(x, y, r, r);
  circ(x + r, y + r, r);
  clip(x + w - r, y, r, r);
  circ(x + w - r - 1, y + r, r);
  clip(x, y + h - r, r, r);
  circ(x + r, y + h - r - 1, r);
  clip(x + w - r, y + h - r, r, r);
  circ(x + w - r - 1, y + h - r - 1, r);
  clip_r();

  rect(x + r, y, w - 2 * r, h);
  rect(x, y + r, r , h - 2 * r);
  rect(x + w - r, y + r, r, h - 2 * r);

}


inline void round_rectb(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t r, uint8_t d) {
  clip(x, y, r, r);
  circb(x + r, y + r, r, d);
  clip(x + w - r, y, r, r);
  circb(x + w - r - 1, y + r, r, d);
  clip(x, y + h - r, r, r);
  circb(x + r, y + h - r - 1, r, d);
  clip(x + w - r, y + h - r, r, r);
  circb(x + w - r - 1, y + h - r - 1, r, d);
  clip_r();

  rect(x + r, y, w - 2 * r, d);
  rect(x, y + r, d, h - 2 * r);
  rect(x + r, y + h - d, w - 2 * r, d);
  rect(x + w - d, y + r, d, h - 2 * r); 
}


inline void cls() {
  switch_color();
  for (int x = 0; x < TEX_W; x++)
    for (int y = 0; y < TEX_H; y++)
      pix(x, y);
  switch_color();
}

#define CHAR_W      7
#define CHAR_H     14
extern uint8_t font_data[CHAR_W * CHAR_H * 16 * 6];

inline void text_char(int16_t x, int16_t y, signed char ch) {
  if (ch < 32) return;
  uint8_t row = (ch - 32) / 16;
  uint8_t col = (ch - 32) % 16;
  uint16_t ptr = row * CHAR_W * 16 * CHAR_H + col * CHAR_W;
  for (uint8_t j = 0; j < CHAR_H; j++)
    for (uint8_t i = 0; i < CHAR_W; i++)
      if (font_data[ptr + (j * CHAR_W * 16 + i)]) pix(x + i, y + j);
}


inline void text_str(int16_t x, int16_t y, const char *str) {
    while (*str != 0) {
        text_char(x, y, *(str++));
        x += CHAR_W;
    }
}


inline void text_cen(int16_t x, int16_t y, const char *str) {
    size_t n = strlen(str);
    if (n > 0) text_str(x - (n * CHAR_W / 2), y - (CHAR_H >> 1), str);
}

inline void text_xcen(int16_t x, int16_t y, const char *str)
{
    size_t n = strlen(str);
    if (n > 0) text_str(x - (n * CHAR_W / 2), y, str);
}

inline void direct_copy(int16_t x_f, int16_t y_f, uint8_t buf_f[TEX_H][TEX_W][3], int16_t x_t, int16_t y_t, uint8_t buf_t[TEX_H][TEX_W][3], uint16_t width, uint16_t height) {
  uint16_t i, j; 
  for (i = 0; i < width; i++) {
    for (j = 0; j < height; j++) {
      buf_t[y_t + j][x_t + i][2] = buf_f[y_f + j][x_f + i][2];
      buf_t[y_t + j][x_t + i][1] = buf_f[y_f + j][x_f + i][1];
      buf_t[y_t + j][x_t + i][0] = buf_f[y_f + j][x_f + i][0];
    }
  }
}

#endif
