/* context.S - Brandon Media OS x86_64 Context Switching */
.section .text

/* Save CPU context to memory */
.global save_context
save_context:
    /* RDI points to cpu_context structure */
    
    /* Save general purpose registers */
    movq %rax, 0(%rdi)      /* rax */
    movq %rbx, 8(%rdi)      /* rbx */
    movq %rcx, 16(%rdi)     /* rcx */
    movq %rdx, 24(%rdi)     /* rdx */
    movq %rsi, 32(%rdi)     /* rsi */
    movq %rdi, 40(%rdi)     /* rdi */
    movq %rbp, 48(%rdi)     /* rbp */
    movq %rsp, 56(%rdi)     /* rsp */
    movq %r8,  64(%rdi)     /* r8 */
    movq %r9,  72(%rdi)     /* r9 */
    movq %r10, 80(%rdi)     /* r10 */
    movq %r11, 88(%rdi)     /* r11 */
    movq %r12, 96(%rdi)     /* r12 */
    movq %r13, 104(%rdi)    /* r13 */
    movq %r14, 112(%rdi)    /* r14 */
    movq %r15, 120(%rdi)    /* r15 */
    
    /* Save RIP (return address on stack) */
    movq (%rsp), %rax
    movq %rax, 128(%rdi)    /* rip */
    
    /* Save RFLAGS */
    pushfq
    popq %rax
    movq %rax, 136(%rdi)    /* rflags */
    
    /* Save CR3 */
    movq %cr3, %rax
    movq %rax, 144(%rdi)    /* cr3 */
    
    /* Save segment registers */
    movw %cs, 152(%rdi)     /* cs */
    movw %ds, 154(%rdi)     /* ds */
    movw %es, 156(%rdi)     /* es */
    movw %fs, 158(%rdi)     /* fs */
    movw %gs, 160(%rdi)     /* gs */
    movw %ss, 162(%rdi)     /* ss */
    
    ret

/* Restore CPU context from memory */
.global restore_context
restore_context:
    /* RDI points to cpu_context structure */
    
    /* Restore CR3 first */
    movq 144(%rdi), %rax    /* cr3 */
    movq %rax, %cr3
    
    /* Restore segment registers */
    movw 152(%rdi), %ax     /* cs - will be restored by iret */
    movw 154(%rdi), %ds     /* ds */
    movw %ax, %ds
    movw 156(%rdi), %es     /* es */
    movw %ax, %es
    movw 158(%rdi), %fs     /* fs */
    movw %ax, %fs
    movw 160(%rdi), %gs     /* gs */
    movw %ax, %gs
    movw 162(%rdi), %ss     /* ss */
    movw %ax, %ss
    
    /* Restore RFLAGS */
    movq 136(%rdi), %rax    /* rflags */
    pushq %rax
    popfq
    
    /* Restore general purpose registers */
    movq 0(%rdi), %rax      /* rax */
    movq 8(%rdi), %rbx      /* rbx */
    movq 16(%rdi), %rcx     /* rcx */
    movq 24(%rdi), %rdx     /* rdx */
    movq 32(%rdi), %rsi     /* rsi */
    movq 48(%rdi), %rbp     /* rbp */
    movq 56(%rdi), %rsp     /* rsp */
    movq 64(%rdi), %r8      /* r8 */
    movq 72(%rdi), %r9      /* r9 */
    movq 80(%rdi), %r10     /* r10 */
    movq 88(%rdi), %r11     /* r11 */
    movq 96(%rdi), %r12     /* r12 */
    movq 104(%rdi), %r13    /* r13 */
    movq 112(%rdi), %r14    /* r14 */
    movq 120(%rdi), %r15    /* r15 */
    
    /* Restore RIP by jumping to it */
    movq 128(%rdi), %rax    /* rip */
    pushq %rax
    
    /* Restore RDI last */
    movq 40(%rdi), %rdi     /* rdi */
    
    /* Jump to restored RIP */
    ret

/* Perform complete context switch */
.global context_switch_asm
context_switch_asm:
    /* RDI = from_context, RSI = to_context */
    
    /* Save current context if from_context is not NULL */
    testq %rdi, %rdi
    jz restore_only
    
    /* Save all registers to from_context */
    movq %rax, 0(%rdi)      /* rax */
    movq %rbx, 8(%rdi)      /* rbx */
    movq %rcx, 16(%rdi)     /* rcx */
    movq %rdx, 24(%rdi)     /* rdx */
    movq %rsi, 32(%rdi)     /* rsi - save before we use it */
    movq %rbp, 48(%rdi)     /* rbp */
    movq %rsp, 56(%rdi)     /* rsp */
    movq %r8,  64(%rdi)     /* r8 */
    movq %r9,  72(%rdi)     /* r9 */
    movq %r10, 80(%rdi)     /* r10 */
    movq %r11, 88(%rdi)     /* r11 */
    movq %r12, 96(%rdi)     /* r12 */
    movq %r13, 104(%rdi)    /* r13 */
    movq %r14, 112(%rdi)    /* r14 */
    movq %r15, 120(%rdi)    /* r15 */
    
    /* Save RIP (return address) */
    movq (%rsp), %rax
    movq %rax, 128(%rdi)    /* rip */
    
    /* Save RFLAGS */
    pushfq
    popq %rax
    movq %rax, 136(%rdi)    /* rflags */
    
    /* Save CR3 */
    movq %cr3, %rax
    movq %rax, 144(%rdi)    /* cr3 */
    
    /* Save segment registers */
    movw %cs, 152(%rdi)     /* cs */
    movw %ds, 154(%rdi)     /* ds */
    movw %es, 156(%rdi)     /* es */
    movw %fs, 158(%rdi)     /* fs */
    movw %gs, 160(%rdi)     /* gs */
    movw %ss, 162(%rdi)     /* ss */
    
    /* Save RDI last */
    movq %rdi, 40(%rdi)     /* rdi */

restore_only:
    /* Now restore to_context (RSI) */
    movq %rsi, %rdi         /* Move to_context to RDI for restore */
    
    /* Restore CR3 */
    movq 144(%rdi), %rax    /* cr3 */
    movq %rax, %cr3
    
    /* Flush TLB */
    movq %cr3, %rax
    movq %rax, %cr3
    
    /* Restore registers except RDI and RSI */
    movq 0(%rdi), %rax      /* rax */
    movq 8(%rdi), %rbx      /* rbx */
    movq 16(%rdi), %rcx     /* rcx */
    movq 24(%rdi), %rdx     /* rdx */
    movq 48(%rdi), %rbp     /* rbp */
    movq 56(%rdi), %rsp     /* rsp */
    movq 64(%rdi), %r8      /* r8 */
    movq 72(%rdi), %r9      /* r9 */
    movq 80(%rdi), %r10     /* r10 */
    movq 88(%rdi), %r11     /* r11 */
    movq 96(%rdi), %r12     /* r12 */
    movq 104(%rdi), %r13    /* r13 */
    movq 112(%rdi), %r14    /* r14 */
    movq 120(%rdi), %r15    /* r15 */
    
    /* Restore RFLAGS */
    movq 136(%rdi), %rax    /* rflags */
    pushq %rax
    popfq
    
    /* Restore RSI and RDI */
    movq 32(%rdi), %rsi     /* rsi */
    movq 40(%rdi), %rax     /* saved rdi */
    
    /* Jump to new RIP */
    movq 128(%rdi), %rcx    /* rip */
    pushq %rcx
    
    movq %rax, %rdi         /* restore rdi */
    ret                     /* Jump to new context */