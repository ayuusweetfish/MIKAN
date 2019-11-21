#ifndef __GRAPHICS2D_API_H___
#define __GRAPHICS2D_API_H___
#include "graphics2d.h"
extern inline void clip_r();
extern inline void clip(int16_t x, int16_t y, uint16_t w, uint16_t h);
extern inline void writemask(bool b);
extern inline void usemask(bool b);

extern inline void switch_color();
extern inline void switch_alpha();

extern inline void pix_rgba(int16_t x, int16_t y, uint8_t r, uint8_t g, uint8_t b, uint8_t a);
extern inline void pix_rgb(int16_t x, int16_t y, uint8_t r, uint8_t g, uint8_t b);
extern inline void pix(int16_t x, int16_t y);
extern inline void pix_polar(float theta, float r, int16_t x, int16_t y);

extern inline void rect(int16_t x, int16_t y, uint16_t w, uint16_t h);

extern inline void rect_cen(int16_t x, int16_t y, uint8_t half_w, uint8_t half_h);

extern inline void linev(int16_t x, int16_t y1, int16_t y2, uint8_t d);

extern inline void lineh(int16_t x1, int16_t x2, int16_t y, uint8_t d);

extern inline void line(int16_t x1, int16_t x2, int16_t y1, int16_t y2);

extern inline void line_aa(int16_t x1, int16_t y1, int16_t x2, int16_t y2);

extern inline void dashlinev(int16_t x, int16_t y1, int16_t y2, uint8_t d, uint8_t l, uint8_t s);

extern inline void dashlineh(int16_t x1, int16_t x2, int16_t y, uint8_t d, uint8_t l, uint8_t s);

extern inline void rectb(int16_t x, int16_t y, uint16_t w, uint16_t h, int8_t d);

extern inline void rectdb(int16_t x, int16_t y, uint16_t w, uint16_t h, int8_t d, uint8_t l, uint8_t s);


extern inline void circ(int16_t x, int16_t y, uint16_t r);

extern inline void circb(int16_t x, int16_t y, uint16_t r, uint8_t d);

extern inline void circr(int16_t x, int16_t y, uint16_t r);

extern inline void ellipse(int16_t x, int16_t y, uint16_t a, uint16_t b);

extern inline void ellipseb(int16_t x, int16_t y, uint16_t a, uint16_t b, uint8_t d);


extern inline void ellipser(int16_t x, int16_t y, uint16_t a, uint16_t b);

extern inline void round_rect(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t r);

extern inline void round_rectb(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t r, uint8_t d);

extern inline void cls();

extern inline void text_char(int16_t x, int16_t y, signed char ch);

extern inline void text_str(int16_t x, int16_t y, const char *str);

extern inline void text_cen(int16_t x, int16_t y, const char *str);

extern inline void text_xcen(int16_t x, int16_t y, const char *str);


#endif
