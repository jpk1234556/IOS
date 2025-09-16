/* uefi_manager.c - Brandon Media OS UEFI Boot Manager
 * Cyberpunk Boot Management and Integration
 */

#include <stdint.h>
#include <stddef.h>
#include "kernel/uefi_boot.h"
#include "kernel/memory.h"

/* Boot Manager State */
typedef struct {
    BOOLEAN             initialized;
    BOOLEAN             uefi_available;
    BOOLEAN             secure_boot_enabled;
    uint32_t            boot_stage;
    NEURAL_BOOT_PARAMS  *boot_params;
    char                boot_log[1024];
    int                 log_position;
} UEFI_BOOT_MANAGER;

static UEFI_BOOT_MANAGER boot_manager;

/* External functions */
extern void serial_puts(const char *s);
extern void print_hex(uint64_t num);
extern void print_dec(uint64_t num);

/* Boot Manager Functions */

void uefi_manager_init(void) {
    serial_puts("[UEFI-MGR] Initializing UEFI Boot Manager...\n");
    
    /* Clear boot manager state */
    boot_manager.initialized = 0;
    boot_manager.uefi_available = 0;
    boot_manager.secure_boot_enabled = 0;
    boot_manager.boot_stage = 0;
    boot_manager.boot_params = NULL;
    boot_manager.log_position = 0;
    
    /* Clear boot log */
    for (int i = 0; i < 1024; i++) {
        boot_manager.boot_log[i] = 0;
    }
    
    /* Check if UEFI boot was used */
    if (neural_is_uefi_boot()) {
        boot_manager.uefi_available = 1;
        boot_manager.boot_params = neural_get_boot_params();
        
        if (boot_manager.boot_params && 
            boot_manager.boot_params->secure_sig.validation_status) {
            boot_manager.secure_boot_enabled = 1;
        }
        
        serial_puts("[UEFI-MGR] UEFI boot detected\n");
    } else {
        serial_puts("[UEFI-MGR] Legacy BIOS boot detected\n");
    }
    
    boot_manager.initialized = 1;
    serial_puts("[UEFI-MGR] UEFI Boot Manager initialized\n");
}

void uefi_manager_log(const char *message) {
    if (!boot_manager.initialized) return;
    
    /* Add message to boot log */
    int msg_len = 0;
    while (message[msg_len] && msg_len < 100) msg_len++;  /* Get length */
    
    if (boot_manager.log_position + msg_len + 1 < 1024) {
        for (int i = 0; i < msg_len; i++) {
            boot_manager.boot_log[boot_manager.log_position++] = message[i];
        }
        boot_manager.boot_log[boot_manager.log_position++] = '\n';
    }
}

void uefi_manager_print_status(void) {
    serial_puts("[UEFI-MGR] === UEFI Boot Manager Status ===\n");
    
    serial_puts("[INFO] Manager Initialized: ");
    serial_puts(boot_manager.initialized ? "YES" : "NO");
    serial_puts("\n");
    
    serial_puts("[INFO] UEFI Available: ");
    serial_puts(boot_manager.uefi_available ? "YES" : "NO");
    serial_puts("\n");
    
    serial_puts("[INFO] Secure Boot: ");
    serial_puts(boot_manager.secure_boot_enabled ? "ENABLED" : "DISABLED");
    serial_puts("\n");
    
    serial_puts("[INFO] Boot Stage: ");
    print_dec(boot_manager.boot_stage);
    serial_puts("\n");
    
    if (boot_manager.uefi_available) {
        serial_puts("[INFO] Boot Mode: ");
        serial_puts(neural_get_boot_mode_string());
        serial_puts("\n");
        
        serial_puts("[INFO] Security Level: ");
        serial_puts(neural_get_security_level_string());
        serial_puts("\n");
        
        if (boot_manager.boot_params) {
            serial_puts("[INFO] Screen Resolution: ");
            print_dec(boot_manager.boot_params->screen_width);
            serial_puts("x");
            print_dec(boot_manager.boot_params->screen_height);
            serial_puts("\n");
            
            serial_puts("[INFO] Framebuffer Size: ");
            print_hex(boot_manager.boot_params->framebuffer_size);
            serial_puts("\n");
            
            serial_puts("[INFO] Neural ID: ");
            serial_puts(boot_manager.boot_params->boot_info.neural_id);
            serial_puts("\n");
            
            serial_puts("[INFO] Cyberpunk Level: ");
            print_dec(boot_manager.boot_params->boot_info.cyberpunk_level);
            serial_puts("\n");
        }
    }
    
    serial_puts("[UEFI-MGR] === End Status ===\n");
}

void uefi_manager_print_boot_log(void) {
    if (!boot_manager.initialized) return;
    
    serial_puts("[UEFI-MGR] === Boot Log ===\n");
    
    if (boot_manager.log_position > 0) {
        /* Print boot log */
        for (int i = 0; i < boot_manager.log_position; i++) {
            char c = boot_manager.boot_log[i];
            if (c == '\n') {
                serial_puts("\n");
            } else {
                /* Output single character */
                char temp[2] = {c, 0};
                serial_puts(temp);
            }
        }
    } else {
        serial_puts("[INFO] Boot log is empty\n");
    }
    
    serial_puts("[UEFI-MGR] === End Boot Log ===\n");
}

BOOLEAN uefi_manager_is_uefi_boot(void) {
    return boot_manager.uefi_available;
}

BOOLEAN uefi_manager_is_secure_boot(void) {
    return boot_manager.secure_boot_enabled;
}

uint32_t uefi_manager_get_boot_stage(void) {
    return boot_manager.boot_stage;
}

void uefi_manager_set_boot_stage(uint32_t stage) {
    boot_manager.boot_stage = stage;
    uefi_manager_log("Boot stage updated");
}

NEURAL_BOOT_PARAMS* uefi_manager_get_boot_params(void) {
    return boot_manager.boot_params;
}

/* Memory Map Analysis */
void uefi_manager_analyze_memory_map(void) {
    if (!boot_manager.uefi_available || !boot_manager.boot_params) {
        serial_puts("[UEFI-MGR] Memory map analysis not available (Legacy boot)\n");
        return;
    }
    
    serial_puts("[UEFI-MGR] === Memory Map Analysis ===\n");
    serial_puts("[INFO] Memory Map Size: ");
    print_hex(boot_manager.boot_params->memory_map_size);
    serial_puts("\n");
    
    serial_puts("[INFO] Descriptor Size: ");
    print_hex(boot_manager.boot_params->descriptor_size);
    serial_puts("\n");
    
    serial_puts("[INFO] Map Key: ");
    print_hex(boot_manager.boot_params->memory_map_key);
    serial_puts("\n");
    
    serial_puts("[INFO] Descriptor Version: ");
    print_dec(boot_manager.boot_params->descriptor_version);
    serial_puts("\n");
    
    /* In a real implementation, we would iterate through memory descriptors */
    serial_puts("[INFO] Memory map contains UEFI memory layout\n");
    serial_puts("[UEFI-MGR] === End Memory Map Analysis ===\n");
}

/* Graphics Information */
void uefi_manager_print_graphics_info(void) {
    if (!boot_manager.uefi_available || !boot_manager.boot_params) {
        serial_puts("[UEFI-MGR] Graphics info not available (Legacy boot)\n");
        return;
    }
    
    serial_puts("[UEFI-MGR] === Graphics Information ===\n");
    serial_puts("[INFO] Screen Width: ");
    print_dec(boot_manager.boot_params->screen_width);
    serial_puts("\n");
    
    serial_puts("[INFO] Screen Height: ");
    print_dec(boot_manager.boot_params->screen_height);
    serial_puts("\n");
    
    serial_puts("[INFO] Pixels Per Scanline: ");
    print_dec(boot_manager.boot_params->pixels_per_scanline);
    serial_puts("\n");
    
    serial_puts("[INFO] Framebuffer Base: ");
    print_hex((uint64_t)boot_manager.boot_params->framebuffer_base);
    serial_puts("\n");
    
    serial_puts("[INFO] Framebuffer Size: ");
    print_hex(boot_manager.boot_params->framebuffer_size);
    serial_puts("\n");
    
    serial_puts("[UEFI-MGR] === End Graphics Information ===\n");
}

/* Secure Boot Information */
void uefi_manager_print_secure_boot_info(void) {
    serial_puts("[UEFI-MGR] === Secure Boot Information ===\n");
    
    if (!boot_manager.uefi_available) {
        serial_puts("[INFO] Secure Boot: Not Available (Legacy Boot)\n");
        serial_puts("[UEFI-MGR] === End Secure Boot Information ===\n");
        return;
    }
    
    if (!boot_manager.boot_params) {
        serial_puts("[INFO] Secure Boot: Configuration Error\n");
        serial_puts("[UEFI-MGR] === End Secure Boot Information ===\n");
        return;
    }
    
    NEURAL_SECURE_BOOT_SIG *sig = &boot_manager.boot_params->secure_sig;
    
    serial_puts("[INFO] Secure Boot: ");
    serial_puts(boot_manager.secure_boot_enabled ? "ENABLED" : "DISABLED");
    serial_puts("\n");
    
    serial_puts("[INFO] Neural Signature: ");
    print_hex(sig->neural_signature);
    serial_puts("\n");
    
    serial_puts("[INFO] Cyberpunk Hash: ");
    print_hex(sig->cyberpunk_hash);
    serial_puts("\n");
    
    serial_puts("[INFO] Security Level: ");
    print_dec(sig->security_level);
    serial_puts("\n");
    
    serial_puts("[INFO] Validation Status: ");
    serial_puts(sig->validation_status ? "VALIDATED" : "NOT_VALIDATED");
    serial_puts("\n");
    
    serial_puts("[INFO] Brandon Media Signature: ");
    serial_puts(sig->brandon_media_signature);
    serial_puts("\n");
    
    serial_puts("[UEFI-MGR] === End Secure Boot Information ===\n");
}

/* Test UEFI Manager Functions */
void uefi_manager_run_tests(void) {
    serial_puts("[UEFI-MGR] Running UEFI Manager Tests...\n");
    
    /* Test basic functionality */
    uefi_manager_log("UEFI Manager test started");
    uefi_manager_log("Testing boot parameter access");
    uefi_manager_log("Testing secure boot validation");
    uefi_manager_log("Testing memory map analysis");
    uefi_manager_log("UEFI Manager test completed");
    
    /* Print test results */
    uefi_manager_print_status();
    uefi_manager_print_boot_log();
    
    if (boot_manager.uefi_available) {
        uefi_manager_analyze_memory_map();
        uefi_manager_print_graphics_info();
        uefi_manager_print_secure_boot_info();
        
        /* Test UEFI functions */
        neural_uefi_test();
    }
    
    serial_puts("[UEFI-MGR] All tests completed\n");
}

/* Get manager status */
void uefi_manager_get_status(BOOLEAN *initialized, BOOLEAN *uefi_available, BOOLEAN *secure_boot) {
    if (initialized) *initialized = boot_manager.initialized;
    if (uefi_available) *uefi_available = boot_manager.uefi_available;
    if (secure_boot) *secure_boot = boot_manager.secure_boot_enabled;
}

/* Cyberpunk Boot Statistics */
void uefi_manager_print_cyberpunk_stats(void) {
    serial_puts("[UEFI-MGR] === Cyberpunk Neural Boot Statistics ===\n");
    
    if (boot_manager.uefi_available && boot_manager.boot_params) {
        CYBERPUNK_BOOT_INFO *info = &boot_manager.boot_params->boot_info;
        
        serial_puts("[CYBER] Boot Version: ");
        print_hex(info->version);
        serial_puts("\n");
        
        serial_puts("[CYBER] Neural Timestamp: ");
        print_hex(info->neural_timestamp);
        serial_puts("\n");
        
        serial_puts("[CYBER] Security Features: ");
        print_hex(info->security_features);
        serial_puts("\n");
        
        serial_puts("[CYBER] Cyberpunk Level: ");
        print_dec(info->cyberpunk_level);
        serial_puts("\n");
        
        serial_puts("[CYBER] Neural ID: ");
        serial_puts(info->neural_id);
        serial_puts("\n");
        
        serial_puts("[CYBER] Matrix Checksum: ");
        print_hex(info->matrix_checksum);
        serial_puts("\n");
    } else {
        serial_puts("[CYBER] Cyberpunk statistics not available in legacy mode\n");
    }
    
    serial_puts("[UEFI-MGR] === End Cyberpunk Statistics ===\n");
}