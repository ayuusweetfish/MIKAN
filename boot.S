.section ".text.startup"

.global _start
_start:
    # Initialize stack pointer
    mov     sp, #0x8000

    # Enable FPU
    mrc     p15, 0, r0, c1, c0, 2
    # 0x300000 for single precision
    # 0xc00000 for double precision
    orr     r0, r0, #0xf00000
    mcr     p15, 0, r0, c1, c0, 2
    mov     r0, #0x40000000
    fmxr    fpexc, r0

    # Let's rock
    b       kernel_startup

_loop:
    # In case kernel returns O_O
    b       _loop
