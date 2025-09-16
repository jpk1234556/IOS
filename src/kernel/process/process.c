/* process.c - Brandon Media OS Process Management Core */
#include <stdint.h>
#include "kernel/process.h"
#include "kernel/memory.h"
#include "kernel/interrupts.h"

/* External functions */
extern void serial_puts(const char *s);
extern void print_hex(uint64_t num);
extern void print_dec(uint64_t num);
extern uint64_t timer_get_ticks(void);
extern void *kmalloc(size_t size);
extern void kfree(void *ptr);
extern void memory_set(void *dst, int value, size_t size);
extern void memory_copy(void *dst, const void *src, size_t size);

/* Process management globals */
static struct process *current_process = NULL;
static struct process *process_list = NULL;
static struct process *idle_process = NULL;
static uint32_t next_pid = 1;
static uint32_t active_process_count = 0;

/* Process statistics */
static struct scheduler_stats sched_stats = {0};

/* Process array for quick PID lookup */
static struct process *process_table[MAX_PROCESSES] = {NULL};

/* Idle process function */
static void idle_process_main(void) {
    while (1) {
        serial_puts("[MATRIX] Idle process - system in low power mode\\n");
        asm volatile("hlt");  /* Wait for interrupts */
    }
}

/* Initialize process management system */
void process_init(void) {
    serial_puts("[NEXUS] Initializing neural process matrix...\\n");
    
    /* Create idle process */
    idle_process = process_create("idle", idle_process_main, PRIORITY_IDLE);
    if (!idle_process) {
        serial_puts("[FATAL] Failed to create idle process - matrix unstable\\n");
        return;
    }
    
    /* Set idle process as current */
    current_process = idle_process;
    
    serial_puts("[MATRIX] Neural process matrix online\\n");
    serial_puts("[STATS] Process limit: ");
    print_dec(MAX_PROCESSES);
    serial_puts(" processes\\n");
}

/* Get next available PID */
uint32_t process_get_next_pid(void) {
    uint32_t pid = next_pid++;
    
    /* Wrap around if we exceed maximum */
    if (next_pid >= MAX_PROCESSES) {
        next_pid = 1;  /* PID 0 reserved for kernel */
    }
    
    /* Ensure PID is not in use */
    while (process_table[pid] != NULL && pid < MAX_PROCESSES) {
        pid = next_pid++;
        if (next_pid >= MAX_PROCESSES) {
            next_pid = 1;
        }
    }
    
    return pid;
}

/* Create new process */
struct process *process_create(const char *name, void (*entry_point)(void), 
                             process_priority_t priority) {
    serial_puts("[MATRIX] Spawning new neural process: ");
    serial_puts(name);
    serial_puts("\\n");
    
    /* Allocate process structure */
    struct process *proc = (struct process *)kmalloc(sizeof(struct process));
    if (!proc) {
        serial_puts("[ERROR] Process allocation failed - memory matrix overflow\\n");
        return NULL;
    }
    
    /* Clear process structure */
    memory_set(proc, 0, sizeof(struct process));
    
    /* Assign PID */
    proc->pid = process_get_next_pid();
    if (proc->pid >= MAX_PROCESSES) {
        serial_puts("[ERROR] Process limit exceeded - matrix capacity reached\\n");
        kfree(proc);
        return NULL;
    }
    
    /* Set process name */
    memory_copy(proc->name, name, sizeof(proc->name) - 1);
    proc->name[sizeof(proc->name) - 1] = 0;  /* Null terminate */
    
    /* Initialize process metadata */
    proc->state = PROCESS_CREATED;
    proc->priority = priority;
    proc->time_slice = DEFAULT_TIME_SLICE;
    proc->creation_time = timer_get_ticks();
    proc->parent = current_process;
    proc->ppid = current_process ? current_process->pid : 0;
    
    /* Create virtual address space */
    proc->page_directory = paging_create_address_space();
    if (!proc->page_directory) {
        serial_puts("[ERROR] Virtual address space creation failed\\n");
        kfree(proc);
        return NULL;
    }
    
    /* Allocate stack */
    proc->stack_size = PROCESS_STACK_SIZE;
    proc->stack_base = (uint64_t)vmm_alloc(proc->stack_size, 
                                          PAGE_PRESENT | PAGE_WRITABLE | PAGE_USER);
    if (!proc->stack_base) {
        serial_puts("[ERROR] Process stack allocation failed\\n");
        paging_destroy_address_space(proc->page_directory);
        kfree(proc);
        return NULL;
    }
    
    /* Allocate heap */
    proc->heap_size = PROCESS_HEAP_SIZE;
    proc->heap_base = (uint64_t)vmm_alloc(proc->heap_size,
                                         PAGE_PRESENT | PAGE_WRITABLE | PAGE_USER);
    if (!proc->heap_base) {
        serial_puts("[ERROR] Process heap allocation failed\\n");
        vmm_free((void *)proc->stack_base);
        paging_destroy_address_space(proc->page_directory);
        kfree(proc);
        return NULL;
    }
    
    /* Initialize CPU context */
    memory_set(&proc->context, 0, sizeof(struct cpu_context));
    proc->context.rip = (uint64_t)entry_point;
    proc->context.rsp = proc->stack_base + proc->stack_size - 16;  /* 16-byte align */
    proc->context.rflags = 0x202;  /* Enable interrupts */
    proc->context.cs = 0x08;       /* Kernel code segment */
    proc->context.ds = 0x10;       /* Kernel data segment */
    proc->context.cr3 = (uint64_t)proc->page_directory;
    
    /* Add to process table */
    process_table[proc->pid] = proc;
    active_process_count++;
    sched_stats.total_processes++;
    
    /* Set state to ready */
    proc->state = PROCESS_READY;
    
    serial_puts("[SUCCESS] Process spawned - PID: ");
    print_dec(proc->pid);
    serial_puts(", Priority: ");
    print_dec(priority);
    serial_puts("\\n");
    
    return proc;
}

/* Terminate process */
void process_terminate(struct process *proc) {
    if (!proc) return;
    
    serial_puts("[MATRIX] Terminating neural process: ");
    serial_puts(proc->name);
    serial_puts(" (PID: ");
    print_dec(proc->pid);
    serial_puts(")\\n");
    
    /* Change state to zombie */
    proc->state = PROCESS_ZOMBIE;
    
    /* Wake up parent if waiting */
    if (proc->parent && proc->parent->state == PROCESS_BLOCKED) {
        proc->parent->state = PROCESS_READY;
    }
    
    /* Schedule cleanup */
    scheduler_remove_process(proc);
}

/* Destroy process (cleanup) */
void process_destroy(struct process *proc) {
    if (!proc) return;
    
    serial_puts("[MATRIX] Destroying process matrix node: ");
    print_dec(proc->pid);
    serial_puts("\\n");
    
    /* Free virtual memory */
    if (proc->stack_base) {
        vmm_free((void *)proc->stack_base);
    }
    if (proc->heap_base) {
        vmm_free((void *)proc->heap_base);
    }
    
    /* Destroy address space */
    if (proc->page_directory) {
        paging_destroy_address_space(proc->page_directory);
    }
    
    /* Remove from process table */
    if (proc->pid < MAX_PROCESSES) {
        process_table[proc->pid] = NULL;
    }
    
    /* Update statistics */
    active_process_count--;
    
    /* Free process structure */
    kfree(proc);
}

/* Get current process */
struct process *process_get_current(void) {
    return current_process;
}

/* Get process by PID */
struct process *process_get_by_pid(uint32_t pid) {
    if (pid >= MAX_PROCESSES) {
        return NULL;
    }
    return process_table[pid];
}

/* Put process to sleep */
void process_sleep(uint32_t milliseconds) {
    if (!current_process) return;
    
    serial_puts("[MATRIX] Process entering temporal suspension: ");
    print_dec(milliseconds);
    serial_puts("ms\\n");
    
    current_process->state = PROCESS_SLEEPING;
    current_process->sleep_until = timer_get_ticks() + (milliseconds / 10);  /* Convert to ticks */
    
    scheduler_yield();
}

/* Wake up sleeping process */
void process_wake(struct process *proc) {
    if (!proc || proc->state != PROCESS_SLEEPING) return;
    
    serial_puts("[MATRIX] Process awakening from temporal suspension: ");
    serial_puts(proc->name);
    serial_puts("\\n");
    
    proc->state = PROCESS_READY;
    proc->sleep_until = 0;
}

/* Dump process information */
void process_dump_info(struct process *proc) {
    if (!proc) return;
    
    serial_puts("\\n[NEURAL PROCESS MATRIX DUMP]\\n");
    serial_puts("╔══════════════════════════════════════════════════════════╗\\n");
    serial_puts("║                 BRANDON MEDIA OS - PROCESS INFO          ║\\n");
    serial_puts("╠══════════════════════════════════════════════════════════╣\\n");
    
    serial_puts("║ Process Name: ");
    serial_puts(proc->name);
    serial_puts("\\n");
    
    serial_puts("║ PID: ");
    print_dec(proc->pid);
    serial_puts(" | PPID: ");
    print_dec(proc->ppid);
    serial_puts("\\n");
    
    serial_puts("║ State: ");
    switch (proc->state) {
        case PROCESS_CREATED:    serial_puts("CREATED"); break;
        case PROCESS_READY:      serial_puts("READY"); break;
        case PROCESS_RUNNING:    serial_puts("RUNNING"); break;
        case PROCESS_BLOCKED:    serial_puts("BLOCKED"); break;
        case PROCESS_SLEEPING:   serial_puts("SLEEPING"); break;
        case PROCESS_ZOMBIE:     serial_puts("ZOMBIE"); break;
        case PROCESS_TERMINATED: serial_puts("TERMINATED"); break;
        default:                 serial_puts("UNKNOWN"); break;
    }
    serial_puts("\\n");
    
    serial_puts("║ Priority: ");
    print_dec(proc->priority);
    serial_puts(" | CPU Time: ");
    print_dec(proc->cpu_time);
    serial_puts("\\n");
    
    serial_puts("║ Memory Usage: ");
    print_hex(proc->memory_usage);
    serial_puts("\\n");
    
    serial_puts("║ Context Switches: ");
    print_dec(proc->context_switches);
    serial_puts("\\n");
    
    serial_puts("╚══════════════════════════════════════════════════════════╝\\n");
}

/* Get scheduler statistics */
struct scheduler_stats *get_scheduler_stats(void) {
    sched_stats.active_processes = active_process_count;
    return &sched_stats;
}