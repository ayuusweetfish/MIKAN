#include <stdint.h>

#define GPIO_BASE   0x20200000

// Act LED is GPIO 47
#define GPFSEL4     (volatile uint32_t *)(GPIO_BASE + 0x10)
#define GPSET1      (volatile uint32_t *)(GPIO_BASE + 0x20)
#define GPCLR1      (volatile uint32_t *)(GPIO_BASE + 0x2c)

#define SYSTMR_BASE 0x20003000

#define SYSTMR_CS   (volatile uint32_t *)(SYSTMR_BASE + 0x00)
#define SYSTMR_CLO  (volatile uint32_t *)(SYSTMR_BASE + 0x04)
#define SYSTMR_C0   (volatile uint32_t *)(SYSTMR_BASE + 0x0c)
#define SYSTMR_C1   (volatile uint32_t *)(SYSTMR_BASE + 0x10)

#define DMB() __asm__ __volatile__ ("mcr p15, 0, %0, c7, c10, 5" : : "r" (0) : "memory")
#define DSB() __asm__ __volatile__ ("mcr p15, 0, %0, c7, c10, 4" : : "r" (0) : "memory")

void wait()
{
    DSB();
    uint32_t val = *SYSTMR_CLO;
    *SYSTMR_C0 = val + 1000000;
    while ((*SYSTMR_CS) & 1) { *SYSTMR_CS = 1; DMB(); DSB(); }
    while (((*SYSTMR_CS) & 1) == 0) { }
    while ((*SYSTMR_CS) & 1) { *SYSTMR_CS = 1; DMB(); DSB(); }
    DMB();
}

void kernel_main()
{
    *GPFSEL4 |= (1 << 21);

    while (1) {
        *GPCLR1 = (1 << 15);
        wait();
        *GPSET1 = (1 << 15);
        wait();
    }
}
