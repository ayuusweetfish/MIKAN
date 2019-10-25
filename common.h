#ifndef __MIKAN__COMMON_H__
#define __MIKAN__COMMON_H__

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "print.h"
#include "printf/printf.h"
#include "uspi.h"
#include "uspios.h"

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
#define SYSTMR_C2   (volatile uint32_t *)(SYSTMR_BASE + 0x14)
#define SYSTMR_C3   (volatile uint32_t *)(SYSTMR_BASE + 0x18)

#define MAIL0_BASE  0x2000b880

#define MAIL0_READ      (volatile uint32_t *)(MAIL0_BASE + 0x00)
#define MAIL0_STATUS    (volatile uint32_t *)(MAIL0_BASE + 0x18)
#define MAIL0_WRITE     (volatile uint32_t *)(MAIL0_BASE + 0x20)

#define MAIL0_CH_FB     1
#define MAIL0_CH_PROP   8

#define ARMTMR_BASE 0x2000b000

#define ARMTMR_LOAD (volatile uint32_t *)(ARMTMR_BASE + 0x400)
#define ARMTMR_VAL  (volatile uint32_t *)(ARMTMR_BASE + 0x404)
#define ARMTMR_CTRL (volatile uint32_t *)(ARMTMR_BASE + 0x408)
#define ARMTMR_IRQC (volatile uint32_t *)(ARMTMR_BASE + 0x40c)

#define INT_BASE    0x2000b000
#define INT_IRQBASPEND  (volatile uint32_t *)(INT_BASE + 0x200)
#define INT_IRQPEND1    (volatile uint32_t *)(INT_BASE + 0x204)
#define INT_IRQENAB1    (volatile uint32_t *)(INT_BASE + 0x210)
#define INT_IRQBASENAB  (volatile uint32_t *)(INT_BASE + 0x218)

#define INT_IRQ_ARMTMR  1

#define DMB() __asm__ __volatile__ ("mcr p15, 0, %0, c7, c10, 5" : : "r" (0) : "memory")
#define DSB() __asm__ __volatile__ ("mcr p15, 0, %0, c7, c10, 4" : : "r" (0) : "memory")

void _enable_int();
void _enable_mmu(uint32_t table_base_addr);
void _set_domain_access(uint32_t control);
void _flush_mmu_table();
void _standby();
uint32_t _get_mode();
void _enter_user_mode();

void syscall(uint32_t code, uint32_t arg);

// Single tag at a time

#define mbox_buf(__sz) \
    volatile struct buf_t {     \
        volatile uint32_t size; \
        volatile uint32_t code; \
        volatile struct tag_t { \
            volatile uint32_t id;       \
            volatile uint32_t size;     \
            volatile uint32_t code;     \
            volatile union {            \
                volatile uint32_t u32[__sz / 4];    \
                volatile uint16_t u16[__sz / 2];    \
                volatile uint8_t u8[__sz];          \
            };                          \
        } tag __attribute__((packed));  \
        volatile uint32_t end_tag;      \
    } __attribute__((aligned(16), packed))

#define mbox_init(__buf)    do {    \
    (__buf).size = sizeof (__buf);  \
    (__buf).code = 0; /* Request */ \
    (__buf).tag.code = 0; /* Request */         \
    (__buf).tag.size = sizeof (__buf).tag.u8;   \
    (__buf).end_tag = 0;            \
} while (0)

#define mbox_emit(__buf)    do {    \
    send_mail(((uint32_t)&(__buf)) >> 4, MAIL0_CH_PROP);    \
    recv_mail(MAIL0_CH_PROP);       \
} while (0)

void send_mail(uint32_t data, uint8_t channel);
uint32_t recv_mail(uint8_t channel);

#endif