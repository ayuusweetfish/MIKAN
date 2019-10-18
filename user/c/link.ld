ENTRY(main)

SECTIONS
{
    . = 0x80000000;

    .text : { *(.text .text.*) }
    .rodata : { *(.rodata) }
    .data : { *(.data) }
    . = ALIGN(0x8);
    _bss_begin = .;
    .bss : { *(.bss) }
    _bss_end = .;
}
