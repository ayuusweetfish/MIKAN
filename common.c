#include "common.h"
#include <stddef.h>

void send_mail(uint32_t data, uint8_t channel)
{
    DMB(); DSB();
    while ((*MAIL0_STATUS) & (1u << 31)) { }
    *MAIL0_WRITE = (data << 4) | (channel & 15);
    DMB(); DSB();
}

uint32_t recv_mail(uint8_t channel)
{
    DMB(); DSB();
    do {
        while ((*MAIL0_STATUS) & (1u << 30)) { }
        uint32_t data = *MAIL0_READ;
        if ((data & 15) == channel) {
            DMB();
            return (data >> 4);
        //} else {
        //    printf("Incorrect channel (expected %u got %u)\n", channel, data & 15);
        }
    } while (1);
}

#define MAX_HANDLERS    32

static irq_handler handlers[MAX_HANDLERS] = { NULL };
static void *args[MAX_HANDLERS] = { NULL };

void set_irq_handler(uint8_t source, irq_handler f, void *arg)
{
    if (source < 0 || source >= MAX_HANDLERS) return;
    handlers[source] = f;
    args[source] = arg;
    DMB(); DSB();
    if (f) *INT_IRQENAB1 = (1 << source);
    else *INT_IRQDISA1 = (1 << source);
    DMB(); DSB();
}

void /*__attribute__((interrupt("IRQ")))*/ _int_irq()
{
    _set_domain_access((3 << 2) | 3);

    DMB(); DSB();
    uint32_t lr;
    __asm__ __volatile__ ("mov %0, r0" : "=r" (lr));

    DMB(); DSB();
    // Check interrupt source
    uint32_t pend_bitmask = *INT_IRQPEND1;
    if (pend_bitmask == 0) {
        // TODO: Take 32~63 and basic set into consideration
        _putchar('?');
        _putchar('\n');
        return;
    }
    uint8_t source = __builtin_ctz(pend_bitmask);

    DMB(); DSB();
    if (0 && source != 2 && source != 9) {
        _putchar('!');
        _putchar('0' + source / 10);
        _putchar('0' + source % 10);
        _putchar(' ');
        static const char *hex = "0123456789abcdef";
        _putchar(hex[(lr >> 28) & 0xf]);
        _putchar(hex[(lr >> 24) & 0xf]);
        _putchar(hex[(lr >> 20) & 0xf]);
        _putchar(hex[(lr >> 16) & 0xf]);
        _putchar(hex[(lr >> 12) & 0xf]);
        _putchar(hex[(lr >> 8) & 0xf]);
        _putchar(hex[(lr >> 4) & 0xf]);
        _putchar(hex[(lr >> 0) & 0xf]);
        _putchar('\n');
    }

    DMB(); DSB();
    if (handlers[source]) (*handlers[source])(args[source]);
    DMB(); DSB();

    _set_domain_access((1 << 2) | 3);
}

void emit_dma(
    void *dst, uint32_t dpitch, void *src, uint32_t spitch,
    uint32_t rowsize, uint32_t nrows)
{
    DMB(); DSB();
    src = (void *)(((uint32_t)src - 0x80000000 + 0x1000000) | 0xc0000000);
    dpitch -= rowsize;
    spitch -= rowsize;
    *DMA_ENABLE = (*DMA_ENABLE) | 2;
    *DMA_1_CS = (1 << 31);
    static uint32_t cblk[8] __attribute__((section(".bss.dmem"), aligned(256)));
    cblk[0] = (1 << 8) | (1 << 4) | (1 << 1);
    cblk[1] = (uint32_t)src;
    cblk[2] = (uint32_t)dst;
    cblk[3] = ((nrows << 16) | rowsize);
    cblk[4] = ((dpitch << 16) | spitch);
    cblk[5] = 0;
    cblk[6] = cblk[7] = 0;
    *DMA_1_CBAD = (uint32_t)cblk | 0xc0000000;
    *DMA_1_CS = 1;
    DMB(); DSB();
}
