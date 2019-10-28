#include "api.h"

static inline void syscall(uint32_t code, uint32_t arg)
{
    __asm__ __volatile__ (
        "mov r0, %0\n\t"
        "mov r1, %1\n\t"
        "svc #0\n\t"
        : : "r"(code), "r"(arg) : "r0", "r1", "memory");
}

void register_update(simply_fun update)
{
}

void pix(uint8_t x, uint8_t y, uint8_t r, uint8_t g, uint8_t b)
{
}

uint32_t buttons()
{
}

int main()
{
    init();

    return 0;
}
