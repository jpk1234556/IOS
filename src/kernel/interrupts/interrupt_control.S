/* interrupt_control.S - Brandon Media OS Interrupt Control Functions */
.section .text

/* Enable interrupts */
.global interrupts_enable
interrupts_enable:
    sti
    ret

/* Disable interrupts */
.global interrupts_disable
interrupts_disable:
    cli
    ret

/* Check if interrupts are enabled */
.global interrupts_enabled
interrupts_enabled:
    pushfq
    popq %rax
    andq $0x200, %rax   /* Test IF flag */
    shrq $9, %rax       /* Move IF to bit 0 */
    ret