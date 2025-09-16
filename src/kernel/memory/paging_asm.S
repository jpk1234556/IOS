/* paging_asm.S - Brandon Media OS Paging Assembly Functions */
.section .text

/* Load CR3 register with new page directory */
.global load_cr3
load_cr3:
    mov %rdi, %cr3
    ret

/* Get current CR3 value */
.global get_cr3
get_cr3:
    mov %cr3, %rax
    ret

/* Flush Translation Lookaside Buffer */
.global flush_tlb
flush_tlb:
    mov %cr3, %rax
    mov %rax, %cr3
    ret

/* Flush single page from TLB */
.global flush_page
flush_page:
    invlpg (%rdi)
    ret

/* Enable paging (set PG bit in CR0) */
.global enable_paging
enable_paging:
    mov %cr0, %rax
    or $0x80000000, %rax  /* Set PG bit */
    mov %rax, %cr0
    ret

/* Disable paging (clear PG bit in CR0) */
.global disable_paging
disable_paging:
    mov %cr0, %rax
    and $0x7FFFFFFF, %rax  /* Clear PG bit */
    mov %rax, %cr0
    ret