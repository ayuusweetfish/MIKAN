.section ".text.startup"

.global _start
_start:
    # Interrupt vector table
    # Will branch to _reset initially due to the first entry
    ldr     pc, _addr_reset
    ldr     pc, _addr_int_uinstr
    ldr     pc, _addr_int_swi
    ldr     pc, _addr_int_pfabort
    ldr     pc, _addr_int_dabort
    ldr     pc, _addr_int_uhandler
    ldr     pc, _addr_int_irq
    ldr     pc, _addr_int_fiq

_addr_reset:        .word   _reset
_addr_int_uinstr:   .word   _int_uinstr
_addr_int_swi:      .word   _int_swi
_addr_int_pfabort:  .word   _int_pfabort
_addr_int_dabort:   .word   _int_dabort
_addr_int_uhandler: .word   _int_uhandler
_addr_int_irq:      .word   _int_irq
_addr_int_fiq:      .word   _int_fiq

_int_uinstr:
_int_swi:
_int_pfabort:
_int_dabort:
_int_uhandler:
_int_fiq:
    b       _loop

_reset:
    # Initialize interrupt vector table
    # Copy instructions and constant values to address 0x0
    mov     r0, #0x8000
    mov     r1, #0x0
    ldmia   r0!, {r2-r9}
    stmia   r1!, {r2-r9}
    ldmia   r0!, {r2-r9}
    stmia   r1!, {r2-r9}

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

.global _enable_int
_enable_int:
    mrs     r0, cpsr
    bic     r0, r0, #0x80
    msr     cpsr_c, r0
    bx      lr

.global _standby
_standby:
    # ARM1176JZF-S does not implement the WFI instruction
    # http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.faqs/ka13332.html
    mov     r0, #0
    mcr     p15, 0, r0, c7, c0, 4
    bx lr
