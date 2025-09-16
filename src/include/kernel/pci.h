/* pci.h - Brandon Media OS PCI Interface
 * Neural Hardware Matrix Definitions
 */

#ifndef KERNEL_PCI_H
#define KERNEL_PCI_H

#include <stdint.h>

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

/* PCI Function Prototypes */
void pci_init(void);
void pci_enumerate_devices(void);
struct pci_device *pci_find_device_by_class(uint8_t class_code, uint8_t subclass);
struct pci_device *pci_find_device_by_id(uint16_t vendor_id, uint16_t device_id);
int pci_get_devices_by_class(uint8_t class_code, struct pci_device **devices, int max_devices);
void pci_print_device_info(struct pci_device *device);
void pci_print_all_devices(void);
int pci_get_device_count(void);
struct pci_device *pci_get_device_by_index(int index);

/* PCI Device Classes */
#define PCI_CLASS_NETWORK  0x02
#define PCI_CLASS_DISPLAY  0x03
#define PCI_CLASS_STORAGE  0x01
#define PCI_CLASS_BRIDGE   0x06
#define PCI_CLASS_SERIAL   0x0C

/* PCI Subclasses */
#define PCI_SUBCLASS_ETHERNET   0x00
#define PCI_SUBCLASS_VGA        0x00
#define PCI_SUBCLASS_USB        0x03

#endif /* KERNEL_PCI_H */