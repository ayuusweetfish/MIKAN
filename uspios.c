#include "common.h"

#define DMB() __asm__ __volatile__ ("mcr p15, 0, %0, c7, c10, 5" : : "r" (0) : "memory")

void *malloc (unsigned nSize)
{
}

void free (void *pBlock)
{
}

void MsDelay (unsigned nMilliSeconds)
{
    usDelay(nMilliSeconds * 1000);
}        

void usDelay (unsigned nMicroSeconds)
{
    DMB();
    uint32_t val = *SYSTMR_CLO;
    *SYSTMR_C0 = val + nMicroSeconds;
    while ((*SYSTMR_CS) & 1) { *SYSTMR_CS = 1; DMB(); DSB(); }
    while (((*SYSTMR_CS) & 1) == 0) { }
    while ((*SYSTMR_CS) & 1) { *SYSTMR_CS = 1; DMB(); DSB(); }
    DMB();
}

unsigned StartKernelTimer (unsigned             nHzDelay,
                           TKernelTimerHandler *pHandler,
                           void *pParam, void *pContext)
{
}

void CancelKernelTimer (unsigned hTimer)
{
}

void ConnectInterrupt (unsigned nIRQ, TInterruptHandler *pHandler, void *pParam)
{
}

int SetPowerStateOn (unsigned nDeviceId)
{
    static mbox_buf(8) buf __attribute__((section(".bss.dmem")));
    mbox_init(buf);
    buf.tag.id = 0x28001;   // Set power state
    buf.tag.u8[0] = nDeviceId;
    buf.tag.u8[1] = 3;      // on | wait
    mbox_emit(buf);
    if ((buf.tag.u8[1] & 3) != 3) {
        LogWrite("SetPowerStateOn", LOG_ERROR,
            "Mailbox response has flags %d instead of 3, for device %d\n",
            buf.tag.u8[1], nDeviceId);
        return 0;
    }
    return 1;
}

int GetMACAddress (unsigned char Buffer[6])
{
    static mbox_buf(6) buf __attribute__((section(".bss.dmem")));
    mbox_init(buf);
    buf.tag.id = 0x10003;   // Get MAC address
    mbox_emit(buf);
    /*if (buf.tag.size != 6) {
        LogWrite("GetMACAddress", LOG_ERROR,
            "Mailbox response has length %d but expected 6", buf.tag.size);
        return 0;
    }*/
    memcpy(Buffer, (void *)buf.tag.u8, 6);
    return 1;
}

void LogWrite (const char *pSource,
               unsigned    Severity,
               const char *pMessage, ...)
{
    printf("[%c] %s: ", "!EWND"[Severity], pSource ? pSource : "undef");

    va_list arglist;
    va_start(arglist, pMessage);
    vprintf(pMessage, arglist);
    va_end(arglist);

    _putchar('\n');
}

void uspi_assertion_failed (const char *pExpr, const char *pFile, unsigned nLine)
{
}

void DebugHexdump (const void *pBuffer, unsigned nBufLen, const char *pSource)
{
    if (pSource == NULL) pSource = "debug";
    const uint8_t *buf = (const uint8_t *)pBuffer;
    unsigned count = nBufLen;
    while (1) {
        LogWrite(pSource, LOG_DEBUG,
            "%04x: %02x %02x %02x %02x %02x %02x %02x %02x-"
                  "%02x %02x %02x %02x %02x %02x %02x %02x",
            (unsigned)buf & 0xffff,
            (unsigned)buf[0], (unsigned)buf[1], (unsigned)buf[2], (unsigned)buf[3],
            (unsigned)buf[4], (unsigned)buf[5], (unsigned)buf[6], (unsigned)buf[7],
            (unsigned)buf[8], (unsigned)buf[9], (unsigned)buf[10], (unsigned)buf[11],
            (unsigned)buf[12], (unsigned)buf[13], (unsigned)buf[14], (unsigned)buf[15]);
        buf += 16;
        if (count <= 16) break; else count -= 16;
    }
}
