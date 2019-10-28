#include "common.h"
#include <stddef.h>

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
}
