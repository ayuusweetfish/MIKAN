#include <stdint.h>

extern unsigned char _bss_begin;
extern unsigned char _bss_end;

void crt_init()
{
    unsigned char *begin = &_bss_begin, *end = &_bss_end;
    while (begin < end) *begin++ = 0;
}

void syscall(uint32_t code, uint32_t arg)
{
    __asm__ __volatile__ ("svc #0" : : : "memory");
}

uint32_t main()
{
    crt_init();

    while (1) {
        for (uint32_t i = 0; i < 30000000; i++) __asm__ __volatile__ ("");
        syscall(42, 1 << 15);
        for (uint32_t i = 0; i < 30000000; i++) __asm__ __volatile__ ("");
        syscall(43, 1 << 15);
    }

    return 0;
}
