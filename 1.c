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

void kernel_main()
{
    *GPFSEL4 |= (1 << 21);

    for (int i = 0; i < 3; i++) {
        *GPCLR1 = (1 << 15);
        wait(500000);
        *GPSET1 = (1 << 15);
        wait(500000);
    }

    // Set up framebuffer
    volatile struct fb f __attribute__((aligned(16))) = { 0 };
    f.pwidth = 128;
    f.pheight = 128;
    f.vwidth = 128;
    f.vheight = 128;
    f.bpp = 24;
    send_mail(((uint32_t)&f + 0x40000000) >> 4, MAIL0_CH_FB);
    recv_mail(MAIL0_CH_FB);

    uint8_t *buf = (uint8_t *)(f.buf);
    for (uint32_t y = 0; y < 128; y++)
    for (uint32_t x = 0; x < 128; x++) {
        buf[y * f.pitch + x * 3 + 2] = ((((x + y) & 1) ? 255 : 0) * (256 - x - y)) >> 8;
        buf[y * f.pitch + x * 3 + 1] = (192 * (256 - x - y)) >> 8;
        buf[y * f.pitch + x * 3 + 0] = (108 * (256 - x - y)) >> 8;
    }

    wait(5000000);

    const uint8_t stripes[8][3] = {
        {255, 0, 0},
        {0, 255, 0},
        {0, 0, 255},
        {255, 255, 0},
        {255, 0, 255},
        {0, 255, 255},
        {128, 128, 128},
        {255, 255, 255}
    };
    for (uint32_t y = 0; y < 128; y++)
    for (uint32_t x = 0; x < 128; x++) {
        buf[y * f.pitch + x * 3 + 2] = ((uint16_t)stripes[y >> 4][0] * (128 - x)) >> 7;
        buf[y * f.pitch + x * 3 + 1] = ((uint16_t)stripes[y >> 4][1] * (128 - x)) >> 7;
        buf[y * f.pitch + x * 3 + 0] = ((uint16_t)stripes[y >> 4][2] * (128 - x)) >> 7;
    }

    while (1) {
        *GPCLR1 = (1 << 15);
        wait(200000);
        *GPSET1 = (1 << 15);
        wait(200000);
    }
}
