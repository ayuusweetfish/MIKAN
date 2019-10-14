#include <stdint.h>
#include "usbd/usbd.h"
#include "device/hid/keyboard.h"

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

void wait(uint32_t ticks)
{
    DSB();
    uint32_t val = *SYSTMR_CLO;
    *SYSTMR_C0 = val + ticks;
    while ((*SYSTMR_CS) & 1) { *SYSTMR_CS = 1; DMB(); DSB(); }
    while (((*SYSTMR_CS) & 1) == 0) { }
    while ((*SYSTMR_CS) & 1) { *SYSTMR_CS = 1; DMB(); DSB(); }
    DMB();
}

void murmur(uint32_t num)
{
    DSB();
    for (uint32_t i = 0; i < num; i++) {
        *GPCLR1 = (1 << 15);
        wait(200000);
        *GPSET1 = (1 << 15);
        wait(200000);
    }
    DMB();
}

void kernel_main()
{
    DSB();
    *GPFSEL4 |= (1 << 21);
    DMB();

    murmur(5);

    DSB();
    csudUsbInitialise();
    DMB();

    uint32_t last_count = 0;
    while (1) {
        DSB();
        csudUsbCheckForChange();
        uint32_t interval = 1000000;
        if (csudKeyboardCount() != 0) {
            uint32_t a = -csudKeyboardPoll(csudKeyboardGetAddress(0));
            murmur(a + 1);
            wait(1000000);
            uint32_t count = csudKeyboardGetKeyDownCount(csudKeyboardGetAddress(0));
            murmur(count < last_count ? 2 : (count == last_count ? 3 : 4));
            last_count = count;
            interval = 400000;
        }
        DMB();
        DSB();
        *GPCLR1 = (1 << 15);
        wait(interval);
        *GPSET1 = (1 << 15);
        wait(interval);
        DMB();
    }
}
