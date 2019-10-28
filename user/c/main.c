#include <stdint.h>

extern unsigned char _bss_begin;
extern unsigned char _bss_end;

uint8_t qwq[1024];
uint8_t qvq[] = "=~=";
const uint8_t *quq = "-v-";

#define GPIO_BASE   0x20200000

// Act LED is GPIO 47
#define GPFSEL4     (volatile uint32_t *)(GPIO_BASE + 0x10)
#define GPSET1      (volatile uint32_t *)(GPIO_BASE + 0x20)
#define GPCLR1      (volatile uint32_t *)(GPIO_BASE + 0x2c)

void crt_init()
{
    unsigned char *begin = &_bss_begin, *end = &_bss_end;
    while (begin < end) *begin++ = 0;
}

void syscall(uint32_t code, uint32_t arg)
{
    __asm__ __volatile__ (
        "mov r0, %0\n\t"
        "mov r1, %1\n\t"
        "svc #0\n\t"
        : : "r"(code), "r"(arg) : "r0", "r1", "memory");
}

__attribute__((noinline)) void wink(uint32_t delay)
{
    for (uint32_t i = 0; i < delay; i++) __asm__ __volatile__ ("");
    syscall(42, 1 << 15);
    for (uint32_t i = 0; i < delay; i++) __asm__ __volatile__ ("");
    syscall(43, 1 << 15);
}

uint32_t main()
{
    crt_init();

    while (1) {
        wink(300000000);
        wink(150000000);
        wink(150000000);
    }

    return 0;
}
