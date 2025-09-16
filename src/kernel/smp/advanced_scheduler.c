/* advanced_scheduler.c - Brandon Media OS Advanced Scheduling System
 * Neural Task Prioritization and Cyberpunk Fair Scheduling
 */

#include <stdint.h>
#include <stddef.h>
#include "kernel/memory.h"
#include "kernel/process.h"
#include "kernel/smp.h"

/* Scheduling Algorithms */
typedef enum {
    SCHED_ALGORITHM_ROUND_ROBIN = 0,
    SCHED_ALGORITHM_CFS,        /* Completely Fair Scheduler */
    SCHED_ALGORITHM_REALTIME,   /* Real-time scheduling */
    SCHED_ALGORITHM_NEURAL,     /* Neural adaptive scheduling */
    SCHED_ALGORITHM_CYBERPUNK   /* Cyberpunk priority scheduling */
} scheduling_algorithm_t;

/* Neural Process Priority Classes */
typedef enum {
    NEURAL_PRIORITY_CRITICAL = 0,    /* System critical processes */
    NEURAL_PRIORITY_HIGH = 1,        /* High priority processes */
    NEURAL_PRIORITY_NORMAL = 2,      /* Normal priority processes */
    NEURAL_PRIORITY_LOW = 3,         /* Low priority processes */
    NEURAL_PRIORITY_BACKGROUND = 4,  /* Background processes */
    NEURAL_PRIORITY_IDLE = 5         /* Idle processes */
} neural_priority_t;

/* Process Scheduling Statistics */
struct process_sched_stats {
    uint64_t runtime;               /* Total runtime in nanoseconds */
    uint64_t wait_time;            /* Total wait time */
    uint64_t neural_cycles;        /* Neural processing cycles used */
    uint32_t context_switches;     /* Number of context switches */
    uint32_t preemptions;          /* Number of preemptions */
    uint64_t last_runtime;         /* Last execution time */
    uint64_t vruntime;             /* Virtual runtime (CFS) */
    neural_priority_t neural_class; /* Neural priority class */
    int nice_value;                /* Nice value (-20 to 19) */
    uint64_t deadline;             /* Real-time deadline */
    uint32_t neural_affinity;      /* CPU affinity mask */
};

/* Per-CPU Run Queue */
struct neural_runqueue {
    struct process *active_queue[6];    /* One queue per priority */
    struct process *expired_queue[6];   /* Expired processes */
    uint32_t queue_bitmap;             /* Bitmap of non-empty queues */
    uint32_t process_count;            /* Total processes in runqueue */
    uint64_t total_weight;             /* Total weight for CFS */
    struct process *current;           /* Currently running process */
    struct process *idle_task;         /* Idle task for this CPU */
    uint64_t clock;                    /* Scheduler clock */
    uint64_t load_average[3];          /* 1, 5, 15 minute load averages */
    scheduling_algorithm_t algorithm;   /* Current scheduling algorithm */
    uint32_t time_slice;               /* Current time slice */
};

/* Global Scheduler State */
static struct neural_runqueue neural_runqueues[64];  /* Per-CPU runqueues */
static scheduling_algorithm_t global_algorithm = SCHED_ALGORITHM_NEURAL;
static uint64_t scheduler_frequency = 1000;  /* 1000 Hz */
static uint64_t neural_time_base = 0;
static int advanced_scheduler_initialized = 0;

/* External functions */
extern void serial_puts(const char *s);
extern void print_hex(uint64_t num);
extern void print_dec(uint64_t num);
extern uint64_t get_system_time(void);

/* Priority to nice value conversion */
static int priority_to_nice(process_priority_t priority) {
    switch (priority) {
        case PRIORITY_CRITICAL: return -20;
        case PRIORITY_HIGH:     return -10;
        case PRIORITY_NORMAL:   return 0;
        case PRIORITY_LOW:      return 10;
        case PRIORITY_IDLE:     return 19;
        default:                return 0;
    }
}

/* Nice value to weight conversion (CFS) */
static uint64_t nice_to_weight(int nice) {
    static const uint64_t weights[] = {
        88761, 71755, 56483, 46273, 36291,
        29154, 23254, 18705, 14949, 11916,
        9548, 7620, 6100, 4904, 3906,
        3121, 2501, 1991, 1586, 1277,
        1024, 820, 655, 526, 423,
        335, 272, 215, 172, 137,
        110, 87, 70, 56, 45,
        36, 29, 23, 18, 15
    };
    
    int index = nice + 20;
    if (index < 0) index = 0;
    if (index >= 40) index = 39;
    
    return weights[index];
}

/* Neural priority to weight conversion */
static uint64_t neural_priority_to_weight(neural_priority_t priority) {
    switch (priority) {
        case NEURAL_PRIORITY_CRITICAL:    return 88761;
        case NEURAL_PRIORITY_HIGH:        return 29154;
        case NEURAL_PRIORITY_NORMAL:      return 1024;
        case NEURAL_PRIORITY_LOW:         return 335;
        case NEURAL_PRIORITY_BACKGROUND:  return 110;
        case NEURAL_PRIORITY_IDLE:        return 15;
        default:                          return 1024;
    }
}

/* Get scheduling algorithm name */
static const char *get_algorithm_name(scheduling_algorithm_t algorithm) {
    switch (algorithm) {
        case SCHED_ALGORITHM_ROUND_ROBIN: return "Neural Round Robin";
        case SCHED_ALGORITHM_CFS:         return "Cyberpunk Fair Scheduler";
        case SCHED_ALGORITHM_REALTIME:    return "Neural Real-Time";
        case SCHED_ALGORITHM_NEURAL:      return "Neural Adaptive";
        case SCHED_ALGORITHM_CYBERPUNK:   return "Cyberpunk Priority";
        default:                          return "Unknown Algorithm";
    }
}

/* Initialize process scheduling statistics */
void sched_init_process_stats(struct process *proc) {
    if (!proc || !proc->sched_stats) return;
    
    struct process_sched_stats *stats = proc->sched_stats;
    stats->runtime = 0;
    stats->wait_time = 0;
    stats->neural_cycles = 0;
    stats->context_switches = 0;
    stats->preemptions = 0;
    stats->last_runtime = get_system_time();
    stats->vruntime = 0;
    stats->neural_class = NEURAL_PRIORITY_NORMAL;
    stats->nice_value = priority_to_nice(proc->priority);
    stats->deadline = 0;
    stats->neural_affinity = 0xFFFFFFFF;  /* All CPUs */
}

/* Update process runtime statistics */
void sched_update_runtime_stats(struct process *proc, uint64_t runtime) {
    if (!proc || !proc->sched_stats) return;
    
    struct process_sched_stats *stats = proc->sched_stats;
    stats->runtime += runtime;
    stats->neural_cycles += runtime / 1000;  /* Approximate cycles */
    
    /* Update virtual runtime for CFS */
    uint64_t weight = nice_to_weight(stats->nice_value);
    stats->vruntime += (runtime * 1024) / weight;
}

/* CFS (Completely Fair Scheduler) Implementation */
static struct process *cfs_pick_next_task(struct neural_runqueue *rq) {
    struct process *best_proc = NULL;
    uint64_t min_vruntime = UINT64_MAX;
    
    /* Find process with minimum virtual runtime */
    for (int priority = 0; priority < 6; priority++) {
        struct process *proc = rq->active_queue[priority];
        while (proc) {
            if (proc->sched_stats && proc->sched_stats->vruntime < min_vruntime) {
                min_vruntime = proc->sched_stats->vruntime;
                best_proc = proc;
            }
            proc = proc->next_in_queue;
        }
    }
    
    return best_proc;
}

/* Neural Adaptive Scheduler */
static struct process *neural_pick_next_task(struct neural_runqueue *rq) {
    uint64_t current_time = get_system_time();
    struct process *best_proc = NULL;
    uint64_t best_score = 0;
    
    /* Neural scoring algorithm combining priority, runtime, and load */
    for (int priority = 0; priority < 6; priority++) {
        struct process *proc = rq->active_queue[priority];
        while (proc) {
            if (proc->sched_stats) {
                uint64_t wait_time = current_time - proc->sched_stats->last_runtime;
                uint64_t neural_weight = neural_priority_to_weight(proc->sched_stats->neural_class);
                uint64_t score = (wait_time * neural_weight) / (proc->sched_stats->runtime + 1);
                
                if (score > best_score) {
                    best_score = score;
                    best_proc = proc;
                }
            }
            proc = proc->next_in_queue;
        }
    }
    
    return best_proc;
}

/* Cyberpunk Priority Scheduler */
static struct process *cyberpunk_pick_next_task(struct neural_runqueue *rq) {
    /* Find highest priority non-empty queue */
    for (int priority = 0; priority < 6; priority++) {
        if (rq->active_queue[priority]) {
            struct process *proc = rq->active_queue[priority];
            
            /* Neural enhancement: prefer processes with cyberpunk attributes */
            struct process *best_proc = proc;
            while (proc) {
                if (proc->name && (
                    strstr(proc->name, "neural") || 
                    strstr(proc->name, "cyber") || 
                    strstr(proc->name, "matrix"))) {
                    best_proc = proc;
                    break;
                }
                proc = proc->next_in_queue;
            }
            
            return best_proc;
        }
    }
    
    return NULL;
}

/* Real-time scheduler */
static struct process *realtime_pick_next_task(struct neural_runqueue *rq) {
    uint64_t current_time = get_system_time();
    struct process *earliest_deadline = NULL;
    uint64_t earliest_time = UINT64_MAX;
    
    /* Earliest Deadline First (EDF) */
    for (int priority = 0; priority < 2; priority++) {  /* Only RT priorities */
        struct process *proc = rq->active_queue[priority];
        while (proc) {
            if (proc->sched_stats && proc->sched_stats->deadline > 0) {
                if (proc->sched_stats->deadline < earliest_time) {
                    earliest_time = proc->sched_stats->deadline;
                    earliest_deadline = proc;
                }
            }
            proc = proc->next_in_queue;
        }
    }
    
    return earliest_deadline;
}

/* Pick next task based on current algorithm */
static struct process *pick_next_task(struct neural_runqueue *rq) {
    switch (rq->algorithm) {
        case SCHED_ALGORITHM_CFS:
            return cfs_pick_next_task(rq);
        case SCHED_ALGORITHM_REALTIME:
            return realtime_pick_next_task(rq);
        case SCHED_ALGORITHM_NEURAL:
            return neural_pick_next_task(rq);
        case SCHED_ALGORITHM_CYBERPUNK:
            return cyberpunk_pick_next_task(rq);
        case SCHED_ALGORITHM_ROUND_ROBIN:
        default:
            /* Simple round-robin fallback */
            for (int priority = 0; priority < 6; priority++) {
                if (rq->active_queue[priority]) {
                    return rq->active_queue[priority];
                }
            }
            return NULL;
    }
}

/* Add process to runqueue */
void sched_add_process_to_runqueue(struct process *proc, uint8_t cpu_id) {
    if (!proc || cpu_id >= 64) return;
    
    struct neural_runqueue *rq = &neural_runqueues[cpu_id];
    int priority = (int)proc->priority;
    if (priority >= 6) priority = 5;
    
    /* Add to active queue */
    proc->next_in_queue = rq->active_queue[priority];
    rq->active_queue[priority] = proc;
    rq->process_count++;
    rq->queue_bitmap |= (1 << priority);
    
    /* Initialize scheduling stats if needed */
    if (!proc->sched_stats) {
        proc->sched_stats = kmalloc(sizeof(struct process_sched_stats));
        if (proc->sched_stats) {
            sched_init_process_stats(proc);
        }
    }
}

/* Remove process from runqueue */
void sched_remove_process_from_runqueue(struct process *proc, uint8_t cpu_id) {
    if (!proc || cpu_id >= 64) return;
    
    struct neural_runqueue *rq = &neural_runqueues[cpu_id];
    int priority = (int)proc->priority;
    if (priority >= 6) priority = 5;
    
    /* Remove from active queue */
    if (rq->active_queue[priority] == proc) {
        rq->active_queue[priority] = proc->next_in_queue;
    } else {
        struct process *prev = rq->active_queue[priority];
        while (prev && prev->next_in_queue != proc) {
            prev = prev->next_in_queue;
        }
        if (prev) {
            prev->next_in_queue = proc->next_in_queue;
        }
    }
    
    proc->next_in_queue = NULL;
    rq->process_count--;
    
    /* Update queue bitmap */
    if (!rq->active_queue[priority]) {
        rq->queue_bitmap &= ~(1 << priority);
    }
}

/* Schedule next process */
struct process *sched_schedule_next(uint8_t cpu_id) {
    if (cpu_id >= 64) return NULL;
    
    struct neural_runqueue *rq = &neural_runqueues[cpu_id];
    struct process *next_proc = pick_next_task(rq);
    
    if (next_proc) {
        rq->current = next_proc;
        rq->clock = get_system_time();
        
        /* Update scheduling statistics */
        if (next_proc->sched_stats) {
            next_proc->sched_stats->context_switches++;
            next_proc->sched_stats->last_runtime = rq->clock;
        }
    } else {
        rq->current = rq->idle_task;
    }
    
    return rq->current;
}

/* Set scheduling algorithm */
void sched_set_algorithm(scheduling_algorithm_t algorithm) {
    global_algorithm = algorithm;
    
    /* Update all CPU runqueues */
    uint32_t cpu_count = smp_get_cpu_count();
    for (uint32_t i = 0; i < cpu_count; i++) {
        neural_runqueues[i].algorithm = algorithm;
    }
    
    serial_puts("[NEURAL-SCHED] Algorithm changed to: ");
    serial_puts(get_algorithm_name(algorithm));
    serial_puts("\n");
}

/* Print scheduling statistics */
void sched_print_statistics(void) {
    serial_puts("[NEURAL-SCHED] === Advanced Scheduling Statistics ===\n");
    
    uint32_t cpu_count = smp_get_cpu_count();
    for (uint32_t i = 0; i < cpu_count; i++) {
        struct neural_runqueue *rq = &neural_runqueues[i];
        
        serial_puts("[CPU-");
        print_dec(i);
        serial_puts("] Algorithm: ");
        serial_puts(get_algorithm_name(rq->algorithm));
        serial_puts("\n");
        
        serial_puts("[CPU-");
        print_dec(i);
        serial_puts("] Processes: ");
        print_dec(rq->process_count);
        serial_puts("\n");
        
        serial_puts("[CPU-");
        print_dec(i);
        serial_puts("] Current: ");
        if (rq->current) {
            serial_puts(rq->current->name);
        } else {
            serial_puts("None");
        }
        serial_puts("\n");
        
        serial_puts("[CPU-");
        print_dec(i);
        serial_puts("] Load Average: ");
        print_dec(rq->load_average[0]);
        serial_puts("\n\n");
    }
    
    serial_puts("[NEURAL-SCHED] === End Statistics ===\n");
}

/* Initialize advanced scheduler */
void advanced_scheduler_init(void) {
    serial_puts("[NEURAL-SCHED] Initializing Advanced Neural Scheduling System...\n");
    
    uint32_t cpu_count = smp_get_cpu_count();
    
    /* Initialize per-CPU runqueues */
    for (uint32_t i = 0; i < cpu_count; i++) {
        struct neural_runqueue *rq = &neural_runqueues[i];
        
        /* Clear all queues */
        for (int j = 0; j < 6; j++) {
            rq->active_queue[j] = NULL;
            rq->expired_queue[j] = NULL;
        }
        
        rq->queue_bitmap = 0;
        rq->process_count = 0;
        rq->total_weight = 0;
        rq->current = NULL;
        rq->idle_task = NULL;
        rq->clock = get_system_time();
        rq->algorithm = global_algorithm;
        rq->time_slice = 1000000000ULL / scheduler_frequency;  /* 1ms default */
        
        /* Initialize load averages */
        for (int j = 0; j < 3; j++) {
            rq->load_average[j] = 0;
        }
    }
    
    neural_time_base = get_system_time();
    advanced_scheduler_initialized = 1;
    
    serial_puts("[NEURAL-SCHED] Advanced scheduling system initialized\n");
    serial_puts("[NEURAL-SCHED] Default algorithm: ");
    serial_puts(get_algorithm_name(global_algorithm));
    serial_puts("\n");
    serial_puts("[NEURAL-SCHED] Scheduler frequency: ");
    print_dec(scheduler_frequency);
    serial_puts(" Hz\n");
}

/* Test advanced scheduling features */
void sched_test_algorithms(void) {
    serial_puts("[NEURAL-SCHED] Testing advanced scheduling algorithms...\n");
    
    scheduling_algorithm_t algorithms[] = {
        SCHED_ALGORITHM_ROUND_ROBIN,
        SCHED_ALGORITHM_CFS,
        SCHED_ALGORITHM_NEURAL,
        SCHED_ALGORITHM_CYBERPUNK,
        SCHED_ALGORITHM_REALTIME
    };
    
    for (int i = 0; i < 5; i++) {
        serial_puts("[TEST] Testing ");
        serial_puts(get_algorithm_name(algorithms[i]));
        serial_puts("...\n");
        
        sched_set_algorithm(algorithms[i]);
        
        /* Small delay to simulate testing */
        for (volatile int j = 0; j < 1000000; j++);
        
        serial_puts("[TEST] ");
        serial_puts(get_algorithm_name(algorithms[i]));
        serial_puts(" test completed\n");
    }
    
    /* Reset to neural algorithm */
    sched_set_algorithm(SCHED_ALGORITHM_NEURAL);
    
    serial_puts("[NEURAL-SCHED] Algorithm testing completed\n");
}

/* Get current scheduling algorithm */
scheduling_algorithm_t sched_get_current_algorithm(void) {
    return global_algorithm;
}

/* Check if advanced scheduler is initialized */
int sched_is_advanced_initialized(void) {
    return advanced_scheduler_initialized;
}