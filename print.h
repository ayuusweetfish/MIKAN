#ifndef _MIKAN__PRINT_H_
#define _MIKAN__PRINT_H_

#include <stdint.h>

void print_init(uint8_t *buf, uint32_t w, uint32_t h, uint32_t pitch);
void print_setbuf(uint8_t *buf);

void print_putchar(char ch);

void LogPrint(const char *str, uint32_t len);
void LogPrintF(const char *fmt, uint32_t len, ...);
inline void print(const char *str) { LogPrint(str, 0); }
#define printf(__fmt, ...) (LogPrintF(__fmt, sizeof __fmt, __VA_ARGS__))

#endif
