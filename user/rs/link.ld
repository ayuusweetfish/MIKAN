ENTRY(main)

SECTIONS
{
    . = 0x8000;

    .text : { *(.text .text.*) }
    /DISCARD/ : { *(.ARM.exidx.*) }
}
