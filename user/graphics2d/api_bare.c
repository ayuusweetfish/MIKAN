#include "api.h"

extern unsigned char _bss_begin;
extern unsigned char _bss_end;

void crt_init()
{
    unsigned char *begin = &_bss_begin, *end = &_bss_end;
    while (begin < end) *begin++ = 0;
}

uint32_t syscall(uint32_t code, uint32_t arg1, uint32_t arg2)
{
    uint32_t ret;
    __asm__ __volatile__ (
        "mov r0, %1\n\t"
        "mov r1, %2\n\t"
        "mov r2, %3\n\t"
        "svc #0\n\t"
        "mov %0, r0\n\t"
        : "=r"(ret)
        : "r"(code), "r"(arg1), "r"(arg2)
        : "r0", "r1", "r2", "r3", "memory");
    return ret;
}

void register_loop(update_func_t update, draw_func_t draw)
{
    syscall(1, (uint32_t)update, (uint32_t)draw);
}

uint32_t buttons()
{
    return syscall(2, 0, 0);
}

int main()
{
    crt_init();

    init();
    register_loop(update, draw);

    syscall(251, 0, 0);
    return 0;
}
