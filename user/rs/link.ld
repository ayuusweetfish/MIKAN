ENTRY(main)

SECTIONS
{
    . = 0x80000000;

    .text : { *(.text .text.*) }
    /DISCARD/ : { *(.ARM.exidx.*) }
}
