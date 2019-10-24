#ifndef _MIKAN__PRINT_H_
#define _MIKAN__PRINT_H_

#include <stdint.h>

void print_init(uint8_t *buf, uint32_t w, uint32_t h, uint32_t pitch);
void print_setbuf(uint8_t *buf);

void _putchar(char ch);

#endif
