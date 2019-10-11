.section ".text.startup"

.global _start
_start:
    mov     sp, #0x8000
    b       kernel_startup
_loop:
    b       _loop
