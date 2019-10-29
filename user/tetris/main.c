#include "api.h"
#include "tetris.h"

#include <math.h>
#include <string.h>

#ifndef M_PI
#define M_PI  3.1415926535897932384626433832795
#endif
#ifndef M_SQRT1_2
#define M_SQRT1_2   0.7071067811865476
#endif

#define MINO_W      12

#define MATRIX_X1   ((256 - MATRIX_W * MINO_W) / 2)
#define MATRIX_Y1   (256 - (256 - MATRIX_HV * MINO_W) / 2)
#define MATRIX_X2   (MATRIX_X1 + MATRIX_W * MINO_W)
#define MATRIX_Y2   (MATRIX_Y1 - MATRIX_HV * MINO_W)

static uint8_t buf[256][256][3];

static uint32_t T = 0;

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
    tetro_init();
    memset(matrix, MINO_NONE, sizeof matrix);
    tetris_spawn();
}

void update()
{
    T++;

    static uint32_t b1 = 0;
    uint32_t b0 = buttons();
    if ((b0 & BUTTON_LEFT) && !(b1 & BUTTON_LEFT)) tetris_hor(-1);
    if ((b0 & BUTTON_RIGHT) && !(b1 & BUTTON_RIGHT)) tetris_hor(+1);
    if (b0 & BUTTON_DOWN) tetris_drop();
    if (((b0 & BUTTON_CIR) && !(b1 & BUTTON_CIR)) ||
        ((b0 & BUTTON_UP) && !(b1 & BUTTON_UP)))
        tetris_rotate(+1);
    if ((b0 & BUTTON_CRO) && !(b1 & BUTTON_CRO)) tetris_rotate(-1);
    if ((b0 & BUTTON_SQR) && !(b1 & BUTTON_SQR)) tetris_harddrop();
    if ((b0 & BUTTON_TRI) && !(b1 & BUTTON_TRI)) tetris_hold();
    b1 = b0;

    uint32_t action = tetris_tick();
    if (action & TETRIS_LOCKDOWN) {
        tetris_spawn();
    }
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
static inline void draw_mino(uint8_t row, uint8_t col, uint8_t t)
{
    if (row >= MATRIX_HV) return;
    uint8_t x = MATRIX_X1 + col * MINO_W;
    uint8_t y = MATRIX_Y1 - (row + 1) * MINO_W;
    for (int i = 0; i < MINO_W; i++)
    for (int j = 0; j < MINO_W; j++)
        pix(x + i, y + j,
            MINO_COLOURS[t][0], MINO_COLOURS[t][1], MINO_COLOURS[t][2]);
}

static inline void draw_mino_ghost(uint8_t row, uint8_t col, uint8_t t)
{
    uint8_t x = MATRIX_X1 + col * MINO_W;
    uint8_t y = MATRIX_Y1 - (row + 1) * MINO_W;
    for (int i = 0; i < MINO_W; i++)
    for (int j = 0; j < MINO_W; j++)
        pix(x + i, y + j,
            MINO_COLOURS[t][0] / 2, MINO_COLOURS[t][1] / 2, MINO_COLOURS[t][2] / 2);
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
            draw_mino(i, j, matrix[i][j]);
    }

    // Ghost
    uint8_t ghost_row = tetris_ghost();
    for (int i = 0; i < 4; i++)
        draw_mino_ghost(
            ghost_row + TETRO[drop_type].mino[drop_ori][i][0],
            drop_pos[1] + TETRO[drop_type].mino[drop_ori][i][1],
            drop_type);
    // Dropping
    for (int i = 0; i < 4; i++)
        draw_mino(
            drop_pos[0] + TETRO[drop_type].mino[drop_ori][i][0],
            drop_pos[1] + TETRO[drop_type].mino[drop_ori][i][1],
            drop_type);

    // Hold
    if (hold_type != MINO_NONE)
        for (int i = 0; i < 4; i++)
            draw_mino(
                MATRIX_HV - 1 - TETRO[hold_type].bbsize + TETRO[hold_type].mino[0][i][0],
                -5 + TETRO[hold_type].mino[0][i][1],
                hold_type);

    // Preview
    for (int i = 0; i < 4; i++)     // Preview index
    for (int j = 0; j < 4; j++) {   // Mino index
        uint8_t t = drop_next[(drop_pointer + i) % 14];
        draw_mino(
            MATRIX_HV - 1 - i * 3 - TETRO[t].bbsize + TETRO[t].mino[0][j][0],
            MATRIX_W + 1 + TETRO[t].mino[0][j][1],
            t);
    }
}

void *draw()
{
    memset(buf, 0, sizeof buf);
    draw_matrix();
    return (uint8_t *)buf;
}
