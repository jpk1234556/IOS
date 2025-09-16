/* syscall_entry.S - Brandon Media OS System Call Entry Point */
.section .text

/* System call entry point (x86_64 SYSCALL instruction) */
.global syscall_entry
.align 16
syscall_entry:
    /* Save user context */
    pushq %rbp
    pushq %rbx
    pushq %r12
    pushq %r13
    pushq %r14
    pushq %r15
    
    /* Save user stack pointer */
    movq %rsp, %rbp
    
    /* Switch to kernel stack */
    /* TODO: Get current process kernel stack */
    /* For now, we'll use the current stack */
    
    /* Prepare syscall parameters structure */
    subq $64, %rsp      /* Allocate space for syscall_params */
    
    /* Fill syscall_params structure */
    movq %rax, 0(%rsp)  /* syscall_num */
    movq %rdi, 8(%rsp)  /* arg0 */
    movq %rsi, 16(%rsp) /* arg1 */
    movq %rdx, 24(%rsp) /* arg2 */
    movq %r10, 32(%rsp) /* arg3 (r10 used instead of rcx for syscall) */
    movq %r8, 40(%rsp)  /* arg4 */
    movq %r9, 48(%rsp)  /* arg5 */
    
    /* Call the C syscall handler */
    movq %rsp, %rdi     /* Pass syscall_params pointer */
    call syscall_handler
    
    /* Clean up stack */
    addq $64, %rsp
    
    /* Restore user context */
    movq %rbp, %rsp
    popq %r15
    popq %r14
    popq %r13
    popq %r12
    popq %rbx
    popq %rbp
    
    /* Return to user mode (sysret) */
    sysretq

/* Enter user mode */
.global enter_user_mode
enter_user_mode:
    /* RDI = entry_point, RSI = stack_pointer */
    
    /* Set up user code segment in RCX */
    movq $0x1B, %rcx    /* User code segment (CPL 3) */
    
    /* Set up user flags in R11 */
    movq $0x202, %r11   /* Enable interrupts, set IOPL=0 */
    
    /* Set up user stack */
    movq %rsi, %rsp
    
    /* Set up user data segments */
    movq $0x23, %rax    /* User data segment (CPL 3) */
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs
    
    /* Jump to user mode using sysret */
    movq %rdi, %rax     /* Entry point in RAX for sysret */
    sysretq

/* Setup MSR registers for syscall/sysret */
.global setup_syscall_msrs
setup_syscall_msrs:
    /* Setup STAR MSR (syscall target) */
    movl $0xC0000081, %ecx  /* IA32_STAR */
    movl $0x00100008, %eax  /* Kernel CS=0x08, User CS=0x10 */
    movl $0x00000000, %edx
    wrmsr
    
    /* Setup LSTAR MSR (syscall entry point) */
    movl $0xC0000082, %ecx  /* IA32_LSTAR */
    movq $syscall_entry, %rax
    movl %eax, %eax
    shrq $32, %rax
    movl %eax, %edx
    movq $syscall_entry, %rax
    wrmsr
    
    /* Setup SFMASK MSR (flags mask) */
    movl $0xC0000084, %ecx  /* IA32_FMASK */
    movl $0x00000200, %eax  /* Mask IF flag */
    movl $0x00000000, %edx
    wrmsr
    
    /* Enable syscall/sysret in EFER */
    movl $0xC0000080, %ecx  /* IA32_EFER */
    rdmsr
    orl $0x00000001, %eax   /* Set SCE bit */
    wrmsr
    
    ret

/* User mode test function */
.global user_mode_test
user_mode_test:
    /* This function runs in user mode */
    
    /* Test system call - write "Hello from user mode!" */
    movq $2, %rax           /* SYS_WRITE */
    movq $1, %rdi           /* STDOUT_FILENO */
    leaq user_test_msg(%rip), %rsi
    movq $user_test_msg_len, %rdx
    syscall
    
    /* Test system call - getpid */
    movq $11, %rax          /* SYS_GETPID */
    syscall
    
    /* Test system call - yield */
    movq $10, %rax          /* SYS_YIELD */
    syscall
    
    /* Test system call - sleep */
    movq $9, %rax           /* SYS_SLEEP */
    movq $1000, %rdi        /* 1 second */
    syscall
    
    /* Infinite loop */
user_loop:
    /* Test periodic output */
    movq $2, %rax           /* SYS_WRITE */
    movq $1, %rdi           /* STDOUT_FILENO */
    leaq user_loop_msg(%rip), %rsi
    movq $user_loop_msg_len, %rdx
    syscall
    
    /* Sleep */
    movq $9, %rax           /* SYS_SLEEP */
    movq $2000, %rdi        /* 2 seconds */
    syscall
    
    jmp user_loop

.section .rodata
user_test_msg:
    .ascii "[USER] Hello from neural user mode interface!\n"
user_test_msg_len = . - user_test_msg

user_loop_msg:
    .ascii "[USER] Neural user process executing...\n"
user_loop_msg_len = . - user_loop_msg