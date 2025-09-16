/* hal.c - Brandon Media OS Hardware Abstraction Layer
 * Neural Device Management Interface
 */

#include <stdint.h>
#include <stddef.h>
#include "kernel/memory.h"
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

/* Device Registry */
static struct hal_device *device_registry = NULL;
static uint32_t next_device_id = 1;
static int device_count = 0;

/* External functions */
extern void serial_puts(const char *s);
extern void print_hex(uint64_t num);
extern void print_dec(uint64_t num);

/* Forward declarations */
void hal_discover_pci_devices(void);

/* Get device type name */
static const char *hal_get_device_type_name(device_type_t type) {
    switch (type) {
        case DEVICE_TYPE_NETWORK: return "Neural Network Interface";
        case DEVICE_TYPE_STORAGE: return "Neural Storage Matrix";
        case DEVICE_TYPE_DISPLAY: return "Neural Display Interface";
        case DEVICE_TYPE_USB: return "Neural USB Controller";
        case DEVICE_TYPE_AUDIO: return "Neural Audio Processor";
        case DEVICE_TYPE_INPUT: return "Neural Input Device";
        default: return "Unknown Neural Device";
    }
}

/* Get device status name */
static const char *hal_get_device_status_name(device_status_t status) {
    switch (status) {
        case DEVICE_STATUS_DETECTED: return "Neural Signature Detected";
        case DEVICE_STATUS_INITIALIZING: return "Neural Interface Initializing";
        case DEVICE_STATUS_ACTIVE: return "Neural Interface Active";
        case DEVICE_STATUS_ERROR: return "Neural Interface Error";
        case DEVICE_STATUS_DISABLED: return "Neural Interface Disabled";
        default: return "Neural Status Unknown";
    }
}

/* Create a new HAL device */
struct hal_device *hal_create_device(device_type_t type, const char *name, const char *manufacturer) {
    struct hal_device *device = (struct hal_device *)kmalloc(sizeof(struct hal_device));
    if (!device) {
        return NULL;
    }
    
    device->device_id = next_device_id++;
    device->type = type;
    device->status = DEVICE_STATUS_DETECTED;
    device->name = name;
    device->manufacturer = manufacturer;
    device->pci_dev = NULL;
    device->device_data = NULL;
    
    /* Initialize function pointers to NULL */
    device->init = NULL;
    device->start = NULL;
    device->stop = NULL;
    device->reset = NULL;
    device->cleanup = NULL;
    
    device->next = NULL;
    
    return device;
}

/* Register a device in the HAL */
int hal_register_device(struct hal_device *device) {
    if (!device) {
        return -1;
    }
    
    /* Add to device registry */
    if (!device_registry) {
        device_registry = device;
    } else {
        struct hal_device *current = device_registry;
        while (current->next) {
            current = current->next;
        }
        current->next = device;
    }
    
    device_count++;
    
    serial_puts("[NEURAL-HAL] Device registered: ");
    serial_puts(device->name);
    serial_puts(" (ID: ");
    print_dec(device->device_id);
    serial_puts(")\n");
    
    return 0;
}

/* Unregister a device from the HAL */
int hal_unregister_device(uint32_t device_id) {
    if (!device_registry) {
        return -1;
    }
    
    /* Handle first device */
    if (device_registry->device_id == device_id) {
        struct hal_device *to_remove = device_registry;
        device_registry = device_registry->next;
        
        if (to_remove->cleanup) {
            to_remove->cleanup(to_remove);
        }
        
        kfree(to_remove);
        device_count--;
        return 0;
    }
    
    /* Handle other devices */
    struct hal_device *current = device_registry;
    while (current->next) {
        if (current->next->device_id == device_id) {
            struct hal_device *to_remove = current->next;
            current->next = to_remove->next;
            
            if (to_remove->cleanup) {
                to_remove->cleanup(to_remove);
            }
            
            kfree(to_remove);
            device_count--;
            return 0;
        }
        current = current->next;
    }
    
    return -1;
}

/* Find device by ID */
struct hal_device *hal_find_device_by_id(uint32_t device_id) {
    struct hal_device *current = device_registry;
    while (current) {
        if (current->device_id == device_id) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

/* Find devices by type */
int hal_find_devices_by_type(device_type_t type, struct hal_device **devices, int max_devices) {
    struct hal_device *current = device_registry;
    int count = 0;
    
    while (current && count < max_devices) {
        if (current->type == type) {
            devices[count] = current;
            count++;
        }
        current = current->next;
    }
    
    return count;
}

/* Initialize a device */
int hal_initialize_device(uint32_t device_id) {
    struct hal_device *device = hal_find_device_by_id(device_id);
    if (!device) {
        return -1;
    }
    
    device->status = DEVICE_STATUS_INITIALIZING;
    
    serial_puts("[NEURAL-HAL] Initializing device: ");
    serial_puts(device->name);
    serial_puts("\n");
    
    if (device->init) {
        int result = device->init(device);
        if (result == 0) {
            device->status = DEVICE_STATUS_ACTIVE;
            serial_puts("[NEURAL-HAL] Device initialization successful\n");
        } else {
            device->status = DEVICE_STATUS_ERROR;
            serial_puts("[NEURAL-HAL] Device initialization failed\n");
        }
        return result;
    }
    
    /* No init function, mark as active */
    device->status = DEVICE_STATUS_ACTIVE;
    return 0;
}

/* Start a device */
int hal_start_device(uint32_t device_id) {
    struct hal_device *device = hal_find_device_by_id(device_id);
    if (!device) {
        return -1;
    }
    
    if (device->status != DEVICE_STATUS_ACTIVE) {
        return -2;
    }
    
    if (device->start) {
        return device->start(device);
    }
    
    return 0;
}

/* Stop a device */
int hal_stop_device(uint32_t device_id) {
    struct hal_device *device = hal_find_device_by_id(device_id);
    if (!device) {
        return -1;
    }
    
    if (device->stop) {
        return device->stop(device);
    }
    
    return 0;
}

/* Reset a device */
int hal_reset_device(uint32_t device_id) {
    struct hal_device *device = hal_find_device_by_id(device_id);
    if (!device) {
        return -1;
    }
    
    serial_puts("[NEURAL-HAL] Resetting device: ");
    serial_puts(device->name);
    serial_puts("\n");
    
    if (device->reset) {
        int result = device->reset(device);
        if (result == 0) {
            device->status = DEVICE_STATUS_ACTIVE;
        } else {
            device->status = DEVICE_STATUS_ERROR;
        }
        return result;
    }
    
    return 0;
}

/* Print device information */
void hal_print_device_info(struct hal_device *device) {
    if (!device) return;
    
    serial_puts("[NEURAL-HAL] === Device Neural Profile ===\n");
    serial_puts("[INFO] Device ID: ");
    print_dec(device->device_id);
    serial_puts("\n");
    
    serial_puts("[INFO] Name: ");
    serial_puts(device->name);
    serial_puts("\n");
    
    serial_puts("[INFO] Manufacturer: ");
    serial_puts(device->manufacturer);
    serial_puts("\n");
    
    serial_puts("[INFO] Type: ");
    serial_puts(hal_get_device_type_name(device->type));
    serial_puts("\n");
    
    serial_puts("[INFO] Status: ");
    serial_puts(hal_get_device_status_name(device->status));
    serial_puts("\n");
    
    if (device->pci_dev) {
        serial_puts("[INFO] PCI Device: ");
        print_hex(device->pci_dev->bus);
        serial_puts(":");
        print_hex(device->pci_dev->device);
        serial_puts(":");
        print_hex(device->pci_dev->function);
        serial_puts("\n");
    }
    
    serial_puts("[NEURAL-HAL] === End Neural Profile ===\n");
}

/* Print all registered devices */
void hal_print_all_devices(void) {
    serial_puts("[NEURAL-HAL] === Neural Device Registry ===\n");
    serial_puts("[INFO] Total registered devices: ");
    print_dec(device_count);
    serial_puts("\n\n");
    
    struct hal_device *current = device_registry;
    while (current) {
        hal_print_device_info(current);
        current = current->next;
    }
    
    serial_puts("[NEURAL-HAL] === End Device Registry ===\n");
}

/* Initialize HAL and discover PCI devices */
void hal_init(void) {
    serial_puts("[NEURAL-HAL] Initializing Hardware Abstraction Layer...\n");
    
    device_registry = NULL;
    next_device_id = 1;
    device_count = 0;
    
    /* Initialize PCI subsystem */
    pci_init();
    
    /* Auto-discover and register PCI devices */
    hal_discover_pci_devices();
    
    serial_puts("[NEURAL-HAL] Hardware Abstraction Layer online\n");
}

/* Discover and register PCI devices automatically */
void hal_discover_pci_devices(void) {
    serial_puts("[NEURAL-HAL] Auto-discovering PCI neural devices...\n");
    
    int pci_count = pci_get_device_count();
    
    for (int i = 0; i < pci_count; i++) {
        struct pci_device *pci_dev = pci_get_device_by_index(i);
        if (!pci_dev) continue;
        
        device_type_t type = DEVICE_TYPE_UNKNOWN;
        const char *device_name = "Unknown Neural Device";
        
        /* Determine device type based on PCI class */
        switch (pci_dev->class_code) {
            case PCI_CLASS_NETWORK:
                type = DEVICE_TYPE_NETWORK;
                device_name = "Neural Network Interface";
                break;
            case PCI_CLASS_DISPLAY:
                type = DEVICE_TYPE_DISPLAY;
                device_name = "Neural Display Controller";
                break;
            case PCI_CLASS_STORAGE:
                type = DEVICE_TYPE_STORAGE;
                device_name = "Neural Storage Controller";
                break;
            case PCI_CLASS_SERIAL:
                if (pci_dev->subclass == PCI_SUBCLASS_USB) {
                    type = DEVICE_TYPE_USB;
                    device_name = "Neural USB Controller";
                }
                break;
        }
        
        if (type != DEVICE_TYPE_UNKNOWN) {
            struct hal_device *hal_dev = hal_create_device(type, device_name, pci_dev->vendor_name);
            if (hal_dev) {
                hal_dev->pci_dev = pci_dev;
                hal_register_device(hal_dev);
            }
        }
    }
    
    serial_puts("[NEURAL-HAL] PCI device discovery complete\n");
}

/* Get device count */
int hal_get_device_count(void) {
    return device_count;
}

/* Initialize all registered devices */
void hal_initialize_all_devices(void) {
    serial_puts("[NEURAL-HAL] Initializing all neural devices...\n");
    
    struct hal_device *current = device_registry;
    while (current) {
        hal_initialize_device(current->device_id);
        current = current->next;
    }
    
    serial_puts("[NEURAL-HAL] All device initialization complete\n");
}