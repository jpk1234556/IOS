/* boot.S - entry symbol and minimal setup; assume GRUB gives us 64-bit entry */
.section .text
.global _start
.extern kmain

_start:
    /* Set up stack pointer from linker symbol _stack_top */
    lea rsp, [_stack_top]

    /* Call kernel main in C */
    call kmain

.hang:
    cli
    hlt
    jmp .hang