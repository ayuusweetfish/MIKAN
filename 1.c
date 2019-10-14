#include <stdint.h>
#include "print.h"
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

#define MAIL0_BASE  0x2000b880

#define MAIL0_READ      (volatile uint32_t *)(MAIL0_BASE + 0x00)
#define MAIL0_STATUS    (volatile uint32_t *)(MAIL0_BASE + 0x18)
#define MAIL0_WRITE     (volatile uint32_t *)(MAIL0_BASE + 0x20)

#define MAIL0_CH_FB 1

#define DMB() __asm__ __volatile__ ("mcr p15, 0, %0, c7, c10, 5" : : "r" (0) : "memory")
#define DSB() __asm__ __volatile__ ("mcr p15, 0, %0, c7, c10, 4" : : "r" (0) : "memory")

void send_mail(uint32_t data, uint8_t channel)
{
    DSB();
    while ((*MAIL0_STATUS) & (1u << 31)) { }
    *MAIL0_WRITE = (data << 4) | (channel & 15);
    DMB();
}

uint32_t recv_mail(uint8_t channel)
{
    DSB();
    do {
        while ((*MAIL0_STATUS) & (1u << 30)) { }
        uint32_t data = *MAIL0_READ;
        if ((data & 15) == channel) {
            DMB();
            return (data >> 4);
        }
    } while (1);
}

struct fb {
    uint32_t pwidth;
    uint32_t pheight;
    uint32_t vwidth;
    uint32_t vheight;
    uint32_t pitch;
    uint32_t bpp;
    uint32_t xoffs;
    uint32_t yoffs;
    uint32_t buf;
    uint32_t size;
};

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

    // Set up framebuffer
    volatile struct fb f __attribute__((aligned(16))) = { 0 };
    f.pwidth = 512;
    f.pheight = 512;
    f.vwidth = 512;
    f.vheight = 512;
    f.bpp = 24;
    send_mail(((uint32_t)&f + 0x40000000) >> 4, MAIL0_CH_FB);
    recv_mail(MAIL0_CH_FB);

    uint8_t *buf = (uint8_t *)(f.buf);
    for (uint32_t y = 0; y < 512; y++)
    for (uint32_t x = 0; x < 512; x++) {
        buf[y * f.pitch + x * 3 + 2] =
        buf[y * f.pitch + x * 3 + 1] =
        buf[y * f.pitch + x * 3 + 0] = 255;
    }

    DMB();
    print_init(buf, f.vwidth, f.vheight, f.pitch);
    print("Hello world!\nHello MIKAN!\n");
    print("abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz\n\n");
    DSB();

    while (1) {
        print_putchar('\r');
        DMB();
        csudUsbCheckForChange();
        DSB();
    }
}
