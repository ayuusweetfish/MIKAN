#include "api.h"

#include <math.h>
#include <string.h>

#ifndef M_PI
#define M_PI  3.1415926535897932384626433832795
#endif
#ifndef M_SQRT1_2
#define M_SQRT1_2   0.7071067811865476
#endif

#define MATRIX_W    10
#define MATRIX_H    30
#define MATRIX_HV   20
#define MINO_W      12

#define MATRIX_X1   ((256 - MATRIX_W * MINO_W) / 2)
#define MATRIX_Y1   (256 - (256 - MATRIX_HV * MINO_W) / 2)
#define MATRIX_X2   (MATRIX_X1 + MATRIX_W * MINO_W)
#define MATRIX_Y2   (MATRIX_Y1 - MATRIX_HV * MINO_W)

static uint8_t buf[256][256][3];

static uint32_t T = 0;

static uint8_t matrix[MATRIX_H][MATRIX_W];
#define MINO_NONE   255
#define MINO_O      0
#define MINO_I      1
#define MINO_T      2
#define MINO_L      3
#define MINO_J      4
#define MINO_S      5
#define MINO_Z      6

static inline void pix(uint8_t x, uint8_t y, uint8_t r, uint8_t g, uint8_t b)
{
    buf[y][x][0] = r;
    buf[y][x][1] = g;
    buf[y][x][2] = b;
}

static inline void lineh(
    uint8_t x1, uint8_t x2, uint8_t y, uint8_t r, uint8_t g, uint8_t b)
{
    for (; x1 <= x2; x1++) pix(x1, y, r, g, b);
}

static inline void linev(
    uint8_t x, uint8_t y1, uint8_t y2, uint8_t r, uint8_t g, uint8_t b)
{
    for (; y1 <= y2; y1++) pix(x, y1, r, g, b);
}

void init()
{
    memset(matrix, MINO_NONE, sizeof matrix);
    for (int i = 0; i < 7; i++) matrix[0][i] = i;
}

void update()
{
    T++;
}

static const uint8_t MINO_COLOURS[7][3] = {
    {254, 203, 0},
    {0, 159, 218},
    {149, 45, 152},
    {255, 121, 0},
    {0, 101, 189},
    {105, 190, 40},
    {237, 41, 57}
};

// Top-left corner
static inline void draw_mino(uint8_t x, uint8_t y, uint8_t t)
{
    for (int i = 0; i < MINO_W; i++)
    for (int j = 0; j < MINO_W; j++)
        pix(x + i, y + j,
            MINO_COLOURS[t][0], MINO_COLOURS[t][1], MINO_COLOURS[t][2]);
}

static inline void draw_matrix()
{
    for (int i = 0; i <= MATRIX_HV; i++)
        lineh(MATRIX_X1, MATRIX_X2, MATRIX_Y1 - i * MINO_W,
            128, 128, 128);
    for (int j = 0; j <= MATRIX_W; j++)
        linev(MATRIX_X1 + j * MINO_W, MATRIX_Y2, MATRIX_Y1,
            128, 128, 128);

    for (int i = 0; i < MATRIX_HV; i++)
    for (int j = 0; j < MATRIX_W; j++) {
        if (matrix[i][j] != MINO_NONE)
            draw_mino(
                MATRIX_X1 + j * MINO_W,
                MATRIX_Y1 - (i + 1) * MINO_W,
                matrix[i][j]);
    }
}

void *draw()
{
    memset(buf, 0, sizeof buf);
    draw_matrix();
    return (uint8_t *)buf;
}
