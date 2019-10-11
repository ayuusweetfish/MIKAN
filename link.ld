SECTIONS
{
    . = 0x8000;
    PROVIDE(__executable_start = 0x8000);

    .text.startup : { *(.text.startup) }
    .text : { *(.text) }
    .rodata : { *(.rodata) }
    .data : { *(.data) }
}
