/* process.h - Brandon Media OS Process Management System */
#ifndef _PROCESS_H
#define _PROCESS_H

#include <stdint.h>
#include <stddef.h>
#include "kernel/memory.h"

/* Brandon Media OS - Neural Process Matrix Definitions */

/* Process states - Cyberpunk process lifecycle */
typedef enum {
    PROCESS_CREATED     = 0,    /* Process spawned in matrix */
    PROCESS_READY       = 1,    /* Ready for neural processing */
    PROCESS_RUNNING     = 2,    /* Currently executing in matrix */
    PROCESS_BLOCKED     = 3,    /* Waiting for I/O or resources */
    PROCESS_SLEEPING    = 4,    /* Temporal suspension state */
    PROCESS_ZOMBIE      = 5,    /* Terminated, awaiting cleanup */
    PROCESS_TERMINATED  = 6     /* Process deleted from matrix */
} process_state_t;

/* Process priority levels */
typedef enum {
    PRIORITY_IDLE       = 0,    /* Background matrix processes */
    PRIORITY_LOW        = 1,    /* Low priority tasks */
    PRIORITY_NORMAL     = 2,    /* Standard neural processes */
    PRIORITY_HIGH       = 3,    /* High priority system tasks */
    PRIORITY_REALTIME   = 4     /* Real-time neural interface */
} process_priority_t;

/* CPU context for x86_64 */
struct cpu_context {
    /* General purpose registers */
    uint64_t rax, rbx, rcx, rdx;
    uint64_t rsi, rdi, rbp, rsp;
    uint64_t r8, r9, r10, r11;
    uint64_t r12, r13, r14, r15;
    
    /* Control registers */
    uint64_t rip;           /* Instruction pointer */
    uint64_t rflags;        /* CPU flags */
    uint64_t cr3;           /* Page directory base */
    
    /* Segment registers */
    uint16_t cs, ds, es, fs, gs, ss;
    
    /* FPU/SSE state pointer */
    void *fpu_state;
} __attribute__((packed));

/* Process Control Block - Neural Process Node */
struct process {
    /* Process identification */
    uint32_t pid;                   /* Process ID */
    uint32_t ppid;                  /* Parent Process ID */
    char name[64];                  /* Process name */
    
    /* Process state */
    process_state_t state;          /* Current process state */
    process_priority_t priority;    /* Process priority */
    uint32_t time_slice;           /* Quantum time allocation */
    uint64_t cpu_time;             /* Total CPU time used */
    uint64_t creation_time;        /* Process spawn timestamp */
    
    /* Memory management */
    pml4_t *page_directory;        /* Virtual memory space */
    uint64_t memory_usage;         /* Memory consumption */
    uint64_t stack_base;           /* Stack base address */
    uint64_t stack_size;           /* Stack size */
    uint64_t heap_base;            /* Heap base address */
    uint64_t heap_size;            /* Heap size */
    
    /* CPU context */
    struct cpu_context context;    /* Saved CPU state */
    
    /* Process relationships */
    struct process *parent;        /* Parent process */
    struct process *children;      /* Child processes list */
    struct process *next_sibling;  /* Next sibling process */
    
    /* Scheduling information */
    struct process *next;          /* Next in scheduler queue */
    struct process *prev;          /* Previous in scheduler queue */
    uint64_t last_scheduled;      /* Last scheduling timestamp */
    uint32_t sleep_until;         /* Wake up time (if sleeping) */
    
    /* IPC and signals */
    uint32_t pending_signals;     /* Pending signal mask */
    void *message_queue;          /* IPC message queue */
    
    /* File descriptors */
    void *file_table;             /* Open file descriptors */
    
    /* Process statistics */
    uint64_t page_faults;         /* Page fault count */
    uint64_t syscall_count;       /* System call count */
    uint64_t context_switches;    /* Context switch count */
} __attribute__((packed));

/* Scheduler statistics */
struct scheduler_stats {
    uint64_t total_processes;     /* Total processes created */
    uint64_t active_processes;    /* Currently active processes */
    uint64_t context_switches;    /* Total context switches */
    uint64_t scheduling_overhead; /* Time spent scheduling */
    uint64_t idle_time;          /* Time spent in idle */
    uint32_t load_average;       /* System load average */
};

/* Thread Control Block */
struct thread {
    uint32_t tid;                 /* Thread ID */
    struct process *process;      /* Parent process */
    struct cpu_context context;  /* Thread CPU context */
    uint64_t stack_base;         /* Thread stack base */
    uint64_t stack_size;         /* Thread stack size */
    process_state_t state;       /* Thread state */
    struct thread *next;         /* Next thread in process */
};

/* Brandon Media OS - Process Management Functions */

/* Process management */
void process_init(void);
struct process *process_create(const char *name, void (*entry_point)(void), 
                              process_priority_t priority);
void process_terminate(struct process *proc);
void process_destroy(struct process *proc);
struct process *process_get_current(void);
struct process *process_get_by_pid(uint32_t pid);
void process_sleep(uint32_t milliseconds);
void process_wake(struct process *proc);

/* Scheduling */
void scheduler_init(void);
void scheduler_add_process(struct process *proc);
void scheduler_remove_process(struct process *proc);
void scheduler_yield(void);
void scheduler_preempt(void);
struct process *scheduler_next(void);
void scheduler_tick(void);

/* Context switching */
void context_switch(struct process *from, struct process *to);
void save_context(struct cpu_context *context);
void restore_context(struct cpu_context *context);

/* Thread management */
struct thread *thread_create(struct process *proc, void (*entry_point)(void));
void thread_destroy(struct thread *thread);
void thread_yield(void);

/* IPC (Inter-Process Communication) */
int ipc_send_message(uint32_t dest_pid, const void *data, size_t size);
int ipc_receive_message(void *buffer, size_t max_size);
int ipc_create_pipe(uint32_t *read_fd, uint32_t *write_fd);

/* Process utilities */
void process_dump_info(struct process *proc);
struct scheduler_stats *get_scheduler_stats(void);
void process_set_priority(struct process *proc, process_priority_t priority);
uint32_t process_get_next_pid(void);

/* Signals */
#define SIGNAL_TERMINATE    1
#define SIGNAL_INTERRUPT    2
#define SIGNAL_SEGFAULT     11
#define SIGNAL_ALARM        14

void signal_send(uint32_t pid, uint32_t signal);
void signal_handle(struct process *proc, uint32_t signal);

/* Process creation helpers */
#define PROCESS_STACK_SIZE  (8 * 1024)     /* 8KB default stack */
#define PROCESS_HEAP_SIZE   (64 * 1024)    /* 64KB default heap */
#define MAX_PROCESSES       256             /* Maximum processes */
#define DEFAULT_TIME_SLICE  10              /* 10ms time slice */

/* Scheduler algorithms */
typedef enum {
    SCHED_ROUND_ROBIN   = 0,    /* Round-robin scheduling */
    SCHED_PRIORITY      = 1,    /* Priority-based scheduling */
    SCHED_CFS          = 2     /* Completely Fair Scheduler */
} scheduler_algorithm_t;

void scheduler_set_algorithm(scheduler_algorithm_t algorithm);

#endif /* _PROCESS_H */