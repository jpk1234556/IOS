/* uefi_manager.h - Brandon Media OS UEFI Boot Manager
 * Cyberpunk Boot Management and Integration
 */

#ifndef UEFI_MANAGER_H
#define UEFI_MANAGER_H

#include <stdint.h>
#include "kernel/uefi_boot.h"

/* Boot Manager Functions */
void uefi_manager_init(void);
void uefi_manager_log(const char *message);
void uefi_manager_print_status(void);
void uefi_manager_print_boot_log(void);

/* Boot Information Functions */
BOOLEAN uefi_manager_is_uefi_boot(void);
BOOLEAN uefi_manager_is_secure_boot(void);
uint32_t uefi_manager_get_boot_stage(void);
void uefi_manager_set_boot_stage(uint32_t stage);
NEURAL_BOOT_PARAMS* uefi_manager_get_boot_params(void);

/* Analysis Functions */
void uefi_manager_analyze_memory_map(void);
void uefi_manager_print_graphics_info(void);
void uefi_manager_print_secure_boot_info(void);
void uefi_manager_print_cyberpunk_stats(void);

/* Testing Functions */
void uefi_manager_run_tests(void);
void uefi_manager_get_status(BOOLEAN *initialized, BOOLEAN *uefi_available, BOOLEAN *secure_boot);

#endif /* UEFI_MANAGER_H */