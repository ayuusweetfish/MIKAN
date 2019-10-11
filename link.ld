SECTIONS
{
    . = 0x8000;
    PROVIDE(__executable_start = 0x8000);

    .text.startup : { *(.text.startup) }
    .rodata : { *(.rodata) }
    .data : { *(.data) }
}
