/* libc.h - Brandon Media OS User Space C Library */
#ifndef _LIBC_H
#define _LIBC_H

#include <stdint.h>
#include <stddef.h>

/* Brandon Media OS - Neural Interface User Library */

/* Standard file descriptors */
#define STDIN_FILENO    0
#define STDOUT_FILENO   1
#define STDERR_FILENO   2

/* System call error codes */
extern int errno;

/* Process management */
int32_t getpid(void);
int32_t getppid(void);
void exit(int status);
int32_t fork(void);
int32_t execve(const char *pathname, char *const argv[], char *const envp[]);
int32_t wait(int *status);
int32_t kill(int32_t pid, int signal);
int32_t sleep(unsigned int seconds);
int32_t usleep(unsigned int usec);
void yield(void);

/* File I/O */
int32_t open(const char *pathname, int flags, ...);
int32_t close(int fd);
ssize_t read(int fd, void *buf, size_t count);
ssize_t write(int fd, const void *buf, size_t count);
off_t lseek(int fd, off_t offset, int whence);

/* Memory management */
void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset);
int munmap(void *addr, size_t length);
void *sbrk(intptr_t increment);
void *brk(void *addr);

/* String functions */
size_t strlen(const char *s);
char *strcpy(char *dest, const char *src);
char *strncpy(char *dest, const char *src, size_t n);
int strcmp(const char *s1, const char *s2);
int strncmp(const char *s1, const char *s2, size_t n);
char *strcat(char *dest, const char *src);
char *strchr(const char *s, int c);
char *strstr(const char *haystack, const char *needle);

/* Memory functions */
void *memset(void *s, int c, size_t n);
void *memcpy(void *dest, const void *src, size_t n);
void *memmove(void *dest, const void *src, size_t n);
int memcmp(const void *s1, const void *s2, size_t n);

/* Standard I/O */
int printf(const char *format, ...);
int sprintf(char *str, const char *format, ...);
int puts(const char *s);
int putchar(int c);
int getchar(void);

/* Memory allocation */
void *malloc(size_t size);
void *calloc(size_t nmemb, size_t size);
void *realloc(void *ptr, size_t size);
void free(void *ptr);

/* Utility functions */
int atoi(const char *nptr);
long atol(const char *nptr);
double atof(const char *nptr);
char *itoa(int value, char *str, int base);

/* Error handling */
void perror(const char *s);
char *strerror(int errnum);

/* Mathematical functions */
int abs(int j);
long labs(long j);
double sqrt(double x);
double pow(double base, double exponent);
double sin(double x);
double cos(double x);

/* Brandon Media OS specific functions */
void neural_output(const char *message);
void neural_debug(const char *message);
int neural_status(void);

#endif /* _LIBC_H */