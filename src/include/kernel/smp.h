/* smp.h - Brandon Media OS SMP (Symmetric Multiprocessing) Support
 * Neural CPU Matrix Management System
 */

#ifndef KERNEL_SMP_H
#define KERNEL_SMP_H

#include <stdint.h>
#include "kernel/process.h"

/* CPU Status Codes */
#define CPU_STATUS_OFFLINE      0x00
#define CPU_STATUS_INITIALIZING 0x01
#define CPU_STATUS_ONLINE       0x02
#define CPU_STATUS_ERROR        0x03
#define CPU_STATUS_NEURAL_SYNC  0x04

/* Neural CPU Structure */
struct neural_cpu {
    uint32_t apic_id;
    uint8_t cpu_id;
    uint8_t status;
    uint64_t neural_frequency;
    uint64_t neural_cycles;
    struct process *current_process;
    struct process *idle_process;
    uint64_t load_average;
    uint32_t interrupts_handled;
    const char *neural_designation;
    int online;
} __attribute__((aligned(64)));

/* SMP Function Prototypes */
void smp_init(void);
struct neural_cpu *smp_get_current_cpu(void);
struct neural_cpu *smp_get_cpu_by_id(uint8_t cpu_id);
void smp_update_cpu_stats(struct neural_cpu *cpu);
void smp_print_neural_matrix_status(void);
void smp_send_ipi(uint8_t cpu_id, uint8_t vector);
void smp_broadcast_ipi(uint8_t vector);
void smp_get_statistics(uint32_t *total_cores, uint32_t *active_cores, uint64_t *total_cycles);
int smp_is_available(void);
uint32_t smp_get_cpu_count(void);
uint32_t smp_get_active_cpu_count(void);

/* IPI Vectors */
#define IPI_VECTOR_RESCHEDULE   0xF0
#define IPI_VECTOR_TLB_FLUSH    0xF1
#define IPI_VECTOR_HALT         0xF2

#endif /* KERNEL_SMP_H */