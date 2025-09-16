/* security.h - Brandon Media OS Security Framework
 * Neural Protection Systems and Cyberpunk Security Features
 */

#ifndef KERNEL_SECURITY_H
#define KERNEL_SECURITY_H

#include <stdint.h>
#include "kernel/process.h"

/* Security Feature Flags */
#define SECURITY_FEATURE_ASLR           0x01    /* Address Space Layout Randomization */
#define SECURITY_FEATURE_STACK_CANARY   0x02    /* Stack canary protection */
#define SECURITY_FEATURE_NX_BIT         0x04    /* No-Execute bit protection */
#define SECURITY_FEATURE_SMEP           0x08    /* Supervisor Mode Execution Prevention */
#define SECURITY_FEATURE_SMAP           0x10    /* Supervisor Mode Access Prevention */
#define SECURITY_FEATURE_NEURAL_GUARD   0x20    /* Neural memory guard */
#define SECURITY_FEATURE_CYBER_SHIELD   0x40    /* Cyberpunk process isolation */

/* Security Context Structure */
struct security_context {
    uint32_t security_flags;
    uint64_t stack_canary;
    uint64_t neural_canary;
    uint64_t aslr_base;
    uint32_t privilege_level;
    uint64_t memory_bounds_low;
    uint64_t memory_bounds_high;
    uint32_t access_permissions;
    uint64_t neural_signature;
    const char *security_profile;
};

/* Security Function Prototypes */
void security_init(void);
int security_verify_stack_canary(uint64_t canary);
int security_verify_neural_canary(uint64_t canary);
uint64_t security_get_aslr_offset(void);
uint64_t security_apply_aslr(uint64_t base_address);
int security_check_memory_bounds(uint64_t address, size_t size);
uint64_t security_generate_neural_signature(struct process *proc);
int security_validate_neural_signature(struct process *proc, uint64_t signature);
int security_init_process_context(struct process *proc);
void security_cleanup_process_context(struct process *proc);
int security_check_privilege(struct process *proc, uint32_t required_level);
void security_enable_feature(uint32_t feature);
void security_disable_feature(uint32_t feature);
void security_print_status(void);
void security_test_features(void);
struct security_context *security_get_global_context(void);
int security_is_initialized(void);
uint32_t security_get_enabled_features(void);

#endif /* KERNEL_SECURITY_H */