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

#define MAIL0_CH_FB     1
#define MAIL0_CH_PROP   8

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

uint32_t get_time()
{
    DSB();
    uint32_t val = *SYSTMR_CLO;
    DMB();
    return val;
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

#define mbox_buf(__sz) \
    struct buf_t {              \
        uint32_t size;          \
        uint32_t code;          \
        struct tag_t {          \
            uint32_t id;        \
            uint32_t size;      \
            uint32_t code;      \
            uint32_t val[__sz]; \
        } tag;                  \
        uint32_t end_tag;       \
    } __attribute__((aligned(16)))

uint32_t get_pixel_order()
{
    mbox_buf(1) buf;

    buf.size = sizeof buf;
    buf.code = 0;           // Request
    buf.tag.id = 0x40006;   // Get pixel order
    buf.tag.size = 4;       // Request length
    buf.tag.code = 0;       // Request
    buf.tag.val[0] = 123;   // Don't care
    buf.end_tag = 0;

    send_mail(((uint32_t)&buf) >> 4, MAIL0_CH_PROP);
    recv_mail(MAIL0_CH_PROP);
    return buf.tag.val[0];
}

void kernel_main()
{
    DSB();
    *GPFSEL4 |= (1 << 21);
    DMB();

    murmur(5);

/*
    DSB();
    csudUsbInitialise();
    DMB();
*/

    // Set up framebuffer
    volatile struct fb f_volatile __attribute__((aligned(16))) = { 0 };
    f_volatile.pwidth = 512;
    f_volatile.pheight = 512;
    f_volatile.vwidth = 512;
    f_volatile.vheight = 512;
    f_volatile.bpp = 24;
    send_mail(((uint32_t)&f_volatile + 0x40000000) >> 4, MAIL0_CH_FB);
    recv_mail(MAIL0_CH_FB);

    struct fb f = f_volatile;

    uint8_t *buf = (uint8_t *)(f.buf);
    for (uint32_t y = 0; y < f.vheight; y++)
    for (uint32_t x = 0; x < f.vwidth; x++) {
        buf[y * f.pitch + x * 3 + 2] =
        buf[y * f.pitch + x * 3 + 1] =
        buf[y * f.pitch + x * 3 + 0] = 255;
    }

    DMB();
    print_init(buf, f.vwidth, f.vheight, f.pitch);
    print("Hello world!\nHello MIKAN!\n");
    print("abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz\n\n");
    DSB();

    uint32_t pix_ord = get_pixel_order();
    printf("Pixel order %s\n", pix_ord ? "RGB" : "BGR");

    while (1) { }

/*
    while (0) {
        print_putchar('\r');
        DMB();
        csudUsbCheckForChange();
        DSB();
    }

    uint32_t r = 255, g = 255, b = 255;
    uint32_t seed = 4481192 + 415092;
    uint32_t frm = 0, t0 = get_time(), t;
    while (1) {
        for (uint32_t y = f.vheight - 1; y != UINT32_MAX; y--)
        for (uint32_t x = 0; x < f.vwidth; x++) {
            buf[y * f.pitch + x * 3 + 2] = r;
            buf[y * f.pitch + x * 3 + 1] = g;
            buf[y * f.pitch + x * 3 + 0] = b;
        }
        seed = ((seed * 1103515245) + 12345) & 0x7fffffff;
        r = (r == 255 ? r - 1 : (r == 144 ? r + 1 : r + ((seed >> 0) & 2) - 1));
        g = (g == 255 ? g - 1 : (g == 144 ? g + 1 : g + ((seed >> 1) & 2) - 1));
        b = (b == 255 ? b - 1 : (b == 144 ? b + 1 : b + ((seed >> 2) & 2) - 1));
        t = get_time() - t0;
        frm++;
        printf("\rT=%d, F=%d, FPS=%d", t / 1000000, frm, frm * 1000000 / t);
    }
*/
}
