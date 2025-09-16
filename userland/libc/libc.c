/* libc.c - Brandon Media OS User Space C Library Implementation */
#include "libc.h"

/* Error number */
int errno = 0;

/* System call wrappers */

int32_t getpid(void) {
    int64_t result;
    asm volatile("syscall" : "=a"(result) : "a"(11) : "rcx", "r11", "memory");
    return (int32_t)result;
}

int32_t getppid(void) {
    int64_t result;
    asm volatile("syscall" : "=a"(result) : "a"(12) : "rcx", "r11", "memory");
    return (int32_t)result;
}

void exit(int status) {
    asm volatile("syscall" : : "a"(0), "D"(status) : "rcx", "r11", "memory");
    /* Should not return */
    while(1);
}

ssize_t write(int fd, const void *buf, size_t count) {
    int64_t result;
    asm volatile("syscall" : "=a"(result) : "a"(2), "D"(fd), "S"(buf), "d"(count) : "rcx", "r11", "memory");
    return (ssize_t)result;
}

ssize_t read(int fd, void *buf, size_t count) {
    int64_t result;
    asm volatile("syscall" : "=a"(result) : "a"(1), "D"(fd), "S"(buf), "d"(count) : "rcx", "r11", "memory");
    return (ssize_t)result;
}

int32_t sleep(unsigned int seconds) {
    int64_t result;
    uint32_t milliseconds = seconds * 1000;
    asm volatile("syscall" : "=a"(result) : "a"(9), "D"(milliseconds) : "rcx", "r11", "memory");
    return (int32_t)result;
}

void yield(void) {
    asm volatile("syscall" : : "a"(10) : "rcx", "r11", "memory");
}

/* String functions */

size_t strlen(const char *s) {
    size_t len = 0;
    while (s[len]) len++;
    return len;
}

char *strcpy(char *dest, const char *src) {
    char *original_dest = dest;
    while ((*dest++ = *src++));
    return original_dest;
}

int strcmp(const char *s1, const char *s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(unsigned char*)s1 - *(unsigned char*)s2;
}

char *strcat(char *dest, const char *src) {
    char *original_dest = dest;
    while (*dest) dest++;  /* Find end of dest */
    while ((*dest++ = *src++));  /* Copy src */
    return original_dest;
}

/* Memory functions */

void *memset(void *s, int c, size_t n) {
    unsigned char *p = (unsigned char*)s;
    while (n--) *p++ = (unsigned char)c;
    return s;
}

void *memcpy(void *dest, const void *src, size_t n) {
    unsigned char *d = (unsigned char*)dest;
    const unsigned char *s = (const unsigned char*)src;
    while (n--) *d++ = *s++;
    return dest;
}

int memcmp(const void *s1, const void *s2, size_t n) {
    const unsigned char *p1 = (const unsigned char*)s1;
    const unsigned char *p2 = (const unsigned char*)s2;
    while (n--) {
        if (*p1 != *p2) return *p1 - *p2;
        p1++;
        p2++;
    }
    return 0;
}

/* Standard I/O functions */

int puts(const char *s) {
    size_t len = strlen(s);
    write(STDOUT_FILENO, s, len);
    write(STDOUT_FILENO, "\\n", 1);
    return len + 1;
}

int putchar(int c) {
    char ch = (char)c;
    return write(STDOUT_FILENO, &ch, 1);
}

/* Simple printf implementation */
int printf(const char *format, ...) {
    /* Very basic implementation - just output the format string */
    return puts(format);
}

/* Utility functions */

int atoi(const char *nptr) {
    int result = 0;
    int sign = 1;
    
    /* Skip whitespace */
    while (*nptr == ' ' || *nptr == '\\t') nptr++;
    
    /* Handle sign */
    if (*nptr == '-') {
        sign = -1;
        nptr++;
    } else if (*nptr == '+') {
        nptr++;
    }
    
    /* Convert digits */
    while (*nptr >= '0' && *nptr <= '9') {
        result = result * 10 + (*nptr - '0');
        nptr++;
    }
    
    return result * sign;
}

char *itoa(int value, char *str, int base) {
    char *ptr = str, *ptr1 = str, tmp_char;
    int tmp_value;
    
    /* Handle negative numbers for base 10 */
    if (value < 0 && base == 10) {
        *ptr++ = '-';
        value = -value;
        ptr1++;
    }
    
    /* Convert to string */
    do {
        tmp_value = value;
        value /= base;
        *ptr++ = "0123456789ABCDEF"[tmp_value - value * base];
    } while (value);
    
    *ptr-- = '\\0';
    
    /* Reverse string */
    while (ptr1 < ptr) {
        tmp_char = *ptr;
        *ptr-- = *ptr1;
        *ptr1++ = tmp_char;
    }
    
    return str;
}

/* Mathematical functions */

int abs(int j) {
    return (j < 0) ? -j : j;
}

/* Brandon Media OS specific functions */

void neural_output(const char *message) {
    write(STDOUT_FILENO, "[NEURAL] ", 9);
    puts(message);
}

void neural_debug(const char *message) {
    write(STDERR_FILENO, "[DEBUG] ", 8);
    puts(message);
}

int neural_status(void) {
    neural_output("Neural interface status: ONLINE");
    return getpid();
}