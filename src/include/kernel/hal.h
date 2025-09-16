/* hal.h - Brandon Media OS Hardware Abstraction Layer
 * Neural Device Management Interface
 */

#ifndef KERNEL_HAL_H
#define KERNEL_HAL_H

#include <stdint.h>
#include "kernel/pci.h"

/* Device Types */
typedef enum {
    DEVICE_TYPE_UNKNOWN = 0,
    DEVICE_TYPE_NETWORK,
    DEVICE_TYPE_STORAGE,
    DEVICE_TYPE_DISPLAY,
    DEVICE_TYPE_USB,
    DEVICE_TYPE_AUDIO,
    DEVICE_TYPE_INPUT
} device_type_t;

/* Device Status */
typedef enum {
    DEVICE_STATUS_UNKNOWN = 0,
    DEVICE_STATUS_DETECTED,
    DEVICE_STATUS_INITIALIZING,
    DEVICE_STATUS_ACTIVE,
    DEVICE_STATUS_ERROR,
    DEVICE_STATUS_DISABLED
} device_status_t;

/* Generic Device Structure */
struct hal_device {
    uint32_t device_id;
    device_type_t type;
    device_status_t status;
    const char *name;
    const char *manufacturer;
    
    /* PCI device reference (if applicable) */
    struct pci_device *pci_dev;
    
    /* Device-specific data */
    void *device_data;
    
    /* Device operations */
    int (*init)(struct hal_device *dev);
    int (*start)(struct hal_device *dev);
    int (*stop)(struct hal_device *dev);
    int (*reset)(struct hal_device *dev);
    void (*cleanup)(struct hal_device *dev);
    
    /* Linked list */
    struct hal_device *next;
};

/* HAL Function Prototypes */
void hal_init(void);
void hal_discover_pci_devices(void);
struct hal_device *hal_create_device(device_type_t type, const char *name, const char *manufacturer);
int hal_register_device(struct hal_device *device);
int hal_unregister_device(uint32_t device_id);
struct hal_device *hal_find_device_by_id(uint32_t device_id);
int hal_find_devices_by_type(device_type_t type, struct hal_device **devices, int max_devices);
int hal_initialize_device(uint32_t device_id);
int hal_start_device(uint32_t device_id);
int hal_stop_device(uint32_t device_id);
int hal_reset_device(uint32_t device_id);
void hal_print_device_info(struct hal_device *device);
void hal_print_all_devices(void);
int hal_get_device_count(void);
void hal_initialize_all_devices(void);

#endif /* KERNEL_HAL_H */