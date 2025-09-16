/* utils.c - Brandon Media OS Utility Functions */
#include <stdint.h>

/* External functions */
extern void serial_puts(const char *s);
extern void serial_putc(char c);

/* Print hex number with cyberpunk styling */
void print_hex(uint64_t num) {
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

/* Print decimal number */
void print_dec(uint64_t num) {
    if (num == 0) {
        serial_putc('0');
        return;
    }
    
    char buffer[21] = {0};  /* Enough for 64-bit number */
    int i = 19;
    
    while (num > 0 && i >= 0) {
        buffer[i--] = '0' + (num % 10);
        num /= 10;
    }
    
    serial_puts(&buffer[i + 1]);
}