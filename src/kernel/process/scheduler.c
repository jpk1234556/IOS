/* scheduler.c - Brandon Media OS Neural Process Scheduler */
#include <stdint.h>
#include "kernel/process.h"
#include "kernel/memory.h"
#include "kernel/interrupts.h"

/* External functions */
extern void serial_puts(const char *s);
extern void print_hex(uint64_t num);
extern void print_dec(uint64_t num);
extern uint64_t timer_get_ticks(void);
extern void context_switch_asm(struct cpu_context *from, struct cpu_context *to);

/* Scheduler state */
static scheduler_algorithm_t current_algorithm = SCHED_ROUND_ROBIN;
static struct process *ready_queue_head = NULL;
static struct process *ready_queue_tail = NULL;
static struct process *running_process = NULL;
static uint64_t last_schedule_time = 0;
static uint32_t time_slice_counter = 0;

/* Priority queues for priority scheduling */
static struct process *priority_queues[5] = {NULL}; /* One for each priority level */

/* External process management functions */
extern struct process *process_get_current(void);
extern void process_wake(struct process *proc);
extern struct scheduler_stats *get_scheduler_stats(void);
extern struct process *process_table[MAX_PROCESSES];
extern struct process *current_pml4;

/* Initialize scheduler */
void scheduler_init(void) {
    serial_puts("[NEXUS] Initializing neural process scheduler matrix...\\n");
    
    /* Clear ready queue */
    ready_queue_head = NULL;
    ready_queue_tail = NULL;
    running_process = NULL;
    
    /* Clear priority queues */
    for (int i = 0; i < 5; i++) {
        priority_queues[i] = NULL;
    }
    
    last_schedule_time = timer_get_ticks();
    time_slice_counter = 0;
    
    serial_puts("[SCHEDULER] Algorithm: ");
    switch (current_algorithm) {
        case SCHED_ROUND_ROBIN:
            serial_puts("Round Robin Quantum Processing\\n");
            break;
        case SCHED_PRIORITY:
            serial_puts("Priority-Based Neural Scheduling\\n");
            break;
        case SCHED_CFS:
            serial_puts("Completely Fair Scheduling Matrix\\n");
            break;
        default:
            serial_puts("Unknown Algorithm\\n");
            break;
    }
    
    serial_puts("[NEXUS] Process scheduler matrix online\\n");
}

/* Add process to ready queue */
void scheduler_add_process(struct process *proc) {
    if (!proc || proc->state != PROCESS_READY) return;
    
    serial_puts("[SCHEDULER] Adding process to neural queue: ");
    serial_puts(proc->name);
    serial_puts(" (PID: ");
    print_dec(proc->pid);
    serial_puts(")\\n");
    
    /* Disable interrupts during queue manipulation */
    interrupts_disable();
    
    switch (current_algorithm) {
        case SCHED_ROUND_ROBIN:
            /* Add to end of round-robin queue */
            proc->next = NULL;
            proc->prev = ready_queue_tail;
            
            if (ready_queue_tail) {
                ready_queue_tail->next = proc;
            } else {
                ready_queue_head = proc;
            }
            ready_queue_tail = proc;
            break;
            
        case SCHED_PRIORITY:
            /* Add to appropriate priority queue */
            if (proc->priority < 5) {
                proc->next = priority_queues[proc->priority];
                priority_queues[proc->priority] = proc;
            }
            break;
            
        case SCHED_CFS:
            /* Simple CFS implementation - add sorted by CPU time */
            if (!ready_queue_head || proc->cpu_time <= ready_queue_head->cpu_time) {
                proc->next = ready_queue_head;
                if (ready_queue_head) {
                    ready_queue_head->prev = proc;
                }
                ready_queue_head = proc;
                proc->prev = NULL;
                if (!ready_queue_tail) {
                    ready_queue_tail = proc;
                }
            } else {
                struct process *current = ready_queue_head;
                while (current->next && current->next->cpu_time < proc->cpu_time) {
                    current = current->next;
                }
                proc->next = current->next;
                proc->prev = current;
                if (current->next) {
                    current->next->prev = proc;
                } else {
                    ready_queue_tail = proc;
                }
                current->next = proc;
            }
            break;
    }
    
    interrupts_enable();
}

/* Remove process from ready queue */
void scheduler_remove_process(struct process *proc) {
    if (!proc) return;
    
    serial_puts("[SCHEDULER] Removing process from neural queue: ");
    print_dec(proc->pid);
    serial_puts("\\n");
    
    interrupts_disable();
    
    switch (current_algorithm) {
        case SCHED_ROUND_ROBIN:
        case SCHED_CFS:
            /* Remove from linked list */
            if (proc->prev) {
                proc->prev->next = proc->next;
            } else {
                ready_queue_head = proc->next;
            }
            
            if (proc->next) {
                proc->next->prev = proc->prev;
            } else {
                ready_queue_tail = proc->prev;
            }
            break;
            
        case SCHED_PRIORITY:
            /* Remove from priority queue */
            if (proc->priority < 5) {
                struct process **queue = &priority_queues[proc->priority];
                struct process *current = *queue;
                struct process *prev = NULL;
                
                while (current && current != proc) {
                    prev = current;
                    current = current->next;
                }
                
                if (current) {
                    if (prev) {
                        prev->next = current->next;
                    } else {
                        *queue = current->next;
                    }
                }
            }
            break;
    }
    
    proc->next = NULL;
    proc->prev = NULL;
    
    interrupts_enable();
}

/* Get next process to run */
struct process *scheduler_next(void) {
    struct process *next = NULL;
    
    switch (current_algorithm) {
        case SCHED_ROUND_ROBIN:
            next = ready_queue_head;
            if (next) {
                scheduler_remove_process(next);
            }
            break;
            
        case SCHED_PRIORITY:
            /* Find highest priority process */
            for (int i = PRIORITY_REALTIME; i >= PRIORITY_IDLE; i--) {
                if (priority_queues[i]) {
                    next = priority_queues[i];
                    priority_queues[i] = next->next;
                    next->next = NULL;
                    break;
                }
            }
            break;
            
        case SCHED_CFS:
            /* Get process with least CPU time */
            next = ready_queue_head;
            if (next) {
                scheduler_remove_process(next);
            }
            break;
    }
    
    return next;
}

/* Perform context switch */
void context_switch(struct process *from, struct process *to) {
    if (!to) return;
    
    serial_puts("[MATRIX] Neural context switch: ");
    if (from) {
        serial_puts(from->name);
        serial_puts(" -> ");
    }
    serial_puts(to->name);
    serial_puts("\\n");
    
    /* Update statistics */
    struct scheduler_stats *stats = get_scheduler_stats();
    stats->context_switches++;
    
    if (from) {
        from->context_switches++;
        from->state = (from->state == PROCESS_RUNNING) ? PROCESS_READY : from->state;
    }
    
    to->state = PROCESS_RUNNING;
    to->last_scheduled = timer_get_ticks();
    running_process = to;
    
    /* Perform actual context switch */
    if (from && to) {
        context_switch_asm(&from->context, &to->context);
    } else if (to) {
        context_switch_asm(NULL, &to->context);
    }
}

/* Yield CPU voluntarily */
void scheduler_yield(void) {
    struct process *current = process_get_current();
    if (!current) return;
    
    serial_puts("[SCHEDULER] Process yielding neural processing: ");
    serial_puts(current->name);
    serial_puts("\\n");
    
    /* Add current process back to ready queue if it's still ready */
    if (current->state == PROCESS_RUNNING) {
        current->state = PROCESS_READY;
        scheduler_add_process(current);
    }
    
    /* Get next process */
    struct process *next = scheduler_next();
    if (next) {
        context_switch(current, next);
    }
}

/* Preemptive scheduling */
void scheduler_preempt(void) {
    struct process *current = process_get_current();
    if (!current) return;
    
    /* Check if time slice expired */
    time_slice_counter++;
    if (time_slice_counter >= current->time_slice) {
        serial_puts("[SCHEDULER] Quantum time slice expired - preempting\\n");
        time_slice_counter = 0;
        scheduler_yield();
    }
}

/* Scheduler tick (called from timer interrupt) */
void scheduler_tick(void) {
    uint64_t current_time = timer_get_ticks();
    struct process *current = process_get_current();
    
    /* Update current process CPU time */
    if (current && current->state == PROCESS_RUNNING) {
        current->cpu_time += current_time - last_schedule_time;
    }
    
    /* Wake up sleeping processes */
    for (int i = 0; i < MAX_PROCESSES; i++) {
        extern struct process *process_table[];
        struct process *proc = process_table[i];
        if (proc && proc->state == PROCESS_SLEEPING) {
            if (current_time >= proc->sleep_until) {
                process_wake(proc);
                scheduler_add_process(proc);
            }
        }
    }
    
    /* Preemptive scheduling */
    scheduler_preempt();
    
    last_schedule_time = current_time;
}

/* Set scheduling algorithm */
void scheduler_set_algorithm(scheduler_algorithm_t algorithm) {
    serial_puts("[SCHEDULER] Switching neural algorithm to: ");
    
    current_algorithm = algorithm;
    
    switch (algorithm) {
        case SCHED_ROUND_ROBIN:
            serial_puts("Round Robin Quantum Processing\\n");
            break;
        case SCHED_PRIORITY:
            serial_puts("Priority-Based Neural Scheduling\\n");
            break;
        case SCHED_CFS:
            serial_puts("Completely Fair Scheduling Matrix\\n");
            break;
        default:
            serial_puts("Unknown Algorithm\\n");
            break;
    }
    
    /* Reinitialize scheduler with new algorithm */
    scheduler_init();
}