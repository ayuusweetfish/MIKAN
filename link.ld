SECTIONS
{
    . = 0x8000;
    PROVIDE(__executable_start = 0x8000);

    .text.startup : { *(.text.startup) }
    .text : { *(.text) }
    .rodata : { *(.rodata) }
    .data : { *(.data) }
    . = ALIGN(0x8);
    _bss_begin = .;
    .bss : { *(.bss) }
    _bss_end = .;
    . = ALIGN(0x100000);
    _bss_dmem_begin = .;
    .bss.dmemqwq : { *(.bss.dmem) }
    _bss_dmem_end = .;
    . = ALIGN(0x100000);
    _bss_normal_begin = .;
    .bss.normalqwq : { *(.bss.normal) }
    _bss_normal_end = .;
}
