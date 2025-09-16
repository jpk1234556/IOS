/* pci.c - Brandon Media OS PCI Bus Enumeration and Device Discovery
 * Neural Interface for Hardware Matrix
 */

#include <stdint.h>
#include <stddef.h>
#include "kernel/memory.h"

/* PCI Configuration Space Access */
#define PCI_CONFIG_ADDRESS 0xCF8
#define PCI_CONFIG_DATA    0xCFC

/* PCI Configuration Registers */
#define PCI_VENDOR_ID      0x00
#define PCI_DEVICE_ID      0x02
#define PCI_COMMAND        0x04
#define PCI_STATUS         0x06
#define PCI_REVISION_ID    0x08
#define PCI_PROG_IF        0x09
#define PCI_SUBCLASS       0x0A
#define PCI_CLASS          0x0B
#define PCI_CACHE_LINE     0x0C
#define PCI_LATENCY_TIMER  0x0D
#define PCI_HEADER_TYPE    0x0E
#define PCI_BIST           0x0F
#define PCI_BAR0           0x10
#define PCI_BAR1           0x14
#define PCI_BAR2           0x18
#define PCI_BAR3           0x1C
#define PCI_BAR4           0x20
#define PCI_BAR5           0x24

/* PCI Device Classes */
#define PCI_CLASS_NETWORK  0x02
#define PCI_CLASS_DISPLAY  0x03
#define PCI_CLASS_STORAGE  0x01
#define PCI_CLASS_BRIDGE   0x06
#define PCI_CLASS_SERIAL   0x0C

/* Maximum PCI devices we can track */
#define MAX_PCI_DEVICES 256

/* PCI Device Structure */
struct pci_device {
    uint8_t bus;
    uint8_t device;
    uint8_t function;
    uint16_t vendor_id;
    uint16_t device_id;
    uint8_t class_code;
    uint8_t subclass;
    uint8_t prog_if;
    uint8_t header_type;
    uint32_t bar[6];
    uint8_t irq_line;
    uint8_t irq_pin;
    const char *device_name;
    const char *vendor_name;
    struct pci_device *next;
};

/* Global PCI device list */
static struct pci_device *pci_device_list = NULL;
static int pci_device_count = 0;

/* External functions for output */
extern void serial_puts(const char *s);
extern void print_hex(uint64_t num);
extern void print_dec(uint64_t num);

/* Port I/O functions */
static inline void outl(uint16_t port, uint32_t val) {
    asm volatile ("outl %0, %1" : : "a"(val), "Nd"(port));
}

static inline uint32_t inl(uint16_t port) {
    uint32_t ret;
    asm volatile ("inl %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline void outw(uint16_t port, uint16_t val) {
    asm volatile ("outw %0, %1" : : "a"(val), "Nd"(port));
}

static inline uint16_t inw(uint16_t port) {
    uint16_t ret;
    asm volatile ("inw %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

/* PCI Configuration Space Access */
static uint32_t pci_config_read_dword(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset) {
    uint32_t address = (1 << 31) | ((uint32_t)bus << 16) | ((uint32_t)device << 11) | 
                       ((uint32_t)function << 8) | (offset & 0xFC);
    
    outl(PCI_CONFIG_ADDRESS, address);
    return inl(PCI_CONFIG_DATA);
}

static uint16_t pci_config_read_word(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset) {
    uint32_t dword = pci_config_read_dword(bus, device, function, offset);
    return (uint16_t)(dword >> ((offset & 2) * 8));
}

static uint8_t pci_config_read_byte(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset) {
    uint32_t dword = pci_config_read_dword(bus, device, function, offset);
    return (uint8_t)(dword >> ((offset & 3) * 8));
}

static void pci_config_write_dword(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset, uint32_t value) {
    uint32_t address = (1 << 31) | ((uint32_t)bus << 16) | ((uint32_t)device << 11) | 
                       ((uint32_t)function << 8) | (offset & 0xFC);
    
    outl(PCI_CONFIG_ADDRESS, address);
    outl(PCI_CONFIG_DATA, value);
}

/* Get vendor name string */
static const char *pci_get_vendor_name(uint16_t vendor_id) {
    switch (vendor_id) {
        case 0x8086: return "Intel Corporation";
        case 0x1022: return "AMD";
        case 0x10DE: return "NVIDIA Corporation";
        case 0x1002: return "ATI Technologies";
        case 0x15AD: return "VMware Inc.";
        case 0x1AF4: return "Red Hat Inc. (Virtio)";
        case 0x1234: return "QEMU Virtual Hardware";
        case 0x1013: return "Cirrus Logic";
        case 0x5853: return "XenSource Inc.";
        case 0x1B36: return "Red Hat QEMU Virtual Machine";
        default: return "Unknown Vendor";
    }
}

/* Get device class name */
static const char *pci_get_class_name(uint8_t class_code, uint8_t subclass) {
    switch (class_code) {
        case 0x00: return "Legacy Device";
        case 0x01:
            switch (subclass) {
                case 0x00: return "SCSI Bus Controller";
                case 0x01: return "IDE Controller";
                case 0x02: return "Floppy Controller";
                case 0x03: return "IPI Bus Controller";
                case 0x04: return "RAID Controller";
                case 0x06: return "SATA Controller";
                default: return "Mass Storage Controller";
            }
        case 0x02:
            switch (subclass) {
                case 0x00: return "Ethernet Controller";
                case 0x01: return "Token Ring Controller";
                case 0x02: return "FDDI Controller";
                case 0x03: return "ATM Controller";
                default: return "Network Controller";
            }
        case 0x03:
            switch (subclass) {
                case 0x00: return "VGA Compatible Controller";
                case 0x01: return "XGA Controller";
                case 0x02: return "3D Controller";
                default: return "Display Controller";
            }
        case 0x04: return "Multimedia Controller";
        case 0x05: return "Memory Controller";
        case 0x06:
            switch (subclass) {
                case 0x00: return "Host Bridge";
                case 0x01: return "ISA Bridge";
                case 0x02: return "EISA Bridge";
                case 0x04: return "PCI-to-PCI Bridge";
                default: return "Bridge Device";
            }
        case 0x0C:
            switch (subclass) {
                case 0x00: return "FireWire Controller";
                case 0x01: return "ACCESS Bus Controller";
                case 0x02: return "SSA Controller";
                case 0x03: return "USB Controller";
                case 0x04: return "Fibre Channel Controller";
                default: return "Serial Bus Controller";
            }
        default: return "Unknown Device";
    }
}

/* Check if device exists */
static int pci_device_exists(uint8_t bus, uint8_t device, uint8_t function) {
    uint16_t vendor_id = pci_config_read_word(bus, device, function, PCI_VENDOR_ID);
    return (vendor_id != 0xFFFF);
}

/* Read BAR (Base Address Register) */
static uint32_t pci_read_bar(uint8_t bus, uint8_t device, uint8_t function, uint8_t bar_num) {
    uint8_t bar_offset = PCI_BAR0 + (bar_num * 4);
    return pci_config_read_dword(bus, device, function, bar_offset);
}

/* Create a new PCI device structure */
static struct pci_device *pci_create_device(uint8_t bus, uint8_t device, uint8_t function) {
    struct pci_device *pci_dev = (struct pci_device *)kmalloc(sizeof(struct pci_device));
    if (!pci_dev) {
        return NULL;
    }

    pci_dev->bus = bus;
    pci_dev->device = device;
    pci_dev->function = function;
    pci_dev->vendor_id = pci_config_read_word(bus, device, function, PCI_VENDOR_ID);
    pci_dev->device_id = pci_config_read_word(bus, device, function, PCI_DEVICE_ID);
    pci_dev->class_code = pci_config_read_byte(bus, device, function, PCI_CLASS);
    pci_dev->subclass = pci_config_read_byte(bus, device, function, PCI_SUBCLASS);
    pci_dev->prog_if = pci_config_read_byte(bus, device, function, PCI_PROG_IF);
    pci_dev->header_type = pci_config_read_byte(bus, device, function, PCI_HEADER_TYPE);

    /* Read BARs */
    for (int i = 0; i < 6; i++) {
        pci_dev->bar[i] = pci_read_bar(bus, device, function, i);
    }

    pci_dev->vendor_name = pci_get_vendor_name(pci_dev->vendor_id);
    pci_dev->device_name = pci_get_class_name(pci_dev->class_code, pci_dev->subclass);
    pci_dev->next = NULL;

    return pci_dev;
}

/* Add device to the global list */
static void pci_add_device(struct pci_device *device) {
    if (!pci_device_list) {
        pci_device_list = device;
    } else {
        struct pci_device *current = pci_device_list;
        while (current->next) {
            current = current->next;
        }
        current->next = device;
    }
    pci_device_count++;
}

/* Enumerate all PCI devices */
void pci_enumerate_devices(void) {
    serial_puts("[NEURAL-PCI] Initiating hardware matrix scan...\n");
    
    for (uint16_t bus = 0; bus < 256; bus++) {
        for (uint8_t device = 0; device < 32; device++) {
            for (uint8_t function = 0; function < 8; function++) {
                if (pci_device_exists(bus, device, function)) {
                    struct pci_device *pci_dev = pci_create_device(bus, device, function);
                    if (pci_dev) {
                        pci_add_device(pci_dev);
                        
                        serial_puts("[NEURAL-PCI] Device detected: ");
                        print_hex(bus);
                        serial_puts(":");
                        print_hex(device);
                        serial_puts(":");
                        print_hex(function);
                        serial_puts(" - ");
                        serial_puts(pci_dev->vendor_name);
                        serial_puts(" (");
                        serial_puts(pci_dev->device_name);
                        serial_puts(")\n");
                    }
                    
                    /* If not a multi-function device, don't check other functions */
                    if (function == 0) {
                        uint8_t header_type = pci_config_read_byte(bus, device, function, PCI_HEADER_TYPE);
                        if ((header_type & 0x80) == 0) {
                            break;
                        }
                    }
                }
            }
        }
    }
    
    serial_puts("[NEURAL-PCI] Hardware matrix scan complete - ");
    print_dec(pci_device_count);
    serial_puts(" neural devices detected\n");
}

/* Find device by class and subclass */
struct pci_device *pci_find_device_by_class(uint8_t class_code, uint8_t subclass) {
    struct pci_device *current = pci_device_list;
    while (current) {
        if (current->class_code == class_code && current->subclass == subclass) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

/* Find device by vendor and device ID */
struct pci_device *pci_find_device_by_id(uint16_t vendor_id, uint16_t device_id) {
    struct pci_device *current = pci_device_list;
    while (current) {
        if (current->vendor_id == vendor_id && current->device_id == device_id) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

/* Get all devices of a specific class */
int pci_get_devices_by_class(uint8_t class_code, struct pci_device **devices, int max_devices) {
    struct pci_device *current = pci_device_list;
    int count = 0;
    
    while (current && count < max_devices) {
        if (current->class_code == class_code) {
            devices[count] = current;
            count++;
        }
        current = current->next;
    }
    
    return count;
}

/* Print detailed device information */
void pci_print_device_info(struct pci_device *device) {
    if (!device) return;
    
    serial_puts("[NEURAL-PCI] === Device Neural Profile ===\n");
    serial_puts("[INFO] Bus:Device:Function = ");
    print_hex(device->bus);
    serial_puts(":");
    print_hex(device->device);
    serial_puts(":");
    print_hex(device->function);
    serial_puts("\n");
    
    serial_puts("[INFO] Vendor: ");
    serial_puts(device->vendor_name);
    serial_puts(" (ID: ");
    print_hex(device->vendor_id);
    serial_puts(")\n");
    
    serial_puts("[INFO] Device: ");
    serial_puts(device->device_name);
    serial_puts(" (ID: ");
    print_hex(device->device_id);
    serial_puts(")\n");
    
    serial_puts("[INFO] Class: ");
    print_hex(device->class_code);
    serial_puts(" Subclass: ");
    print_hex(device->subclass);
    serial_puts(" ProgIF: ");
    print_hex(device->prog_if);
    serial_puts("\n");
    
    serial_puts("[INFO] Base Address Registers:\n");
    for (int i = 0; i < 6; i++) {
        if (device->bar[i] != 0) {
            serial_puts("[BAR");
            print_dec(i);
            serial_puts("] ");
            print_hex(device->bar[i]);
            serial_puts("\n");
        }
    }
    
    serial_puts("[NEURAL-PCI] === End Neural Profile ===\n");
}

/* Print all detected devices */
void pci_print_all_devices(void) {
    serial_puts("[NEURAL-PCI] === Neural Hardware Matrix ===\n");
    serial_puts("[INFO] Total devices detected: ");
    print_dec(pci_device_count);
    serial_puts("\n\n");
    
    struct pci_device *current = pci_device_list;
    while (current) {
        pci_print_device_info(current);
        current = current->next;
    }
    
    serial_puts("[NEURAL-PCI] === End Hardware Matrix ===\n");
}

/* Initialize PCI subsystem */
void pci_init(void) {
    serial_puts("[NEURAL-PCI] Initializing PCI neural interface...\n");
    
    pci_device_list = NULL;
    pci_device_count = 0;
    
    /* Enumerate all PCI devices */
    pci_enumerate_devices();
    
    serial_puts("[NEURAL-PCI] PCI neural interface online\n");
}

/* Get device count */
int pci_get_device_count(void) {
    return pci_device_count;
}

/* Get device by index */
struct pci_device *pci_get_device_by_index(int index) {
    if (index < 0 || index >= pci_device_count) {
        return NULL;
    }
    
    struct pci_device *current = pci_device_list;
    for (int i = 0; i < index && current; i++) {
        current = current->next;
    }
    
    return current;
}