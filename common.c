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

#define MAX_HANDLERS    128

static irq_handler handlers[MAX_HANDLERS] = { NULL };
static void *args[MAX_HANDLERS] = { NULL };

void set_irq_handler(uint8_t source, irq_handler f, void *arg)
{
	if (source < 0 || source >= MAX_HANDLERS) return;
	handlers[source] = f;
	args[source] = arg;
	DMB(); DSB();
	if (f) {
		if (source < 32) *INT_IRQENAB1 = (1 << source);
		else if (source < 64) *INT_IRQENAB2 = (1 << (source - 32));
		else if (source < 72) *INT_IRQBASENAB = (1 << (source - 64));
	} else {
		if (source < 32) *INT_IRQDISA1 = (1 << source);
		else if (source < 64) *INT_IRQDISA2 = (1 << (source - 32));
		else if (source < 72) *INT_IRQBASDISA = (1 << (source - 64));
	}
	DMB(); DSB();
}

void _int_irq()
{
	DMB(); DSB();

	// Check interrupt source
	uint32_t pend_base = *INT_IRQBASPEND;
	uint8_t source;
	if (pend_base & (1 << 8)) {
		source = 0 + __builtin_ctz(*INT_IRQPEND1);
	} else if (pend_base & (1 << 9)) {
		source = 32 + __builtin_ctz(*INT_IRQPEND2);
	} else if (pend_base & 0xff) {
		source = 64 + __builtin_ctz(pend_base & 0xff);
	} else {
		// Should not reach here
		DMB(); DSB();
		return;
	}

	if (handlers[source]) (*handlers[source])(args[source]);
	DMB(); DSB();
}


uint8_t ___qwq___unused;

void emit_dma(
    void *dst, uint32_t dpitch, void *src, uint32_t spitch,
    uint32_t rowsize, uint32_t nrows)
{
    DMB(); DSB();
    uint8_t *_dst = dst, *_src = src;
    spitch = 400 * 3;
    rowsize = 400 * 3;
    nrows = 240;
    // XXX: Manually cleaning data cache...
    /*_clean_data_cache();
    for (uint32_t i = 0; i < nrows; i++, _dst += dpitch, _src += spitch)
        for (uint32_t j = 0; j < rowsize; j += 8) ___qwq___unused += _src[j];*/
    for (uint32_t i = 0; i < nrows; i++, _dst += dpitch, _src += spitch)
        for (uint32_t j = 0; j < rowsize; j++) _dst[j] = _src[j];
    return;
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
