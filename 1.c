#include <stdint.h>

#define GPIO_BASE   0x20200000

// Act LED is GPIO 47
#define GPFSEL4     (volatile uint32_t *)(GPIO_BASE + 0x10)
#define GPSET1      (volatile uint32_t *)(GPIO_BASE + 0x20)
#define GPCLR1      (volatile uint32_t *)(GPIO_BASE + 0x2c)

uint32_t COUNT = 3000000;
const uint32_t FLAG = 1 << 15;

uint32_t flag() __attribute__((noinline, optimize(0)));
uint32_t flag() { return FLAG; }

void on() __attribute__((noinline));
void on() { *GPCLR1 = flag(); }

int kernel_main()
{
    *GPFSEL4 |= (1 << 21);

    while (1) {
        on();
        for (uint32_t t = 0; t < COUNT; t++) __asm__("");
        *GPSET1 = FLAG;
        for (uint32_t t = 0; t < COUNT; t++) __asm__("");
    }
}
