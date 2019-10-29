#include "api.h"

#include <math.h>

#ifndef M_PI
#define M_PI  3.1415926535897932384626433832795
#endif
#ifndef M_SQRT1_2
#define M_SQRT1_2   0.7071067811865476
#endif

static uint32_t buf[256][256];

static inline void pix(uint8_t x, uint8_t y, uint8_t r, uint8_t g, uint8_t b)
{
    buf[y][x] = (r << 16) | (g << 8) | b;
}

static float p0[2];
static float v0[2];
static int p[5][2];
static int T = 0;

static inline void recal_p()
{
    float phase = T * M_PI / 240;
    float r = 32 * (1.5f + sinf(phase * 1.96f));
    float x = r * cosf(phase), y = r * sinf(phase);
    p[0][0] = (int)(p0[0] + 0.5f);
    p[0][1] = (int)(p0[1] + 0.5f);
    p[1][0] = (int)(p0[0] + x); p[1][1] = (int)(p0[1] + y);
    p[2][0] = (int)(p0[0] - y); p[2][1] = (int)(p0[1] + x);
    p[3][0] = (int)(p0[0] - x); p[3][1] = (int)(p0[1] - y);
    p[4][0] = (int)(p0[0] + y); p[4][1] = (int)(p0[1] - x);
}

void init()
{
    p0[0] = p0[1] = 128;
}

void update()
{
    T++;

    uint32_t b = buttons();
    float v1[2] = { 0, 0 };
    if (b & BUTTON_UP) v1[1] += 1;
    if (b & BUTTON_DOWN) v1[1] -= 1;
    if (b & BUTTON_LEFT) v1[0] -= 1;
    if (b & BUTTON_RIGHT) v1[0] += 1;
    if (v1[0] != 0 && v1[1] != 0) {
        v1[0] *= M_SQRT1_2;
        v1[1] *= M_SQRT1_2;
    }

    float v0len = v0[0] * v0[0] + v0[1] * v0[1];
    float v1len = v1[0] * v1[0] + v1[1] * v1[1];
    float delta[2] = { v1[0] - v0[0], v1[1] - v0[1] };
    float rate = (v1len > v0len ? 0.1 : 0.05);
    v0[0] += delta[0] * rate;
    v0[1] += delta[1] * rate;
    p0[0] += v0[0];
    p0[1] += v0[1];
}

void *draw()
{
    recal_p();

    int x0, y0;
    for (int i = 0; i <= 4; i++)
    for (int x = -8; x <= 8; x++) if ((x0 = p[i][0] + x) >= 0 && x0 < 256)
    for (int y = 8; y >= -8; y--) if ((y0 = p[i][1] + y) >= 0 && y0 < 256) {
        if (x * x + y * y > (int)(8.5f * 8.5f)) continue;
        if (y0 > 0) pix(x0, y0 - 1, 128, 96, 32);
        if (x0 < 255) pix(x0 + 1, y0, 128, 96, 32);
        pix(x0, y0, 240, 192, 108);
    }

    return (uint8_t *)buf;
}
