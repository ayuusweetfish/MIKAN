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

static inline void pix(uint8_t x, uint8_t y, uint8_t r, uint8_t g, uint8_t b)
{
    buf[y][x][0] = r;
    buf[y][x][1] = g;
    buf[y][x][2] = b;
}

static float p0[2];
static float v0[2];
static int p[5][2], q[5][2];
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
    recal_p();
    memcpy(q, p, sizeof q);
    memset(buf, 0, sizeof buf);
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

static inline void blit(uint8_t i)
{
    int x0, y0;
    for (int x = -8; x <= 8; x++) if ((x0 = q[i][0] + x) >= 0 && x0 < 256)
    for (int y = -8; y <= 8; y++) if ((y0 = q[i][1] + y) >= 0 && y0 < 256) {
        if (x * x + y * y > (int)(8.5f * 8.5f)) continue;

        static const uint32_t button[4] = {
            BUTTON_TRI, BUTTON_CIR, BUTTON_CRO, BUTTON_SQR
        };
        uint8_t r1 = 240, g1 = 192, b1 = 108;
        uint8_t r2 = 128, g2 = 96, b2 = 32;
        if (i == 0) {
            g1 = 144;
            g2 = 48;
        } else if (buttons() & button[i - 1]) {
            r1 = 192, g1 = 240;
            r2 = 96, g2 = 128;
        }
        if (y0 < 255) pix(x0, y0 + 1, r2, g2, b2);
        if (x0 < 255) pix(x0 + 1, y0, r2, g2, b2);
        pix(x0, y0, r1, g1, b1);
    }
}

void *draw()
{
    recal_p();

    for (int i = 0; i <= 4; i++) {
        int sgnx = (p[i][0] < q[i][0] ? -1 : +1);
        int sgny = (p[i][1] < q[i][1] ? -1 : +1);
        do {
            if (p[i][0] != q[i][0]) q[i][0] += sgnx;
            if (p[i][1] != q[i][1]) q[i][1] += sgny;
            blit(i);
        } while (p[i][0] != q[i][0] || p[i][1] != q[i][1]);
    }

    return (uint8_t *)buf;
}
