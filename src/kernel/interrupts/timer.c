/* timer.c - Brandon Media OS PIT (Programmable Interval Timer) */
#include <stdint.h>
#include "kernel/interrupts.h"

/* PIT ports and commands */
#define PIT_CHANNEL0    0x40
#define PIT_CHANNEL1    0x41
#define PIT_CHANNEL2    0x42
#define PIT_COMMAND     0x43

/* PIT frequency (1.193182 MHz) */
#define PIT_FREQUENCY   1193182

/* I/O port functions */
static inline void outb(uint16_t port, uint8_t val) {
    asm volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

/* External functions */
extern void serial_puts(const char *s);
extern void irq_enable(uint8_t irq);

/* Initialize PIT timer */
void timer_init(uint32_t frequency) {
    /* Calculate divisor for desired frequency */
    uint32_t divisor = PIT_FREQUENCY / frequency;
    
    /* Brandon Media OS - Cyberpunk initialization message */
    serial_puts("[NEXUS] Initializing temporal synchronization matrix...\n");
    serial_puts("[TIMER] Setting quantum frequency to ");
    
    /* Simple decimal print */
    if (frequency >= 1000) {
        serial_puts("1kHz");
    } else if (frequency >= 100) {
        serial_puts("100Hz");
    } else {
        serial_puts("50Hz");
    }
    serial_puts("\n");
    
    /* Configure PIT Channel 0 */
    outb(PIT_COMMAND, 0x36);  /* Channel 0, low/high byte, mode 3 (square wave) */
    
    /* Send divisor (low byte first, then high byte) */
    outb(PIT_CHANNEL0, divisor & 0xFF);
    outb(PIT_CHANNEL0, (divisor >> 8) & 0xFF);
    
    /* Enable timer IRQ */
    irq_enable(0);
    
    serial_puts("[NEXUS] Temporal matrix synchronized - quantum flow active\n");
}

/* Get current timer ticks (extern declaration for access from other modules) */
extern volatile uint64_t timer_ticks;

uint64_t timer_get_ticks(void) {
    return timer_ticks;
}

/* Simple delay function using timer ticks */
void timer_delay(uint32_t ms) {
    uint64_t start_ticks = timer_get_ticks();
    uint64_t target_ticks = start_ticks + (ms / 10);  /* Assuming 100Hz timer */
    
    while (timer_get_ticks() < target_ticks) {
        asm volatile("hlt");  /* Halt until next interrupt */
    }
}