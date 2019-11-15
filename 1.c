#include "common.h"
#include "user/elf/elf.h"

extern unsigned char _bss_dmem_begin;
extern unsigned char _bss_dmem_end;

uint32_t mm_sys[4096] __attribute__((aligned(1 << 14)));
uint32_t mm_user[4096] __attribute__((aligned(1 << 14)));

void mmu_table_section(uint32_t *table, uint32_t vaddr, uint32_t paddr, uint32_t flags)
{
    uint32_t *table_addr = (uint32_t *)((uint8_t *)table + (vaddr >> 18));
    uint32_t table_val = paddr | flags | 2;
    // 2 = Section; see ARM ARM B4-27
    *table_addr = table_val;
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

struct fb f;

static volatile bool new_frame = false;
static volatile uint8_t bufid = 0;
#define BUF_COUNT   4

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
        for (uint32_t i = 0; i < 10000000; i++) __asm__ __volatile__ ("");
        *GPSET1 = (1 << 15);
        for (uint32_t i = 0; i < 10000000; i++) __asm__ __volatile__ ("");
    }
    DMB();
}

uint32_t set_pixel_order(uint32_t val)
{
    static mbox_buf(4) buf __attribute__((section(".bss.dmem"), aligned(16)));
    mbox_init(buf);
    buf.tag.id = 0x00048006;   // Set pixel order
    buf.tag.u32[0] = val;
    mbox_emit(buf);
    return buf.tag.u32[0];
}

uint32_t get_pixel_order()
{
    static mbox_buf(4) buf __attribute__((section(".bss.dmem"), aligned(16)));
    mbox_init(buf);
    buf.tag.id = 0x40006;   // Get pixel order
    buf.tag.u32[0] = 123;
    mbox_emit(buf);
    return buf.tag.u32[0];
}

void set_virtual_offs(uint32_t x, uint32_t y)
{
    static mbox_buf(8) buf __attribute__((section(".bss.dmem"), aligned(16)));
    mbox_init(buf);
    buf.tag.id = 0x48009;   // Set virtual offset
    buf.tag.u32[0] = x;
    buf.tag.u32[1] = y;
    mbox_emit(buf);
}

uint32_t get_clock_rate(uint8_t id)
{
    static mbox_buf(8) buf __attribute__((section(".bss.dmem"), aligned(16)));
    mbox_init(buf);
    buf.tag.id = 0x30002;   // Get clock rate
    buf.tag.u32[0] = id;
    mbox_emit(buf);
    return buf.tag.u32[1];
}

uint32_t set_clock_rate(uint8_t id, uint32_t hz)
{
    static mbox_buf(12) buf __attribute__((section(".bss.dmem"), aligned(16)));
    mbox_init(buf);
    buf.tag.id = 0x38002;   // Set clock rate
    buf.tag.u32[0] = id;
    buf.tag.u32[1] = hz;
    buf.tag.u32[2] = 0;
    mbox_emit(buf);
    return buf.tag.u32[1];
}

uint32_t get_min_clock_rate(uint8_t id)
{
    static mbox_buf(8) buf __attribute__((section(".bss.dmem"), aligned(16)));
    mbox_init(buf);
    buf.tag.id = 0x30007;   // Get min clock rate
    buf.tag.u32[0] = id;
    mbox_emit(buf);
    return buf.tag.u32[1];
}

uint32_t get_max_clock_rate()
{
    static mbox_buf(8) buf __attribute__((section(".bss.dmem"), aligned(16)));
    mbox_init(buf);
    buf.tag.id = 0x30004;   // Get max clock rate
    buf.tag.u32[0] = 3;     // ARM
    mbox_emit(buf);
    return buf.tag.u32[1];
}

void __attribute__((interrupt("UNDEFINED"))) _int_uinstr()
{
    // TODO: Handle bounced instructions from FPU (ARM ARM p. C2-26)
    uint32_t r14;
    __asm__ __volatile__ ("mov %0, r14" : "=g"(r14));
    r14 -= 4;
    _set_domain_access((3 << 2) | 3);
    DSB();
    print_init((uint8_t *)(f.buf + f.pitch * f.pheight * bufid),
        f.pwidth, f.pheight, f.pitch);
    set_virtual_offs(0, bufid * f.pheight);
    printf("Undefined Instruction %x\n", r14);
    DMB();
    while (1) { murmur(2); wait(1000000); }
}

void __attribute__((interrupt("UNDEFINED"))) _int_uhandler()
{
    _set_domain_access((3 << 2) | 3);
    DSB();
    print_init((uint8_t *)(f.buf + f.pitch * f.pheight * bufid),
        f.pwidth, f.pheight, f.pitch);
    set_virtual_offs(0, bufid * f.pheight);
    printf("Undefined Handler\n");
    DMB();
    while (1) { murmur(3); wait(1000000); }
}

typedef void (*update_func_t)();
typedef void *(*draw_func_t)();

static volatile update_func_t _update = NULL;
static volatile draw_func_t _draw = NULL;

#define BUTTON_UP       (1 << 0)
#define BUTTON_DOWN     (1 << 1)
#define BUTTON_LEFT     (1 << 2)
#define BUTTON_RIGHT    (1 << 3)
#define BUTTON_A        (1 << 4)
#define BUTTON_B        (1 << 5)
#define BUTTON_X        (1 << 6)
#define BUTTON_Y        (1 << 7)
#define BUTTON_CRO      BUTTON_A
#define BUTTON_CIR      BUTTON_B
#define BUTTON_SQR      BUTTON_X
#define BUTTON_TRI      BUTTON_Y

static uint32_t _buttons = 0;

uint32_t _int_swi(uint32_t r0, uint32_t r1, uint32_t r2)
{
    _set_domain_access((3 << 2) | 3);
    DMB(); DSB();
    //printf("SVC %u %u %u\n", r0, r1, r2);
    //DMB(); DSB();
    uint32_t ret = 0;
    if (r0 == 1) {
        _update = (update_func_t)r1;
        _draw = (draw_func_t)r2;
    } else if (r0 == 2) {
        ret = _buttons;
    } else if (r0 == 42) {
        *GPCLR1 = r1;
    } else if (r0 == 43) {
        *GPSET1 = r1;
    }
    DMB(); DSB();
    _set_domain_access((1 << 2) | 3);
    return ret;
}

void __attribute__((interrupt("ABORT"))) _int_pfabort()
{
    _set_domain_access((3 << 2) | 3);
    DSB();
    print_init((uint8_t *)(f.buf + f.pitch * f.pheight * bufid),
        f.pwidth, f.pheight, f.pitch);
    set_virtual_offs(0, bufid * f.pheight);
    printf("Prefetch Abort\n");
    DMB();
    while (1) { murmur(5); wait(1000000); }
}

void __attribute__((interrupt("ABORT"))) _int_dabort()
{
    uint32_t lr;
    __asm__ __volatile__ ("mov %0, lr" : "=g"(lr));
    _set_domain_access((3 << 2) | 3);
    DSB();
    print_init((uint8_t *)(f.buf + f.pitch * f.pheight * bufid),
        f.pwidth, f.pheight, f.pitch);
    set_virtual_offs(0, bufid * f.pheight);
    printf("Data Abort at %x\n", lr);
    DMB();
    while (1) {
        for (uint32_t i = 0; i < 10000000; i++) __asm__ __volatile__ ("");
        *GPCLR1 = (1 << 15);
        for (uint32_t i = 0; i < 10000000; i++) __asm__ __volatile__ ("");
        *GPSET1 = (1 << 15);
    }
}

void draw()
{
    static uint32_t r = 255, g = 255, b = 255;
    static uint32_t seed = 4481192 + 415092;
    static uint32_t frm = 0, t0 = UINT32_MAX, t;
    if (t0 == UINT32_MAX) t0 = get_time();

    uint8_t *gbuf = (uint8_t *)f.buf;
    for (uint32_t y = 0; y < f.pheight; y++)
    for (uint32_t x = 0; x < f.pwidth; x++) {
        gbuf[y * f.pitch + x * 3 + 2] =
        gbuf[y * f.pitch + x * 3 + 1] =
        gbuf[y * f.pitch + x * 3 + 0] = 0;
    }
    for (uint32_t y = 0; y < f.pheight; y++)
    for (uint32_t x = 0; x < f.pwidth; x++) {
        gbuf[y * f.pitch + x * 3 + 2] = r;
        gbuf[y * f.pitch + x * 3 + 1] = (x == (frm << 1) % f.pwidth ? 0 : g);
        gbuf[y * f.pitch + x * 3 + 0] = b;
    }
    seed = ((seed * 1103515245) + 12345) & 0x7fffffff;
    r = (r == 255 ? r - 1 : (r == 144 ? r + 1 : r + ((seed >> 0) & 2) - 1));
    g = (g == 255 ? g - 1 : (g == 144 ? g + 1 : g + ((seed >> 1) & 2) - 1));
    b = (b == 255 ? b - 1 : (b == 144 ? b + 1 : b + ((seed >> 2) & 2) - 1));
    t = get_time() - t0;
    frm++;
    print_setbuf(gbuf);
    _putchar('\r');
    _putchar('0' + frm / 10000 % 10);
    _putchar('0' + frm / 1000 % 10);
    _putchar('0' + frm / 100 % 10);
    _putchar('0' + frm / 10 % 10);
    _putchar('0' + frm % 10);
    _putchar(' ');
    _putchar('0' + frm * 1000000 / t / 100);
    _putchar('0' + frm * 1000000 / t / 10 % 10);
    _putchar('0' + frm * 1000000 / t % 10);
}

void status_handler(unsigned int index, const USPiGamePadState *state)
{
    //printf("!!!!\n");
    const uint8_t *report = state->report;
    uint32_t report_len = state->report_len;

    uint32_t btns = 0;
    uint8_t dpad = report[5] & 15, act = report[5] >> 4;
    if (dpad == 7 || dpad <= 1) btns |= BUTTON_UP;
    if (dpad >= 1 && dpad <= 3) btns |= BUTTON_RIGHT;
    if (dpad >= 3 && dpad <= 5) btns |= BUTTON_DOWN;
    if (dpad >= 5 && dpad <= 7) btns |= BUTTON_LEFT;
    if (act & 1) btns |= BUTTON_X;
    if (act & 2) btns |= BUTTON_A;
    if (act & 4) btns |= BUTTON_B;
    if (act & 8) btns |= BUTTON_Y;

    _buttons = btns;
    //printf("%d\r", btns);
    return; // No further

    for (int i = 0; i < report_len; i++) printf(" %02x", report[i]);
    _putchar('\r');
    _putchar('\b');
    _putchar('\b');
    return;

    printf("GP %u", index);
    uint32_t naxes = state->naxes;
    uint32_t nhats = state->nhats;
    uint32_t nbtns = state->nbuttons;
    for (uint32_t i = 0; i < naxes; i++) printf(" %3d", state->axes[i].value); _putchar('|');
    for (uint32_t i = 0; i < nhats; i++) printf(" %x", state->hats[i]); _putchar('|');
    printf(" %04x\r", state->buttons);
}

void load_program(const elf_ehdr *ehdr, const elf_phdr *program)
{
    const char *buf = (const char *)ehdr;
    uint32_t empty_len =
        (program->filesz < program->memsz ? program->memsz - program->filesz : 0);
    memcpy((void *)program->vaddr, buf + program->offs, program->filesz);
    memset((void *)program->vaddr + program->filesz, 0, empty_len);
}

void timer3_handler(void *_unused)
{
    _set_domain_access((3 << 2) | 3);

    do *SYSTMR_CS = 8; while (*SYSTMR_CS & 8);
    uint32_t t = *SYSTMR_CLO;
    t = t - t % 16667 + 16667;
    *SYSTMR_C3 = t;

    if (!new_frame) {
        // Flip
        set_virtual_offs(0, bufid * f.pheight);
        new_frame = true;
        bufid = (bufid + 1) % BUF_COUNT;
    }

    _set_domain_access((1 << 2) | 3);
}

void kernel_main()
{
    DSB();
    *GPFSEL4 |= (1 << 21);
    DMB();

    _enable_int();

    // 60 FPS tick
    DSB();
    *SYSTMR_CS = 8;
    *SYSTMR_C3 = 3000000;
    DMB(); DSB();
    *INT_IRQENAB1 = 8;
    DMB(); DSB();
    set_irq_handler(3, timer3_handler, NULL);
    DMB();

    // Prepare TLB
    for (uint32_t i = 0; i < 4096; i++) {
        mmu_table_section(mm_sys, i << 20, i << 20, (i < 1 ? (8 | 4) : 0));
    }
    // Disable buffering/caching on .bss.dmem(qwq) sections
    uint32_t dmem_start = ((uint32_t)&_bss_dmem_begin) >> 20;
    uint32_t dmem_end = ((uint32_t)&_bss_dmem_end - 1) >> 20;
    for (uint32_t i = dmem_start; i < dmem_end; i++) {
        mmu_table_section(mm_sys, i << 20, i << 20, 0);
    }
    _enable_mmu((uint32_t)mm_sys);

    // Set up framebuffer
    static struct fb f_volatile __attribute__((section(".bss.dmem"), aligned(16))) = { 0 };
    f_volatile.pwidth = 400;
    f_volatile.pheight = 240;
    f_volatile.vwidth = 400;
    f_volatile.vheight = 240 * BUF_COUNT;
    f_volatile.bpp = 24;
    send_mail(((uint32_t)&f_volatile + 0x40000000) >> 4, MAIL0_CH_FB);
    recv_mail(MAIL0_CH_FB);

    f = f_volatile;

    uint8_t *buf = (uint8_t *)(f.buf);
    for (uint32_t y = 0; y < f.vheight; y++)
    for (uint32_t x = 0; x < f.vwidth; x++) {
        buf[y * f.pitch + x * 3 + 2] =
        buf[y * f.pitch + x * 3 + 1] =
        buf[y * f.pitch + x * 3 + 0] = 255;
    }

    uint32_t buf_p = (uint32_t)buf;
    buf_p = (buf_p >> 20) << 20;
    // Region attributes: B4-12
    // Descriptor: B4-27
    // AP = (3 bits << 12), C = 8, B = 4
    for (uint32_t i = 0; i < 4; i++)
        mmu_table_section(mm_sys, buf_p + (i << 20), buf_p + (i << 20), 12);
    _flush_mmu_table();

    DMB();
    print_init(buf, f.pwidth, f.pheight, f.pitch);
    printf("Hello world!\nHello MIKAN!\n");
    printf("abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz\n\n");
    printf("%d %d\n", dmem_start, dmem_end);
    DSB();

    uint32_t ret = set_pixel_order(0);
    if (ret != 0) while (1) { } // !
    uint32_t pix_ord = get_pixel_order();
    printf("Pixel order %s\n", pix_ord ? "RGB" : "BGR");

    for (uint8_t i = 1; i <= 9; i++) {
        printf("Clock %u rate range %u - %u\n", i, get_min_clock_rate(i), get_max_clock_rate(i));
        printf("Current clock rate %u\n", get_clock_rate(i));
    }
    set_clock_rate(3, (get_min_clock_rate(3) + get_max_clock_rate(3)) / 2);
    set_clock_rate(4, 500000000);

    DMB();
    sdInit();
    DSB();
    DMB();
    int32_t i = sdInitCard();
    DSB();
    DMB();
    wait(100000);
    DSB();
    printf("sdInitCard() returns %d\n", i);

    // MBR
    uint8_t carddata[512] = { 0 };
    i = sdTransferBlocks(0x0LL, 1, carddata, 0);
    printf("sdTransferBlocks() returns %d\n", i);
    for (uint32_t j = 432; j < 512; j += 16) {
        printf("\n%3x | ", j);
        for (uint8_t k = 0; k < 16; k++)
            printf("%2x", carddata[j + k]);
    }
    _putchar('\n');

    FATFS fs;
    DIR dir;
    FILINFO finfo;
    FRESULT fr;

    static char path_buf[FF_LFN_BUF * 2 + 10];
    static char appnames[256][FF_LFN_BUF + 1];
    uint8_t appcount = 0;

    fr = f_mount(&fs, "", 1);
    printf("f_mount() returned %d\n", (int32_t)fr);
    fr = f_opendir(&dir, "/app");
    printf("f_opendir() returned %d\n", (int32_t)fr);
    while (1) {
        fr = f_readdir(&dir, &finfo);
        if (fr != FR_OK || finfo.fname[0] == 0) break;
        if (finfo.fattrib & AM_DIR) {
            strcpy(appnames[appcount], finfo.fname);
            appcount++;
        }
    }
    f_closedir(&dir);

    uspios_init();

    uint8_t mac_addr[6];
    GetMACAddress(mac_addr);
    printf("MAC address:\n");
    DebugHexdump(mac_addr, 6, NULL);

    USPiInitialize();
    uint32_t count = USPiGamePadAvailable();
    if (count) USPiGamePadRegisterStatusHandler(status_handler);

    // Stupid application selection interface
    bool selected;
    uint8_t selappidx = 0;
    uint32_t b0 = 0, b1;
reselect:
    selected = false;
    do {
        // Update
        b1 = b0;
        b0 = _buttons;
        if ((b0 & BUTTON_UP) && !(b1 & BUTTON_UP))
            selappidx = (selappidx + appcount - 1) % appcount;
        if ((b0 & BUTTON_DOWN) && !(b1 & BUTTON_DOWN))
            selappidx = (selappidx + 1) % appcount;
        if ((b0 & BUTTON_CRO) && !(b1 & BUTTON_CRO))
            selected = true;
        // Draw
        bufid = (bufid + 1) % BUF_COUNT;
        uint8_t *buf = (uint8_t *)(f.buf + f.pitch * f.pheight * bufid);
        for (uint32_t y = 0; y < f.pheight; y++)
        for (uint32_t x = 0; x < f.pwidth; x++) {
            buf[y * f.pitch + x * 3 + 2] =
            buf[y * f.pitch + x * 3 + 1] =
            buf[y * f.pitch + x * 3 + 0] = 240;
        }
        print_init(buf, f.pwidth, f.pheight, f.pitch);
        printf("\n%u application%s, %u gamepad%s\n------------\n\n",
            appcount, appcount == 1 ? "" : "s",
            count, count == 1 ? "" : "s");
        for (uint8_t i = 0; i != appcount; i++)
            printf("%c  %s\n", (i == selappidx ? '*' : ' '), appnames[i]);
        set_virtual_offs(0, bufid * f.pheight);
        for (uint32_t i = 0; i < 10000000; i++) __asm__ __volatile__ ("");
    } while (!selected);

    // Load selected application
    // TODO: Directly load from file system to memory
    uint8_t *start_file = (uint8_t *)0x5000000;
    snprintf(path_buf, sizeof path_buf, "/app/%s/start", appnames[selappidx]);
    FIL file;
    fr = f_open(&file, path_buf, FA_READ);
    if (fr != FR_OK) {
        printf("\n\n! Cannot open file %s: error %d\n", path_buf, (int32_t)fr);
        wait(3000000);
        goto reselect;
    }
    UINT fsz = f_size(&file);
    UINT bread;
    f_read(&file, start_file, fsz, &bread);
    f_close(&file);
    printf("\nTotal %u (%u) bytes, ready to go\n", bread, fsz);
    wait(1000000);

    // Set domain to 1
    // Set AP = 0b01 (privileged access only) (ARM ARM p. B4-9/B4-27)
    memcpy(mm_user, mm_sys, sizeof mm_user);
    mmu_table_section(mm_user, 0x20000000, 0x20000000, (1 << 5) | (1 << 10));
    mmu_table_section(mm_user, 0x20200000, 0x20200000, (1 << 5) | (1 << 10));
    // Open up enough space for user code
    for (uint32_t i = 0x80000000; i < 0x90000000; i += 0x100000) {
        mmu_table_section(mm_user, i, i - 0x80000000 + 0x1000000, 8 | 4);
    }
    _enable_mmu((uint32_t)mm_user);
    // Client for domain 1, Manager for domain 0

    load_elf(start_file);
    const elf_ehdr *ehdr = (const elf_ehdr *)start_file;

    _set_domain_access((1 << 2) | 3);
    //_enter_user_mode();
    _enter_user_code(ehdr->entry);

    _set_domain_access((3 << 2) | 3);
    //uint32_t T1 = get_time();
    while (1) {
        if (new_frame) {
            // TODO: Optionally skip a frame
            if (_update && _draw) {
                //DMB();
                //_set_domain_access((1 << 2) | 3);
                (*_update)();
                uint8_t *ret = (uint8_t *)(*_draw)();
                //_set_domain_access((3 << 2) | 3);
                emit_dma((void *)(f.buf + f.pitch * f.pheight * bufid),
                    f.pitch, ret, f.pwidth * 3, f.pwidth * 3, f.pheight);
                print_init((uint8_t *)(f.buf + f.pitch * f.pheight * bufid),
                    f.pwidth, f.pheight, f.pitch);
                set_virtual_offs(0, bufid * f.pheight);
                /*uint32_t T0 = get_time();
                printf("%d\n", T1 - T0);
                T1 = T0;*/
                new_frame = false;
            }
        }
    }
}
