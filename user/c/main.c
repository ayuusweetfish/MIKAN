#include <stdint.h>

extern unsigned char _bss_begin;
extern unsigned char _bss_end;

uint8_t qwq[1024];
uint8_t qvq[] = "=~=";
const uint8_t *quq = "-v-";

uint8_t buf[256][256][3] = {{{ 0 }}};

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

__attribute__((noinline)) void wink(uint32_t delay)
{
    for (uint32_t i = 0; i < delay; i++) __asm__ __volatile__ ("");
    syscall(42, 1 << 15, 0);
    for (uint32_t i = 0; i < delay; i++) __asm__ __volatile__ ("");
    syscall(43, 1 << 15, 0);
}

void update()
{
    uint32_t b = syscall(2, 0, 0);
    for (int i = 0; i < 256; i++)
        buf[127][i][0] = buf[127][i][1] = buf[127][i][2] = 255;
    for (int i = 0; i < 256; i++)
        buf[63][i][0] = buf[63][i][1] = (b ? 128 : 255);
}

void *draw()
{
    return buf;
}

uint32_t main()
{
    crt_init();

    syscall(1, (uint32_t)update, (uint32_t)draw);

    while (1) {
        wink(300000000);
        wink(150000000);
        wink(150000000);
    }

    return 0;
}
