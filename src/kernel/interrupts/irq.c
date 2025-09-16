/* irq.c - Brandon Media OS Hardware Interrupt Handlers */
#include <stdint.h>
#include "kernel/interrupts.h"

/* Register structure for interrupt context */
struct registers {
    uint64_t r15, r14, r13, r12, r11, r10, r9, r8;
    uint64_t rbp, rdi, rsi, rdx, rcx, rbx, rax;
    uint64_t int_no, err_code;
    uint64_t rip, cs, rflags, rsp, ss;
} __attribute__((packed));

/* PIC (Programmable Interrupt Controller) ports */
#define PIC1_COMMAND    0x20
#define PIC1_DATA       0x21
#define PIC2_COMMAND    0xA0
#define PIC2_DATA       0xA1
#define PIC_EOI         0x20    /* End of Interrupt */

/* I/O port functions */
static inline void outb(uint16_t port, uint8_t val) {
    asm volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    asm volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

/* External functions */
extern void serial_puts(const char *s);
extern void serial_putc(char c);
extern void scheduler_tick(void);

/* Global timer tick counter */
static volatile uint64_t timer_ticks = 0;

/* Send End of Interrupt signal */
static void send_eoi(uint8_t irq) {
    if (irq >= 8) {
        outb(PIC2_COMMAND, PIC_EOI);  /* Send EOI to slave PIC */
    }
    outb(PIC1_COMMAND, PIC_EOI);      /* Send EOI to master PIC */
}

/* Initialize PIC (Programmable Interrupt Controller) */
void pic_init(void) {
    /* Save current interrupt masks */
    uint8_t mask1 = inb(PIC1_DATA);
    uint8_t mask2 = inb(PIC2_DATA);
    
    /* Start initialization sequence */
    outb(PIC1_COMMAND, 0x11);  /* ICW1: Initialize + cascade */
    outb(PIC2_COMMAND, 0x11);
    
    /* ICW2: Set interrupt vector offsets */
    outb(PIC1_DATA, 32);       /* Master PIC starts at IRQ 32 */
    outb(PIC2_DATA, 40);       /* Slave PIC starts at IRQ 40 */
    
    /* ICW3: Configure cascade */
    outb(PIC1_DATA, 0x04);     /* Tell master about slave at IRQ 2 */
    outb(PIC2_DATA, 0x02);     /* Tell slave its cascade identity */
    
    /* ICW4: Set 8086 mode */
    outb(PIC1_DATA, 0x01);
    outb(PIC2_DATA, 0x01);
    
    /* Restore interrupt masks */
    outb(PIC1_DATA, mask1);
    outb(PIC2_DATA, mask2);
}

/* Enable specific IRQ */
void irq_enable(uint8_t irq) {
    uint16_t port;
    uint8_t value;
    
    if (irq < 8) {
        port = PIC1_DATA;
    } else {
        port = PIC2_DATA;
        irq -= 8;
    }
    
    value = inb(port) & ~(1 << irq);
    outb(port, value);
}

/* Timer interrupt handler */
void handle_timer_irq(void) {
    timer_ticks++;
    
    /* Call scheduler tick for process management */
    scheduler_tick();
    
    /* Brandon Media OS - Cyberpunk style timer display every 100 ticks */
    if (timer_ticks % 100 == 0) {
        serial_puts("[NEXUS] Temporal sync: ");
        /* Simple hex display of timer_ticks */
        char hex_chars[] = "0123456789ABCDEF";
        uint64_t temp = timer_ticks;
        char buffer[17] = {0};
        int i = 15;
        
        while (temp > 0 && i >= 0) {
            buffer[i--] = hex_chars[temp & 0xF];
            temp >>= 4;
        }
        serial_puts(&buffer[i + 1]);
        serial_puts(" cycles\n");
    }
}

/* Keyboard interrupt handler */
void handle_keyboard_irq(void) {
    uint8_t scancode = inb(0x60);  /* Read from keyboard data port */
    
    serial_puts("[INPUT] Neural interface scancode: 0x");
    char hex_chars[] = "0123456789ABCDEF";
    serial_putc(hex_chars[(scancode >> 4) & 0xF]);
    serial_putc(hex_chars[scancode & 0xF]);
    serial_puts("\n");
}

/* Serial interrupt handler */
void handle_serial_irq(void) {
    serial_puts("[COMM] Serial data transmission complete\n");
}

/* Main IRQ handler dispatcher */
void irq_handler(struct registers *regs) {
    uint8_t irq_num = regs->int_no - 32;  /* Convert to IRQ number */
    
    switch (irq_num) {
        case 0:  /* Timer IRQ */
            handle_timer_irq();
            break;
            
        case 1:  /* Keyboard IRQ */
            handle_keyboard_irq();
            break;
            
        case 4:  /* Serial IRQ */
            handle_serial_irq();
            break;
            
        default:
            serial_puts("[UNKNOWN] IRQ #");
            char hex_chars[] = "0123456789ABCDEF";
            serial_putc(hex_chars[irq_num & 0xF]);
            serial_puts(" received\n");
            break;
    }
    
    /* Send End of Interrupt signal */
    send_eoi(irq_num);
}