/* idt.c - Brandon Media OS Interrupt Descriptor Table */
#include <stdint.h>
#include "kernel/interrupts.h"

/* IDT with 256 entries for x86_64 */
static struct idt_entry idt[256];
static struct idt_ptr idt_pointer;

/* External assembly functions */
extern void idt_flush(uint64_t);

/* Set up an IDT entry */
void idt_set_gate(uint8_t num, uint64_t handler, uint16_t sel, uint8_t flags) {
    idt[num].offset_low  = handler & 0xFFFF;
    idt[num].offset_mid  = (handler >> 16) & 0xFFFF;
    idt[num].offset_high = (handler >> 32) & 0xFFFFFFFF;
    idt[num].selector    = sel;
    idt[num].ist         = 0;  /* No IST for now */
    idt[num].type_attr   = flags;
    idt[num].zero        = 0;
}

/* Initialize the IDT */
void idt_init(void) {
    /* Set up the IDT pointer */
    idt_pointer.limit = sizeof(struct idt_entry) * 256 - 1;
    idt_pointer.base  = (uint64_t)&idt;

    /* Clear the IDT */
    for (int i = 0; i < 256; i++) {
        idt_set_gate(i, 0, 0, 0);
    }

    /* Set up exception handlers (0-31) */
    idt_set_gate(0,  (uint64_t)divide_error_handler,         0x08, IDT_PRESENT | IDT_EXCEPTION | IDT_RING0);
    idt_set_gate(1,  (uint64_t)debug_handler,                0x08, IDT_PRESENT | IDT_EXCEPTION | IDT_RING0);
    idt_set_gate(2,  (uint64_t)nmi_handler,                  0x08, IDT_PRESENT | IDT_EXCEPTION | IDT_RING0);
    idt_set_gate(3,  (uint64_t)breakpoint_handler,           0x08, IDT_PRESENT | IDT_EXCEPTION | IDT_RING0);
    idt_set_gate(4,  (uint64_t)overflow_handler,             0x08, IDT_PRESENT | IDT_EXCEPTION | IDT_RING0);
    idt_set_gate(5,  (uint64_t)bounds_handler,               0x08, IDT_PRESENT | IDT_EXCEPTION | IDT_RING0);
    idt_set_gate(6,  (uint64_t)invalid_opcode_handler,       0x08, IDT_PRESENT | IDT_EXCEPTION | IDT_RING0);
    idt_set_gate(7,  (uint64_t)device_not_available_handler, 0x08, IDT_PRESENT | IDT_EXCEPTION | IDT_RING0);
    idt_set_gate(8,  (uint64_t)double_fault_handler,         0x08, IDT_PRESENT | IDT_EXCEPTION | IDT_RING0);
    idt_set_gate(10, (uint64_t)invalid_tss_handler,          0x08, IDT_PRESENT | IDT_EXCEPTION | IDT_RING0);
    idt_set_gate(11, (uint64_t)segment_not_present_handler,  0x08, IDT_PRESENT | IDT_EXCEPTION | IDT_RING0);
    idt_set_gate(12, (uint64_t)stack_fault_handler,          0x08, IDT_PRESENT | IDT_EXCEPTION | IDT_RING0);
    idt_set_gate(13, (uint64_t)general_protection_handler,   0x08, IDT_PRESENT | IDT_EXCEPTION | IDT_RING0);
    idt_set_gate(14, (uint64_t)page_fault_handler,           0x08, IDT_PRESENT | IDT_EXCEPTION | IDT_RING0);

    /* Set up hardware interrupt handlers (32+) */
    idt_set_gate(IRQ_TIMER,    (uint64_t)timer_handler,    0x08, IDT_PRESENT | IDT_INTERRUPT | IDT_RING0);
    idt_set_gate(IRQ_KEYBOARD, (uint64_t)keyboard_handler, 0x08, IDT_PRESENT | IDT_INTERRUPT | IDT_RING0);
    idt_set_gate(IRQ_SERIAL,   (uint64_t)serial_handler,   0x08, IDT_PRESENT | IDT_INTERRUPT | IDT_RING0);

    /* Load the IDT */
    idt_flush((uint64_t)&idt_pointer);
}