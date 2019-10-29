#include "api.h"

#include <math.h>
#include <string.h>

#ifndef M_PI
#define M_PI  3.1415926535897932384626433832795
#endif
#ifndef M_SQRT1_2
#define M_SQRT1_2   0.7071067811865476
#endif

static uint8_t buf[256][256][3];

static uint32_t T = 0;

static inline void pix(uint8_t x, uint8_t y, uint8_t r, uint8_t g, uint8_t b)
{
    buf[y][x][0] = r;
    buf[y][x][1] = g;
    buf[y][x][2] = b;
}

void init()
{
    memset(buf, 0, sizeof buf);
}

void update()
{
    T++;
}

void *draw()
{
    return (uint8_t *)buf;
}
