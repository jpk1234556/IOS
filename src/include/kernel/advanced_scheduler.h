/* advanced_scheduler.h - Brandon Media OS Advanced Scheduling System
 * Neural Task Prioritization and Cyberpunk Fair Scheduling
 */

#ifndef KERNEL_ADVANCED_SCHEDULER_H
#define KERNEL_ADVANCED_SCHEDULER_H

#include <stdint.h>
#include "kernel/process.h"

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

/* Advanced Scheduler Function Prototypes */
void advanced_scheduler_init(void);
void sched_init_process_stats(struct process *proc);
void sched_update_runtime_stats(struct process *proc, uint64_t runtime);
void sched_add_process_to_runqueue(struct process *proc, uint8_t cpu_id);
void sched_remove_process_from_runqueue(struct process *proc, uint8_t cpu_id);
struct process *sched_schedule_next(uint8_t cpu_id);
void sched_set_algorithm(scheduling_algorithm_t algorithm);
void sched_print_statistics(void);
void sched_test_algorithms(void);
scheduling_algorithm_t sched_get_current_algorithm(void);
int sched_is_advanced_initialized(void);

#endif /* KERNEL_ADVANCED_SCHEDULER_H */