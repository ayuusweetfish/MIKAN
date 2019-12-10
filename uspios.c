#include "common.h"
#include "uspi/assert.h"

#define DMB() __asm__ __volatile__ ("mcr p15, 0, %0, c7, c10, 5" : : "r" (0) : "memory")

static uint8_t heap[1 << 19] __attribute__((section(".bss.dmem")));
static uint32_t ptr = 0;

void uspi_EnterCritical (void);
void uspi_LeaveCritical (void);

void *malloc (unsigned nSize)
{
    uspi_EnterCritical();
    // TODO: Make this work with interrupt contexts
    void *ret = (void *)(heap + ptr);
    nSize <<= 4;
    ptr += nSize;
    uspi_LeaveCritical();
    DMB(); DSB();
    //LogWrite("malloc", LOG_DEBUG, "(%d) Total allocated %d bytes - %x", nSize, ptr, ret);
    DMB(); DSB();
    return ret;
}

void free (void *pBlock)
{
}

static uint8_t heapn[1 << 19] __attribute__((section(".bss.normal")));
static uint32_t ptrn = 0;

void *ampi_malloc(unsigned nSize)
{
    uspi_EnterCritical();
    void *ret = (void *)(heapn + ptrn);
    nSize <<= 4;
    ptrn += nSize;
    uspi_LeaveCritical();
    DMB(); DSB();
    return ret;
}
void ampi_free(void *p) { }

static uint8_t coh[512 * 1024] __attribute__((section(".bss.dmem"), aligned(4096)));

void *GetCoherentRegion512K()
{
    return coh;
}

void MsDelay (unsigned nMilliSeconds)
{
    usDelay(nMilliSeconds * 1000);
}        

void usDelay (unsigned nMicroSeconds)
{
    DSB(); DMB();
    uint32_t val = *SYSTMR_CLO + nMicroSeconds;
    DSB(); DMB();
    while (*SYSTMR_CLO < val) { }
    DSB(); DMB();
    //for (unsigned i = 0; i < 350 * nMicroSeconds; i++) __asm__ __volatile__ ("");
}

#define MAX_TIMERS  128
static struct qwq_timer_t {
    TKernelTimerHandler *handler;
    void *arg1, *arg2;
    uint32_t trigger;
} timers[MAX_TIMERS] = {{ 0 }};
static uint8_t timer_count = 0;
static volatile uint8_t timer_tick = 0;

// Handle-to-index map
// TODO: Implement later
//static uint8_t timer_index[MAX_TIMERS];
//static uint8_t timer_avail_handles[MAX_TIMERS];

unsigned StartKernelTimer (unsigned             nHzDelay,
                           TKernelTimerHandler *pHandler,
                           void *pParam, void *pContext)
{
    /*
    if (timer_count == MAX_TIMERS) {
        LogWrite("timer", LOG_ERROR, "Too many timers");
        return 0;
    }
    timers[timer_count].handler = pHandler;
    timers[timer_count].arg1 = pParam;
    timers[timer_count].arg2 = pContext;
    timers[timer_count].trigger = timer_tick + nHzDelay;
    LogWrite("timer", LOG_DEBUG, "Registered %u (after %u)", timer_count + 1, nHzDelay);
    return ++timer_count;
    */
}

void CancelKernelTimer (unsigned hTimer)
{
    /*
    if (hTimer <= 0 || hTimer > timer_count) {
        LogWrite("timer", LOG_DEBUG, "Invalid cancellation %u", hTimer);
        return;
    }
    LogWrite("timer", LOG_DEBUG, "Cancelled %u", hTimer);
    timers[hTimer - 1].handler = NULL;
    */
}

TPeriodicTimerHandler *periodic_handler = NULL;

static void timer2_handler(void *_unused)
{
    DMB(); DSB();
    do *SYSTMR_CS = 4; while (*SYSTMR_CS & 4);
    uint32_t t = *SYSTMR_CLO;
    t = t - t % (1000000 / HZ) + (1000000 / HZ);
    *SYSTMR_C2 = t;

    DMB(); DSB();
    timer_tick++;
    for (uint8_t i = 0; i < timer_count; i++)
        if (timers[i].handler && (int32_t)(timers[i].trigger - timer_tick) <= 0) {
            TKernelTimerHandler *h = timers[i].handler;
            timers[i].handler = NULL;
            LogWrite("timer", LOG_DEBUG, "Called %u", i + 1);
            h(i + 1, timers[i].arg1, timers[i].arg2);
        }

    if (periodic_handler) periodic_handler();
    AMPiPoke();
}

void RegisterPeriodicHandler (TPeriodicTimerHandler *pHandler)
{
    // XXX: Why it works without this!!
    //periodic_handler = pHandler;
}

void uspios_init()
{
    /*
    DSB();
    *SYSTMR_CS = 4;
    *SYSTMR_C2 = 3000000;
    DMB(); DSB();
    *INT_IRQENAB1 = 4;
    DMB(); DSB();
    set_irq_handler(2, timer2_handler, NULL);
    DMB();
    */
}

void ConnectInterrupt (unsigned nIRQ, TInterruptHandler *pHandler, void *pParam)
{
    set_irq_handler(nIRQ, pHandler, pParam);
}

int SetPowerStateOn (unsigned nDeviceId)
{
    static mbox_buf(8) buf __attribute__((section(".bss.dmem"), aligned(16)));
    mbox_init(buf);
    buf.tag.id = 0x28001;   // Set power state
    buf.tag.u32[0] = nDeviceId;
    buf.tag.u32[1] = 3;     // on | wait
    mbox_emit(buf);
    if ((buf.tag.u32[1] & 1) != 1) {
        LogWrite("SetPowerStateOn", LOG_ERROR,
            "Mailbox response has flags %d instead of 3, for device %d",
            buf.tag.u32[1], nDeviceId);
        return 0;
    }
    LogWrite("SetPowerStateOn", LOG_DEBUG,
        "Device ID: %u; returned state: %u", nDeviceId, buf.tag.u32[1]);
    return 1;
}

int GetMACAddress (unsigned char Buffer[6])
{
    static mbox_buf(6) buf __attribute__((section(".bss.dmem"), aligned(16)));
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

uint32_t EnableVCHIQ (uint32_t p)
{
    static mbox_buf(4) buf __attribute__((section(".bss.dmem"), aligned(16)));
    mbox_init(buf);
    buf.tag.id = 0x48010;   // Enable VCHIQ
    buf.tag.u32[0] = p;
    mbox_emit(buf);
    printf("EnableVCHIQ returns %d\n", buf.tag.u32[0]);
    return buf.tag.u32[0];
}

void LogWrite (const char *pSource,
               unsigned    Severity,
               const char *pMessage, ...)
{
    uspi_EnterCritical();
    printf("[%c] %s: ", "!EWND"[Severity], pSource ? pSource : "undef");

    va_list arglist;
    va_start(arglist, pMessage);
    vprintf(pMessage, arglist);
    va_end(arglist);

    _putchar('\n');
    uspi_LeaveCritical();
}

void uspi_assertion_failed (const char *pExpr, const char *pFile, unsigned nLine)
{
    LogWrite("assert", LOG_ERROR,
        "Assertion failed: %s (%s:%d)", pExpr, pFile, nLine);
    while (1) { }
}

void ampi_assertion_failed (const char *pExpr, const char *pFile, unsigned nLine)
{
    uspi_assertion_failed(pExpr, pFile, nLine);
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
