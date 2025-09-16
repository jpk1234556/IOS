/* threads.c - Brandon Media OS Thread Management */
#include <stdint.h>
#include "kernel/process.h"
#include "kernel/memory.h"

/* External functions */
extern void serial_puts(const char *s);
extern void print_dec(uint64_t num);
extern void *kmalloc(size_t size);
extern void kfree(void *ptr);
extern void memory_set(void *dst, int value, size_t size);

static uint32_t next_tid = 1;

/* Create new thread */
struct thread *thread_create(struct process *proc, void (*entry_point)(void)) {
    if (!proc || !entry_point) return NULL;
    
    serial_puts("[THREADS] Spawning neural thread in process: ");
    serial_puts(proc->name);
    serial_puts("\\n");
    
    /* Allocate thread structure */
    struct thread *thread = (struct thread *)kmalloc(sizeof(struct thread));
    if (!thread) {
        serial_puts("[ERROR] Thread allocation failed\\n");
        return NULL;
    }
    
    /* Initialize thread */
    memory_set(thread, 0, sizeof(struct thread));
    thread->tid = next_tid++;
    thread->process = proc;
    thread->state = PROCESS_READY;
    
    /* Allocate thread stack */
    thread->stack_size = 4096;  /* 4KB thread stack */
    thread->stack_base = (uint64_t)vmm_alloc(thread->stack_size, 
                                            PAGE_PRESENT | PAGE_WRITABLE);
    if (!thread->stack_base) {
        serial_puts("[ERROR] Thread stack allocation failed\\n");
        kfree(thread);
        return NULL;
    }
    
    /* Initialize thread context */
    memory_set(&thread->context, 0, sizeof(struct cpu_context));
    thread->context.rip = (uint64_t)entry_point;
    thread->context.rsp = thread->stack_base + thread->stack_size - 16;
    thread->context.rflags = 0x202;
    thread->context.cs = 0x08;
    thread->context.ds = 0x10;
    thread->context.cr3 = (uint64_t)proc->page_directory;
    
    /* Add to process thread list */
    thread->next = proc->children;  /* Reusing children pointer for threads */
    proc->children = (struct process *)thread;  /* Cast for simplicity */
    
    serial_puts("[SUCCESS] Thread created - TID: ");
    print_dec(thread->tid);
    serial_puts("\\n");
    
    return thread;
}

/* Destroy thread */
void thread_destroy(struct thread *thread) {
    if (!thread) return;
    
    serial_puts("[THREADS] Destroying neural thread: ");
    print_dec(thread->tid);
    serial_puts("\\n");
    
    /* Free thread stack */
    if (thread->stack_base) {
        vmm_free((void *)thread->stack_base);
    }
    
    /* Remove from process thread list (simplified) */
    /* In a full implementation, we'd properly manage the linked list */
    
    /* Free thread structure */
    kfree(thread);
}

/* Thread yield */
void thread_yield(void) {
    serial_puts("[THREADS] Thread yielding neural processing\\n");
    scheduler_yield();
}