/* exceptions.c - Brandon Media OS Exception Handlers */
#include <stdint.h>
#include "kernel/interrupts.h"

/* Register structure for interrupt context */
struct registers {
    uint64_t r15, r14, r13, r12, r11, r10, r9, r8;
    uint64_t rbp, rdi, rsi, rdx, rcx, rbx, rax;
    uint64_t int_no, err_code;
    uint64_t rip, cs, rflags, rsp, ss;
} __attribute__((packed));

/* External VGA/Serial functions */
extern void serial_puts(const char *s);
extern void vga_puts(const char *s);

/* Brandon Media OS - Cyberpunk style error messages */
static const char *exception_messages[] = {
    "[SYSTEM ERROR] Division by Zero - Matrix glitch detected",
    "[DEBUG] Debug exception - Neural interface active", 
    "[CRITICAL] Non-Maskable Interrupt - Core breach",
    "[BREAK] Breakpoint - Cyber diagnostic mode",
    "[OVERFLOW] Arithmetic overflow - Data stream corruption",
    "[BOUNDS] Bounds check failed - Memory barrier breach",
    "[INVALID] Invalid opcode - Unknown instruction sequence",
    "[DEVICE] Device not available - Hardware offline",
    "[FATAL] Double fault - System cascade failure",
    "[RESERVED] Coprocessor segment overrun",
    "[TSS] Invalid Task State Segment - Process corruption",
    "[SEGMENT] Segment not present - Virtual space fragmented",
    "[STACK] Stack fault - Memory stack overflow",
    "[PROTECTION] General protection fault - Security breach",
    "[PAGE] Page fault - Virtual memory violation",
    "[RESERVED] Reserved exception"
};

/* Print hex number (simple implementation) */
static void print_hex(uint64_t num) {
    char hex_chars[] = "0123456789ABCDEF";
    char buffer[17] = {0};
    int i = 15;
    
    if (num == 0) {
        serial_puts("0x0");
        return;
    }
    
    while (num > 0 && i >= 0) {
        buffer[i--] = hex_chars[num & 0xF];
        num >>= 4;
    }
    
    serial_puts("0x");
    serial_puts(&buffer[i + 1]);
}

/* Main exception handler */
void exception_handler(struct registers *regs) {
    /* Brandon Media OS cyberpunk error display */
    serial_puts("\n");
    serial_puts("╔═══════════════════════════════════════════════════════════╗\n");
    serial_puts("║                 BRANDON MEDIA OS - SYSTEM ALERT           ║\n"); 
    serial_puts("╠═══════════════════════════════════════════════════════════╣\n");
    
    if (regs->int_no < 16) {
        serial_puts("║ Exception: ");
        serial_puts(exception_messages[regs->int_no]);
        serial_puts("\n");
    } else {
        serial_puts("║ Unknown Exception #");
        print_hex(regs->int_no);
        serial_puts("\n");
    }
    
    serial_puts("║ Error Code: ");
    print_hex(regs->err_code);
    serial_puts("\n");
    
    serial_puts("║ RIP: ");
    print_hex(regs->rip);
    serial_puts("\n");
    
    serial_puts("║ RSP: ");
    print_hex(regs->rsp);
    serial_puts("\n");
    
    serial_puts("║ RAX: ");
    print_hex(regs->rax);
    serial_puts("  RBX: ");
    print_hex(regs->rbx);
    serial_puts("\n");
    
    serial_puts("║ RCX: ");
    print_hex(regs->rcx);
    serial_puts("  RDX: ");
    print_hex(regs->rdx);
    serial_puts("\n");
    
    serial_puts("╚═══════════════════════════════════════════════════════════╝\n");
    
    /* Display on VGA as well */
    vga_puts("[BRANDON MEDIA OS] SYSTEM EXCEPTION - Check serial output");
    
    /* Halt the system */
    serial_puts("System halted. Reboot required.\n");
    for (;;) {
        asm volatile("cli; hlt");
    }
}