#include "tetris.h"

tetro_type TETRO[7] = {
    {
        // O
        .bbsize = 2,
        .spawn = 0,
        .mino = {{{0, 0}, {0, 1}, {1, 0}, {1, 1}}},
        .rot = {
            {{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}},
            {{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}},
            {{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}},
            {{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}},
        }
    }, {
        // I
        .bbsize = 4,
        .spawn = -2,
        .mino = {{{2, 0}, {2, 1}, {2, 2}, {2, 3}}},
        .rot = {
            {{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}},
            {{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}},
            {{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}},
            {{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}},
        }
    }, {
        // T
        .bbsize = 3,
        .spawn = -1,
        .mino = {{{1, 0}, {1, 1}, {1, 2}, {2, 1}}},
        .rot = {
            {{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}},
            {{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}},
            {{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}},
            {{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}},
        }
    }, {
        // L
        .bbsize = 3,
        .spawn = -1,
        .mino = {{{1, 0}, {1, 1}, {1, 2}, {2, 2}}},
        .rot = {
            {{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}},
            {{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}},
            {{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}},
            {{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}},
        }
    }, {
        // J
        .bbsize = 3,
        .spawn = -1,
        .mino = {{{1, 0}, {1, 1}, {1, 2}, {2, 0}}},
        .rot = {
            {{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}},
            {{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}},
            {{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}},
            {{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}},
        }
    }, {
        // S
        .bbsize = 3,
        .spawn = -1,
        .mino = {{{1, 0}, {1, 1}, {2, 1}, {2, 2}}},
        .rot = {
            {{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}},
            {{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}},
            {{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}},
            {{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}},
        }
    }, {
        // Z
        .bbsize = 3,
        .spawn = -1,
        .mino = {{{1, 1}, {1, 2}, {2, 0}, {2, 1}}},
        .rot = {
            {{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}},
            {{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}},
            {{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}},
            {{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}},
        }
    }
};

uint8_t matrix[MATRIX_H][MATRIX_W];

uint8_t drop_type;
uint8_t drop_ori;
uint8_t drop_pos[2];
uint16_t drop_counter;

void tetro_init()
{
    for (uint8_t i = 0; i < 7; i++) {
        uint8_t s = TETRO[i].bbsize;
#define m   TETRO[i].mino
        for (uint8_t j = 0; j < 4; j++) {
            uint8_t x = m[0][j][0];
            uint8_t y = m[0][j][1];
            m[1][j][0] = y;
            m[1][j][1] = x;
            m[2][j][0] = s - 1 - x;
            m[2][j][1] = s - 1 - y;
            m[3][j][0] = s - 1 - y;
            m[3][j][1] = s - 1 - x;
        }
        /*for (uint8_t j = 0; j < 4; j++) {
            printf("Tetrimino %u rotation %u:", i, j);
            for (uint8_t k = 0; k < 4; k++)
                printf(" (%u %u)", m[j][k][0], m[j][k][1]);
            putchar('\n');
        }*/
    }
};

void tetris_spawn()
{
    drop_type = 1;
    drop_ori = 0;
    drop_pos[0] = MATRIX_HV + TETRO[drop_type].spawn;
    drop_pos[1] = (MATRIX_W - TETRO[drop_type].bbsize) / 2;
    tetris_drop();
}

bool tetris_check()
{
    for (uint8_t i = 0; i < 4; i++) {
        uint8_t r = drop_pos[0] + TETRO[drop_type].mino[drop_ori][i][0];
        uint8_t c = drop_pos[1] + TETRO[drop_type].mino[drop_ori][i][1];
        if (r >= MATRIX_H || c >= MATRIX_W || matrix[r][c] != MINO_NONE)
            return false;
    }
    return true;
}

void tetris_lockdown()
{
    for (uint8_t i = 0; i < 4; i++) {
        uint8_t r = drop_pos[0] + TETRO[drop_type].mino[drop_ori][i][0];
        uint8_t c = drop_pos[1] + TETRO[drop_type].mino[drop_ori][i][1];
        if (r < MATRIX_H || c < MATRIX_W) matrix[r][c] = drop_type;
    }
    drop_type = MINO_NONE;
}

bool tetris_drop()
{
    drop_pos[0]--;
    if (!tetris_check()) {
        drop_pos[0]++;
        return false;
    }
    drop_counter = 30;
    return true;
}

bool tetris_hor(int8_t dx)
{
    drop_pos[1] += dx;
    if (!tetris_check()) {
        drop_pos[1] -= dx;
        return false;
    }
    return true;
}

uint8_t tetris_tick()
{
    if (--drop_counter == 0) {
        if (tetris_drop()) {
        } else {
            if (!tetris_check()) return TETRIS_GAMEOVER;
            tetris_lockdown();
            return TETRIS_LOCKDOWN;
        }
    }
    return TETRIS_NONE;
}
