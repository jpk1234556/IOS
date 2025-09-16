/* interrupts.h - Brandon Media OS Interrupt System */
#ifndef _INTERRUPTS_H
#define _INTERRUPTS_H

#include <stdint.h>

/* IDT Entry Structure for x86_64 */
struct idt_entry {
    uint16_t offset_low;    /* Offset bits 0-15 */
    uint16_t selector;      /* Code segment selector */
    uint8_t  ist;          /* Interrupt Stack Table offset (bits 0-2) */
    uint8_t  type_attr;    /* Type and attributes */
    uint16_t offset_mid;    /* Offset bits 16-31 */
    uint32_t offset_high;   /* Offset bits 32-63 */
    uint32_t zero;         /* Reserved */
} __attribute__((packed));

/* IDT Pointer Structure */
struct idt_ptr {
    uint16_t limit;
    uint64_t base;
} __attribute__((packed));

/* Exception/Interrupt Numbers */
#define IRQ_TIMER        32  /* PIT Timer */
#define IRQ_KEYBOARD     33  /* Keyboard */
#define IRQ_SERIAL       36  /* Serial COM1 */

/* Interrupt attribute flags */
#define IDT_PRESENT      0x80
#define IDT_INTERRUPT    0x0E
#define IDT_EXCEPTION    0x0F
#define IDT_RING0        0x00
#define IDT_RING3        0x60

/* Brandon Media OS - Interrupt System Functions */
void idt_init(void);
void idt_set_gate(uint8_t num, uint64_t handler, uint16_t sel, uint8_t flags);
void interrupts_enable(void);
void interrupts_disable(void);

/* Exception handlers */
void divide_error_handler(void);
void debug_handler(void);
void nmi_handler(void);
void breakpoint_handler(void);
void overflow_handler(void);
void bounds_handler(void);
void invalid_opcode_handler(void);
void device_not_available_handler(void);
void double_fault_handler(void);
void invalid_tss_handler(void);
void segment_not_present_handler(void);
void stack_fault_handler(void);
void general_protection_handler(void);
void page_fault_handler(void);

/* Hardware interrupt handlers */
void timer_handler(void);
void keyboard_handler(void);
void serial_handler(void);

#endif /* _INTERRUPTS_H */