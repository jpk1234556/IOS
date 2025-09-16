/* uefi_boot.h - Brandon Media OS UEFI Boot Support
 * Cyberpunk Secure Boot Interface
 */

#ifndef UEFI_BOOT_H
#define UEFI_BOOT_H

#include <stdint.h>
#include <stddef.h>

/* UEFI Standard Data Types */
typedef uint8_t   BOOLEAN;
typedef uint64_t  EFI_STATUS;
typedef void*     EFI_HANDLE;

/* Forward declarations */
struct EFI_SYSTEM_TABLE;
struct EFI_MEMORY_DESCRIPTOR;

/* UEFI Status Codes */
#define EFI_SUCCESS               0x0000000000000000ULL
#define EFI_LOAD_ERROR            0x8000000000000001ULL
#define EFI_INVALID_PARAMETER     0x8000000000000002ULL
#define EFI_UNSUPPORTED           0x8000000000000003ULL
#define EFI_SECURITY_VIOLATION    0x800000000000001AULL

/* UEFI GUID Structure */
typedef struct {
    uint32_t Data1;
    uint16_t Data2;
    uint16_t Data3;
    uint8_t  Data4[8];
} EFI_GUID;

/* Neural Secure Boot Signature */
typedef struct {
    EFI_GUID   neural_guid;
    uint64_t   neural_signature;
    uint64_t   cyberpunk_hash;
    uint32_t   security_level;
    uint32_t   validation_status;
    uint8_t    neural_key[32];
    char       brandon_media_signature[64];
} NEURAL_SECURE_BOOT_SIG;

/* Cyberpunk Boot Information */
typedef struct {
    uint32_t   version;
    uint32_t   boot_mode;        /* 0=Legacy, 1=UEFI, 2=Neural */
    uint64_t   neural_timestamp;
    uint32_t   security_features;
    uint32_t   cyberpunk_level;
    EFI_GUID   system_guid;
    char       neural_id[32];
    uint64_t   matrix_checksum;
} CYBERPUNK_BOOT_INFO;

/* Neural Boot Parameters */
typedef struct {
    CYBERPUNK_BOOT_INFO           boot_info;
    NEURAL_SECURE_BOOT_SIG        secure_sig;
    struct EFI_MEMORY_DESCRIPTOR  *memory_map;
    uint64_t                      memory_map_size;
    uint64_t                      memory_map_key;
    uint64_t                      descriptor_size;
    uint32_t                      descriptor_version;
    void                          *framebuffer_base;
    uint64_t                      framebuffer_size;
    uint32_t                      screen_width;
    uint32_t                      screen_height;
    uint32_t                      pixels_per_scanline;
} NEURAL_BOOT_PARAMS;

/* UEFI Function Prototypes */

/* Main UEFI entry point */
EFI_STATUS efi_main(EFI_HANDLE ImageHandle, struct EFI_SYSTEM_TABLE *SystemTable);

/* Secure Boot Functions */
EFI_STATUS neural_secure_boot_init(NEURAL_SECURE_BOOT_SIG *sig);
EFI_STATUS neural_validate_secure_boot(NEURAL_SECURE_BOOT_SIG *sig);

/* Boot Information Functions */
EFI_STATUS neural_boot_info_init(CYBERPUNK_BOOT_INFO *info);
NEURAL_BOOT_PARAMS* neural_get_boot_params(void);
struct EFI_SYSTEM_TABLE* neural_get_system_table(void);

/* Boot Status Functions */
BOOLEAN neural_is_uefi_boot(void);
const char* neural_get_boot_mode_string(void);
const char* neural_get_security_level_string(void);

/* Memory and Graphics Functions */
EFI_STATUS neural_get_memory_map(NEURAL_BOOT_PARAMS *params);
EFI_STATUS neural_setup_graphics(NEURAL_BOOT_PARAMS *params);
EFI_STATUS neural_exit_boot_services(EFI_HANDLE image_handle, uint64_t map_key);

/* Display Functions */
void neural_display_boot_screen(void);
void neural_uefi_print(const char *message);
void neural_uefi_error(const char *error);
void neural_uefi_status(const char *status);

/* Testing Functions */
void neural_uefi_test(void);

#endif /* UEFI_BOOT_H */