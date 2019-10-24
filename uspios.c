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
}

int GetMACAddress (unsigned char Buffer[6])
{
    // TODO: mailbox tag 0x00010003
    return 0;
}

void LogWrite (const char *pSource,
               unsigned    Severity,
               const char *pMessage, ...)
{
    _putchar('[');
    _putchar("!EWND"[Severity]);
    _putchar(']');
    _putchar(' ');

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
}
