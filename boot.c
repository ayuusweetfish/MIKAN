extern unsigned char _bss_begin;
extern unsigned char _bss_end;

extern unsigned char _bss_dmem_begin;
extern unsigned char _bss_dmem_end;

extern void kernel_main();

extern void kernel_startup()
{
    unsigned char *begin = &_bss_begin, *end = &_bss_end;
    while (begin < end) *begin++ = 0;

    begin = &_bss_dmem_begin, end = &_bss_dmem_end;
    while (begin < end) *begin++ = 0;

    kernel_main();
}
