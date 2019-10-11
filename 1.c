#include <stdint.h>

#define GPIO_BASE   0x20200000

// Act LED is GPIO 47
#define GPFSEL4     (volatile uint32_t *)(GPIO_BASE + 0x10)
#define GPSET1      (volatile uint32_t *)(GPIO_BASE + 0x20)
#define GPCLR1      (volatile uint32_t *)(GPIO_BASE + 0x2c)

float COUNT = 30000;
const uint32_t FLAG = 1 << 15;
uint32_t ZERO;  // .bss

void kernel_main()
{
    *GPFSEL4 |= (1 << 21);

    while (1) {
        *GPCLR1 = FLAG;
        for (float t = ZERO; t < COUNT; t += 0.01f) __asm__("");
        *GPSET1 = FLAG;
        for (float t = ZERO; t < COUNT; t += 0.01f) __asm__("");
    }
}
