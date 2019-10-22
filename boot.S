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

_int_fiq:
    b       _reset

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

    # Switch to system mode
    mrs     r0, cpsr
    orr     r0, r0, #0x1f
    msr     cpsr_c, r0
    # Set up stack pointer for system & user mode
    # XXX: Of course this is not valid
    mov     sp, #0x400000

    # Back to supervisor mode
    mrs     r0, cpsr
    bic     r0, r0, #0x1f
    orr     r0, r0, #0x13
    msr     cpsr_c, r0

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

# r0 is the translation table base
.global _enable_mmu
_enable_mmu:
    mov     r1, #0
    # Invalidate d/i/unified caches (ARM ARM p. B6-21)
    mcr     p15, 0, r1, c7, c7, 0
    # Invalidate d/i/unified TLBs (ARM ARM p. B4-45)
    mcr     p15, 0, r1, c8, c7, 0
    # Tell the coprocessor about the table address (ARM ARM p. B4-41/B5-18)
    mcr     p15, 0, r0, c2, c0, 0
    mcr     p15, 0, r0, c2, c0, 1
    # Set domain control access to Manager (ARM ARM p. B4-10/B5-18)
    # Also http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.ddi0360f/CHDGIJFB.html
    mov     r1, #0xf
    mcr     p15, 0, r1, c3, c0, 0
    # Turn on MMU, with i/d caches (ARM ARM p. B3-12/B4-40/B5-18/B6-18)
    mrc     p15, 0, r1, c1, c0, 0
    # Enable MMU & d-cache; B3-12
    orr     r1, r1, #5
    # Enable i-cache; B3-12
    orr     r1, r1, #0x1000
    mcr     p15, 0, r1, c1, c0, 0
    bx      lr

.global _flush_mmu_table
_flush_mmu_table:
    mov     r0, #0
    mcr     p15, 0, r0, c7, c7, 0
    mcr     p15, 0, r0, c8, c7, 0
    bx      lr

# r0 is the desired domain access vector (ARM ARM p. B4-10/B4-42)
.global _set_domain_access
_set_domain_access:
    mcr     p15, 0, r0, c3, c0, 0
    bx      lr

.global _standby
_standby:
    # ARM1176JZF-S does not implement the WFI instruction
    # http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.faqs/ka13332.html
    mov     r0, #0
    mcr     p15, 0, r0, c7, c0, 4
    bx      lr

.global _get_mode
_get_mode:
    mrs     r0, cpsr
    and     r0, #0x1f
    bx      lr

.global _enter_user_mode
_enter_user_mode:
    mov     r1, lr
    mrs     r0, cpsr
    bic     r0, r0, #0x1f
    orr     r0, r0, #0x10
    msr     cpsr_c, r0
    bx      r1

.global syscall
syscall:
    svc     #0
    bx      lr
