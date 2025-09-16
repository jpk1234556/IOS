/* isr.S - Brandon Media OS Interrupt Service Routines (Assembly) */
.section .text

/* IDT flush function */
.global idt_flush
idt_flush:
    lidt (%rdi)
    ret

/* Macro for exception handlers without error code */
.macro isr_no_err num
.global exception_\num
exception_\num:
    pushq $0        /* Dummy error code */
    pushq $\num     /* Interrupt number */
    jmp isr_common_stub
.endm

/* Macro for exception handlers with error code */
.macro isr_err num
.global exception_\num
exception_\num:
    pushq $\num     /* Interrupt number */
    jmp isr_common_stub
.endm

/* Macro for IRQ handlers */
.macro irq num, irq_num
.global irq_\num
irq_\num:
    pushq $0        /* Dummy error code */
    pushq $\irq_num /* IRQ number */
    jmp irq_common_stub
.endm

/* Exception handlers (0-31) */
isr_no_err 0    /* Divide by zero */
isr_no_err 1    /* Debug */
isr_no_err 2    /* NMI */
isr_no_err 3    /* Breakpoint */
isr_no_err 4    /* Overflow */
isr_no_err 5    /* Bounds */
isr_no_err 6    /* Invalid opcode */
isr_no_err 7    /* Device not available */
isr_err    8    /* Double fault */
isr_no_err 9    /* Coprocessor segment overrun */
isr_err    10   /* Invalid TSS */
isr_err    11   /* Segment not present */
isr_err    12   /* Stack fault */
isr_err    13   /* General protection fault */
isr_err    14   /* Page fault */
isr_no_err 15   /* Reserved */

/* Hardware interrupts (32+) */
irq 0, 32   /* Timer */
irq 1, 33   /* Keyboard */
irq 4, 36   /* Serial COM1 */

/* Common exception stub */
isr_common_stub:
    /* Save all registers */
    pushq %rax
    pushq %rbx
    pushq %rcx
    pushq %rdx
    pushq %rsi
    pushq %rdi
    pushq %rbp
    pushq %r8
    pushq %r9
    pushq %r10
    pushq %r11
    pushq %r12
    pushq %r13
    pushq %r14
    pushq %r15

    /* Call C exception handler */
    mov %rsp, %rdi  /* Pass stack pointer as argument */
    call exception_handler

    /* Restore all registers */
    popq %r15
    popq %r14
    popq %r13
    popq %r12
    popq %r11
    popq %r10
    popq %r9
    popq %r8
    popq %rbp
    popq %rdi
    popq %rsi
    popq %rdx
    popq %rcx
    popq %rbx
    popq %rax

    /* Clean up error code and interrupt number */
    addq $16, %rsp
    iretq

/* Common IRQ stub */
irq_common_stub:
    /* Save all registers */
    pushq %rax
    pushq %rbx
    pushq %rcx
    pushq %rdx
    pushq %rsi
    pushq %rdi
    pushq %rbp
    pushq %r8
    pushq %r9
    pushq %r10
    pushq %r11
    pushq %r12
    pushq %r13
    pushq %r14
    pushq %r15

    /* Call C IRQ handler */
    mov %rsp, %rdi  /* Pass stack pointer as argument */
    call irq_handler

    /* Restore all registers */
    popq %r15
    popq %r14
    popq %r13
    popq %r12
    popq %r11
    popq %r10
    popq %r9
    popq %r8
    popq %rbp
    popq %rdi
    popq %rsi
    popq %rdx
    popq %rcx
    popq %rbx
    popq %rax

    /* Clean up error code and IRQ number */
    addq $16, %rsp
    iretq

/* Export handler symbols for IDT setup */
.global divide_error_handler
.global debug_handler
.global nmi_handler
.global breakpoint_handler
.global overflow_handler
.global bounds_handler
.global invalid_opcode_handler
.global device_not_available_handler
.global double_fault_handler
.global invalid_tss_handler
.global segment_not_present_handler
.global stack_fault_handler
.global general_protection_handler
.global page_fault_handler
.global timer_handler
.global keyboard_handler
.global serial_handler

/* Map symbols to actual handlers */
divide_error_handler:           jmp exception_0
debug_handler:                  jmp exception_1
nmi_handler:                    jmp exception_2
breakpoint_handler:             jmp exception_3
overflow_handler:               jmp exception_4
bounds_handler:                 jmp exception_5
invalid_opcode_handler:         jmp exception_6
device_not_available_handler:   jmp exception_7
double_fault_handler:           jmp exception_8
invalid_tss_handler:            jmp exception_10
segment_not_present_handler:    jmp exception_11
stack_fault_handler:            jmp exception_12
general_protection_handler:     jmp exception_13
page_fault_handler:             jmp exception_14
timer_handler:                  jmp irq_0
keyboard_handler:               jmp irq_1
serial_handler:                 jmp irq_4