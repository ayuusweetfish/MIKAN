#ifndef __GRAPHICS2D_API_H___
#define __GRAPHICS2D_API_H___
#include "graphics2d.h"
extern inline void clip_r();
extern inline void clip(int16_t x, int16_t y, uint16_t w, uint16_t h);
extern inline void writemask(bool b);
extern inline void usemask(bool b);

extern inline void switch_color();
extern inline void switch_alpha();
extern inline bool is_color(int16_t x, int16_t y);
  
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

extern inline void circr_aa(int16_t x, int16_t y, uint16_t r);

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

extern inline void direct_copy(int16_t x_f, int16_t y_f, uint8_t buf_f[TEX_H][TEX_W][3], int16_t x_t, int16_t y_t, uint8_t buf_t[TEX_H][TEX_W][3], uint16_t width, uint16_t height);

extern inline void transparent_copy(int16_t x_f, int16_t y_f, uint8_t buf_f[TEX_H][TEX_W][3], int16_t x_t, int16_t y_t, uint8_t buf_t[TEX_H][TEX_W][3], uint16_t width, uint16_t height, RGB* transparent_color);

extern inline void polygon(int16_t* x, int16_t* y, uint8_t n);

extern inline void polygonb(int16_t* x, int16_t* y, uint8_t n);

extern inline void tri(int16_t x1, int16_t y1, int16_t x2, int16_t y2, int16_t x3, int16_t y3);

extern inline void trib(int16_t x1, int16_t y1, int16_t x2, int16_t y2, int16_t x3, int16_t y3);

extern inline void add_to_polygon_bucket(ceat_node *node, ceat_node *head);

extern inline void tidy_the_bucket(ceat_node *head, uint8_t y_iter);

extern inline void clear_the_bucket();

extern inline void seedfullfill(int16_t x, int16_t y, uint8_t fill_type);

extern inline void transform_2d(float A[][3], float B[3][3], uint8_t n);

extern inline void transform_3d(float A[][4], float B[4][4], uint8_t n);

extern inline void affine_transform_2d_init(int16_t x1, int16_t y1, int16_t x2, int16_t y2, int16_t x3, int16_t y3,
					    int16_t u1, int16_t v1, int16_t u2, int16_t v2, int16_t u3, int16_t v3);
#endif
