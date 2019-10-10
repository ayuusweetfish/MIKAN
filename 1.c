#include <stdint.h>

#define GPIO_BASE   0x20200000

// Act LED is GPIO 47
#define GPFSEL4     (volatile uint32_t *)(GPIO_BASE + 0x10)
#define GPSET1      (volatile uint32_t *)(GPIO_BASE + 0x20)
#define GPCLR1      (volatile uint32_t *)(GPIO_BASE + 0x2c)

volatile uint32_t t;

int main() __attribute((naked));
int main()
{
    *GPFSEL4 |= (1 << 21);

    while (1) {
        *GPCLR1 = (1 << 15);
        for (t = 0; t < 1500000; t++) { }
        *GPSET1 = (1 << 15);
        for (t = 0; t < 1500000; t++) { }
    }
}
