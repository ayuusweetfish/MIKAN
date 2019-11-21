extern inline void clip_r();
extern inline void clip(int16_t x, int16_t y, uint16_t w, uint16_t h);
extern inline void writemask(bool b);
extern inline void usemask(bool b);
extern inline void pix(uint16_t x, uint16_t y, RGB *color);
extern inline void pix_a(uint16_t x, uint16_t y, RGB *color, uint8_t a);
extern inline void rect(int16_t x, int16_t y, uint16_t w, uint16_t h, RGB *color);
extern inline void rect_a(int16_t x, int16_t y, uint16_t w, uint16_t h, RGB *color, uint8_t a);
extern inline void rect_cen(int16_t x, int16_t y, uint8_t half_w, uint8_t half_h, RGB *color);
extern inline void rect_cen_a(int16_t x, int16_t y, uint8_t half_w, uint8_t half_h, RGB *color, uint8_t a);
extern inline void linev(int16_t x, int16_t y1, int16_t y2, RGB *color, uint8_t d);
extern inline void linev_a(int16_t x, int16_t y1, int16_t y2, RGB *color, uint8_t d, uint8_t a);
extern inline void lineh(int16_t x1, int16_t x2, int16_t y, RGB *color, uint8_t d);
extern inline void lineh_a(int16_t x1, int16_t x2, int16_t y, RGB *color, uint8_t d, uint8_t a);

extern inline void dashlinev(int16_t x, int16_t y1, int16_t y2, RGB *color, uint8_t d, uint8_t l, uint8_t s);
extern inline void dashlinev_a(int16_t x, int16_t y1, int16_t y2, RGB *color, uint8_t d, uint8_t l, uint8_t s, uint8_t a);

extern inline void dashlineh(int16_t x1, int16_t x2, int16_t y, RGB *color, uint8_t d, uint8_t l, uint8_t s);
extern inline void dashlineh_a(int16_t x1, int16_t x2, int16_t y, RGB *color, uint8_t d, uint8_t l, uint8_t s, uint8_t a);

extern inline void rectb(int16_t x, int16_t y, uint16_t w, uint16_t h, RGB *color, int8_t d);
extern inline void rectb_a(int16_t x, int16_t y, uint16_t w, uint16_t h, RGB *color, int8_t d, uint8_t a);

extern inline void rectdb(int16_t x, int16_t y, uint16_t w, uint16_t h, RGB *color, int8_t d, uint8_t l, uint8_t s);
extern inline void rectdb_a(int16_t x, int16_t y, uint16_t w, uint16_t h, RGB *color, int8_t d, uint8_t l, uint8_t s, uint8_t a);

extern inline void circ(int16_t x, int16_t y, uint16_t r, RGB* color);
extern inline void circ_a(int16_t x, int16_t y, uint16_t r, RGB* color, uint8_t a);

extern inline void circb(int16_t x, int16_t y, uint16_t r, RGB* color, uint8_t d);
extern inline void circb_a(int16_t x, int16_t y, uint16_t r, RGB* color, uint8_t d, uint8_t a);

extern inline void ellipse(int16_t x, int16_t y, uint16_t a, uint16_t b, RGB* color);
extern inline void ellipse_a(int16_t x, int16_t y, uint16_t a, uint16_t b, RGB* color, uint8_t alpha);

extern inline void ellipseb(int16_t x, int16_t y, uint16_t a, uint16_t b, RGB* color, uint8_t d);
extern inline void ellipseb_a(int16_t x, int16_t y, uint16_t a, uint16_t b, RGB* color, uint8_t d, uint8_t alpha);

extern inline void round_rect(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t r, RGB* color);
extern inline void round_rect_a(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t r, RGB* color, uint8_t a);

extern inline void round_rectb(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t r, RGB* color, uint8_t d);

extern inline void cls(RGB* color);

