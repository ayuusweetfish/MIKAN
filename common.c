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
    uint8_t *_dst = dst, *_src = src;
    for (uint32_t i = 0; i < nrows; i++, _dst += dpitch, _src += spitch)
        for (uint32_t j = 0; j < rowsize; j++) _dst[j] = _src[j];
}
