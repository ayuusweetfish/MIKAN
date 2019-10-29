#ifndef __TETRIS_H__
#define __TETRIS_H__

#include <stdint.h>

#define MATRIX_W    10
#define MATRIX_H    30

extern uint8_t matrix[MATRIX_H][MATRIX_W];
#define MINO_NONE   255
#define MINO_O      0
#define MINO_I      1
#define MINO_T      2
#define MINO_L      3
#define MINO_J      4
#define MINO_S      5
#define MINO_Z      6

typedef struct tetro_type {
    uint8_t bbsize;         // Side length of bounding box
    uint8_t mino[4][4][2];  // The four blocks; [orientation][block index][x/y]
    uint8_t rot[4][5][2];   // Rotation points; [orientation][point index][x/y]
                            // Orientation is NESW
} tetro_type;

extern tetro_type TETRO[7];

void tetro_init();

#endif
