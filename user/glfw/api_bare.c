#include "api.h"

static inline void syscall(uint32_t code, uint32_t arg)
{
    __asm__ __volatile__ (
        "mov r0, %0\n\t"
        "mov r1, %1\n\t"
        "svc #0\n\t"
        : : "r"(code), "r"(arg) : "r0", "r1", "memory");
}

void register_loop(update_func_t update, draw_func_t draw)
{
}

uint32_t buttons()
{
}

int main()
{
    init();
    register_loop(update, draw);

    return 0;
}
