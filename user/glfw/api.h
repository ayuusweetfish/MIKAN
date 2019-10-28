#ifndef __MIKAN__API_H__
#define __MIKAN__API_H__

#include <stdint.h>

#define BUTTON_UP       (1 << 0)
#define BUTTON_DOWN     (1 << 1)
#define BUTTON_LEFT     (1 << 2)
#define BUTTON_RIGHT    (1 << 3)
#define BUTTON_A        (1 << 4)
#define BUTTON_B        (1 << 5)
#define BUTTON_X        (1 << 6)
#define BUTTON_Y        (1 << 7)
#define BUTTON_CRO      BUTTON_A
#define BUTTON_CIR      BUTTON_B
#define BUTTON_SQR      BUTTON_X
#define BUTTON_TRI      BUTTON_Y

typedef void (*simply_fun)();
void register_update(simply_fun update);

void pix(uint8_t x, uint8_t y, uint8_t r, uint8_t g, uint8_t b);
uint32_t buttons();

// Provided by application
void init();
void update();

#endif
