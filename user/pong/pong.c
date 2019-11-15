#include "api.h"
#include <math.h>

#ifndef M_PI
#define M_PI  3.1415926535897932384626433832795
#endif
#ifndef M_SQRT1_2
#define M_SQRT1_2   0.7071067811865476
#endif

#define CHAR_W 7
#define CHAR_H 14

static uint8_t font_data[CHAR_W * CHAR_H * 16 * 6];

#define WIN_W 400
#define WIN_H 240

static uint8_t buf[WIN_H][WIN_W][3];

static inline void pix(uint8_t x, uint8_t y, uint8_t r, uint8_t g, uint8_t b)
{
  buf[y][x][0] = r;
  buf[y][x][1] = g; 
  buf[y][x][3] = b;
}


static struct pongball {
  uint16_t x, y;
  uint8_t r, g, b;
  float vx, vy;
  uint8_t radius;
} ball; 

static inline uint32_t mrand()
{
    static uint32_t seed = 20191115;
    return (seed = ((seed * 1103515245) + 12345) & 0x7fffffff);
}


#define INIT_VELOVITY = 30

static inline void init_ball()
{
  ball.x = (int)(WIN_W / 2);
  ball.y = (int)(WIN_H / 2);
  int 
  ball.
}
  
static inline void draw_ball()
{
  for (int 
