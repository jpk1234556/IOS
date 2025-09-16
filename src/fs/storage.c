/* storage.c - Brandon Media OS Storage Device Abstraction */
#include <stdint.h>
#include "kernel/fs.h"
#include "kernel/memory.h"

/* External functions */
extern void serial_puts(const char *s);
extern void print_hex(uint64_t num);
extern void print_dec(uint64_t num);
extern void *kmalloc(size_t size);
extern void kfree(void *ptr);
extern void memory_set(void *dst, int value, size_t size);
extern void memory_copy(void *dst, const void *src, size_t size);

/* Storage device types */
#define STORAGE_TYPE_RAM        1   /* Neural RAM storage */
#define STORAGE_TYPE_DISK       2   /* Neural disk storage */
#define STORAGE_TYPE_NETWORK    3   /* Neural network storage */

/* Storage device structure */
struct storage_device {
    char name[32];                  /* Neural storage device name */
    uint32_t type;                  /* Neural storage type */
    uint64_t capacity;              /* Neural storage capacity */
    uint64_t sector_size;           /* Neural sector size */
    uint32_t flags;                 /* Neural device flags */
    
    /* Device operations */
    int (*read)(struct storage_device *dev, uint64_t lba, uint32_t count, void *buffer);
    int (*write)(struct storage_device *dev, uint64_t lba, uint32_t count, const void *buffer);
    int (*flush)(struct storage_device *dev);
    int (*format)(struct storage_device *dev);
    
    /* Private device data */
    void *private_data;
    
    /* List linkage */
    struct storage_device *next;
};

/* Global storage device list */
static struct storage_device *storage_devices = NULL;

/* RAM storage device data */
struct ram_storage_data {
    uint8_t *data;              /* Neural RAM buffer */
    uint64_t size;              /* Neural buffer size */
};

/* Forward declarations */
static int ram_storage_read(struct storage_device *dev, uint64_t lba, uint32_t count, void *buffer);
static int ram_storage_write(struct storage_device *dev, uint64_t lba, uint32_t count, const void *buffer);
static int ram_storage_flush(struct storage_device *dev);
static int ram_storage_format(struct storage_device *dev);

/* Initialize storage subsystem */
void storage_init(void) {
    serial_puts("[NEXUS] Initializing neural storage matrix...\n");
    
    storage_devices = NULL;
    
    serial_puts("[STORAGE] Neural storage matrix online\n");
}

/* Register a storage device */
int storage_register_device(struct storage_device *device) {
    if (!device) return -1;
    
    serial_puts("[STORAGE] Registering neural storage device: ");
    serial_puts(device->name);
    serial_puts("\n");
    
    /* Add to device list */
    device->next = storage_devices;
    storage_devices = device;
    
    serial_puts("[SUCCESS] Neural storage device registered\n");
    return 0;
}

/* Create RAM storage device */
struct storage_device *storage_create_ram_device(const char *name, uint64_t size) {
    if (!name || size == 0) return NULL;
    
    serial_puts("[STORAGE] Creating neural RAM storage: ");
    serial_puts(name);
    serial_puts(" (");
    print_dec(size);
    serial_puts(" bytes)\n");
    
    /* Allocate device structure */
    struct storage_device *device = (struct storage_device *)kmalloc(sizeof(struct storage_device));
    if (!device) {
        serial_puts("[ERROR] Failed to allocate neural storage device\n");
        return NULL;
    }
    
    /* Allocate RAM storage data */
    struct ram_storage_data *ram_data = (struct ram_storage_data *)kmalloc(sizeof(struct ram_storage_data));
    if (!ram_data) {
        serial_puts("[ERROR] Failed to allocate neural RAM storage data\n");
        kfree(device);
        return NULL;
    }
    
    /* Allocate storage buffer */
    ram_data->data = (uint8_t *)kmalloc(size);
    if (!ram_data->data) {
        serial_puts("[ERROR] Failed to allocate neural RAM buffer\n");
        kfree(ram_data);
        kfree(device);
        return NULL;
    }
    
    /* Initialize storage buffer */
    memory_set(ram_data->data, 0, size);
    ram_data->size = size;
    
    /* Initialize device structure */
    memory_set(device, 0, sizeof(struct storage_device));
    str_cpy(device->name, name);
    device->type = STORAGE_TYPE_RAM;
    device->capacity = size;
    device->sector_size = 512;  /* Standard 512-byte sectors */
    device->flags = 0;
    device->private_data = ram_data;
    
    /* Set device operations */
    device->read = ram_storage_read;
    device->write = ram_storage_write;
    device->flush = ram_storage_flush;
    device->format = ram_storage_format;
    
    serial_puts("[SUCCESS] Neural RAM storage device created\n");
    return device;
}

/* RAM storage read operation */
static int ram_storage_read(struct storage_device *dev, uint64_t lba, uint32_t count, void *buffer) {
    if (!dev || !buffer || count == 0) return -1;
    
    struct ram_storage_data *ram_data = (struct ram_storage_data *)dev->private_data;
    if (!ram_data || !ram_data->data) return -1;
    
    /* Calculate byte offset and size */
    uint64_t offset = lba * dev->sector_size;
    uint64_t read_size = count * dev->sector_size;
    
    /* Check bounds */
    if (offset + read_size > ram_data->size) {
        serial_puts("[ERROR] Neural storage read out of bounds\n");
        return -1;
    }
    
    /* Copy data */
    memory_copy(buffer, ram_data->data + offset, read_size);
    
    serial_puts("[STORAGE] Neural data read: ");
    print_dec(count);
    serial_puts(" sectors from LBA ");
    print_dec(lba);
    serial_puts("\n");
    
    return 0;
}

/* RAM storage write operation */
static int ram_storage_write(struct storage_device *dev, uint64_t lba, uint32_t count, const void *buffer) {
    if (!dev || !buffer || count == 0) return -1;
    
    struct ram_storage_data *ram_data = (struct ram_storage_data *)dev->private_data;
    if (!ram_data || !ram_data->data) return -1;
    
    /* Calculate byte offset and size */
    uint64_t offset = lba * dev->sector_size;
    uint64_t write_size = count * dev->sector_size;
    
    /* Check bounds */
    if (offset + write_size > ram_data->size) {
        serial_puts("[ERROR] Neural storage write out of bounds\n");
        return -1;
    }
    
    /* Copy data */
    memory_copy(ram_data->data + offset, buffer, write_size);
    
    serial_puts("[STORAGE] Neural data written: ");
    print_dec(count);
    serial_puts(" sectors to LBA ");
    print_dec(lba);
    serial_puts("\n");
    
    return 0;
}

/* RAM storage flush operation */
static int ram_storage_flush(struct storage_device *dev) {
    if (!dev) return -1;
    
    /* RAM storage doesn't need flushing */
    serial_puts("[STORAGE] Neural RAM storage flushed\n");
    return 0;
}

/* RAM storage format operation */
static int ram_storage_format(struct storage_device *dev) {
    if (!dev) return -1;
    
    struct ram_storage_data *ram_data = (struct ram_storage_data *)dev->private_data;
    if (!ram_data || !ram_data->data) return -1;
    
    /* Clear all data */
    memory_set(ram_data->data, 0, ram_data->size);
    
    serial_puts("[STORAGE] Neural RAM storage formatted\n");
    return 0;
}

/* Find storage device by name */
struct storage_device *storage_find_device(const char *name) {
    if (!name) return NULL;
    
    struct storage_device *current = storage_devices;
    while (current) {
        if (str_cmp(current->name, name) == 0) {
            return current;
        }
        current = current->next;
    }
    
    return NULL;
}

/* Get storage device information */
void storage_print_devices(void) {
    serial_puts("[STORAGE] Neural storage devices:\n");
    
    struct storage_device *current = storage_devices;
    int count = 0;
    
    while (current) {
        serial_puts("[");
        print_dec(count++);
        serial_puts("] ");
        serial_puts(current->name);
        serial_puts(" - Type: ");
        
        switch (current->type) {
            case STORAGE_TYPE_RAM:
                serial_puts("Neural RAM");
                break;
            case STORAGE_TYPE_DISK:
                serial_puts("Neural Disk");
                break;
            case STORAGE_TYPE_NETWORK:
                serial_puts("Neural Network");
                break;
            default:
                serial_puts("Unknown");
                break;
        }
        
        serial_puts(", Capacity: ");
        print_dec(current->capacity);
        serial_puts(" bytes\n");
        
        current = current->next;
    }
    
    if (count == 0) {
        serial_puts("  No neural storage devices registered\n");
    }
}

/* String utility function */
static char *str_cpy(char *dest, const char *src) {
    char *original_dest = dest;
    while ((*dest++ = *src++));
    return original_dest;
}

static int str_cmp(const char *s1, const char *s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(unsigned char*)s1 - *(unsigned char*)s2;
}/* storage.c - Brandon Media OS Storage Device Abstraction */
#include <stdint.h>
#include "kernel/fs.h"
#include "kernel/memory.h"

/* External functions */
extern void serial_puts(const char *s);
extern void print_hex(uint64_t num);
extern void print_dec(uint64_t num);
extern void *kmalloc(size_t size);
extern void kfree(void *ptr);
extern void memory_set(void *dst, int value, size_t size);
extern void memory_copy(void *dst, const void *src, size_t size);

/* Storage device types */
#define STORAGE_TYPE_RAM        1   /* Neural RAM storage */
#define STORAGE_TYPE_DISK       2   /* Neural disk storage */
#define STORAGE_TYPE_NETWORK    3   /* Neural network storage */

/* Storage device structure */
struct storage_device {
    char name[32];                  /* Neural storage device name */
    uint32_t type;                  /* Neural storage type */
    uint64_t capacity;              /* Neural storage capacity */
    uint64_t sector_size;           /* Neural sector size */
    uint32_t flags;                 /* Neural device flags */
    
    /* Device operations */
    int (*read)(struct storage_device *dev, uint64_t lba, uint32_t count, void *buffer);
    int (*write)(struct storage_device *dev, uint64_t lba, uint32_t count, const void *buffer);
    int (*flush)(struct storage_device *dev);
    int (*format)(struct storage_device *dev);
    
    /* Private device data */
    void *private_data;
    
    /* List linkage */
    struct storage_device *next;
};

/* Global storage device list */
static struct storage_device *storage_devices = NULL;

/* RAM storage device data */
struct ram_storage_data {
    uint8_t *data;              /* Neural RAM buffer */
    uint64_t size;              /* Neural buffer size */
};

/* Forward declarations */
static int ram_storage_read(struct storage_device *dev, uint64_t lba, uint32_t count, void *buffer);
static int ram_storage_write(struct storage_device *dev, uint64_t lba, uint32_t count, const void *buffer);
static int ram_storage_flush(struct storage_device *dev);
static int ram_storage_format(struct storage_device *dev);

/* Initialize storage subsystem */
void storage_init(void) {
    serial_puts("[NEXUS] Initializing neural storage matrix...\n");
    
    storage_devices = NULL;
    
    serial_puts("[STORAGE] Neural storage matrix online\n");
}

/* Register a storage device */
int storage_register_device(struct storage_device *device) {
    if (!device) return -1;
    
    serial_puts("[STORAGE] Registering neural storage device: ");
    serial_puts(device->name);
    serial_puts("\n");
    
    /* Add to device list */
    device->next = storage_devices;
    storage_devices = device;
    
    serial_puts("[SUCCESS] Neural storage device registered\n");
    return 0;
}

/* Create RAM storage device */
struct storage_device *storage_create_ram_device(const char *name, uint64_t size) {
    if (!name || size == 0) return NULL;
    
    serial_puts("[STORAGE] Creating neural RAM storage: ");
    serial_puts(name);
    serial_puts(" (");
    print_dec(size);
    serial_puts(" bytes)\n");
    
    /* Allocate device structure */
    struct storage_device *device = (struct storage_device *)kmalloc(sizeof(struct storage_device));
    if (!device) {
        serial_puts("[ERROR] Failed to allocate neural storage device\n");
        return NULL;
    }
    
    /* Allocate RAM storage data */
    struct ram_storage_data *ram_data = (struct ram_storage_data *)kmalloc(sizeof(struct ram_storage_data));
    if (!ram_data) {
        serial_puts("[ERROR] Failed to allocate neural RAM storage data\n");
        kfree(device);
        return NULL;
    }
    
    /* Allocate storage buffer */
    ram_data->data = (uint8_t *)kmalloc(size);
    if (!ram_data->data) {
        serial_puts("[ERROR] Failed to allocate neural RAM buffer\n");
        kfree(ram_data);
        kfree(device);
        return NULL;
    }
    
    /* Initialize storage buffer */
    memory_set(ram_data->data, 0, size);
    ram_data->size = size;
    
    /* Initialize device structure */
    memory_set(device, 0, sizeof(struct storage_device));
    str_cpy(device->name, name);
    device->type = STORAGE_TYPE_RAM;
    device->capacity = size;
    device->sector_size = 512;  /* Standard 512-byte sectors */
    device->flags = 0;
    device->private_data = ram_data;
    
    /* Set device operations */
    device->read = ram_storage_read;
    device->write = ram_storage_write;
    device->flush = ram_storage_flush;
    device->format = ram_storage_format;
    
    serial_puts("[SUCCESS] Neural RAM storage device created\n");
    return device;
}

/* RAM storage read operation */
static int ram_storage_read(struct storage_device *dev, uint64_t lba, uint32_t count, void *buffer) {
    if (!dev || !buffer || count == 0) return -1;
    
    struct ram_storage_data *ram_data = (struct ram_storage_data *)dev->private_data;
    if (!ram_data || !ram_data->data) return -1;
    
    /* Calculate byte offset and size */
    uint64_t offset = lba * dev->sector_size;
    uint64_t read_size = count * dev->sector_size;
    
    /* Check bounds */
    if (offset + read_size > ram_data->size) {
        serial_puts("[ERROR] Neural storage read out of bounds\n");
        return -1;
    }
    
    /* Copy data */
    memory_copy(buffer, ram_data->data + offset, read_size);
    
    serial_puts("[STORAGE] Neural data read: ");
    print_dec(count);
    serial_puts(" sectors from LBA ");
    print_dec(lba);
    serial_puts("\n");
    
    return 0;
}

/* RAM storage write operation */
static int ram_storage_write(struct storage_device *dev, uint64_t lba, uint32_t count, const void *buffer) {
    if (!dev || !buffer || count == 0) return -1;
    
    struct ram_storage_data *ram_data = (struct ram_storage_data *)dev->private_data;
    if (!ram_data || !ram_data->data) return -1;
    
    /* Calculate byte offset and size */
    uint64_t offset = lba * dev->sector_size;
    uint64_t write_size = count * dev->sector_size;
    
    /* Check bounds */
    if (offset + write_size > ram_data->size) {
        serial_puts("[ERROR] Neural storage write out of bounds\n");
        return -1;
    }
    
    /* Copy data */
    memory_copy(ram_data->data + offset, buffer, write_size);
    
    serial_puts("[STORAGE] Neural data written: ");
    print_dec(count);
    serial_puts(" sectors to LBA ");
    print_dec(lba);
    serial_puts("\n");
    
    return 0;
}

/* RAM storage flush operation */
static int ram_storage_flush(struct storage_device *dev) {
    if (!dev) return -1;
    
    /* RAM storage doesn't need flushing */
    serial_puts("[STORAGE] Neural RAM storage flushed\n");
    return 0;
}

/* RAM storage format operation */
static int ram_storage_format(struct storage_device *dev) {
    if (!dev) return -1;
    
    struct ram_storage_data *ram_data = (struct ram_storage_data *)dev->private_data;
    if (!ram_data || !ram_data->data) return -1;
    
    /* Clear all data */
    memory_set(ram_data->data, 0, ram_data->size);
    
    serial_puts("[STORAGE] Neural RAM storage formatted\n");
    return 0;
}

/* Find storage device by name */
struct storage_device *storage_find_device(const char *name) {
    if (!name) return NULL;
    
    struct storage_device *current = storage_devices;
    while (current) {
        if (str_cmp(current->name, name) == 0) {
            return current;
        }
        current = current->next;
    }
    
    return NULL;
}

/* Get storage device information */
void storage_print_devices(void) {
    serial_puts("[STORAGE] Neural storage devices:\n");
    
    struct storage_device *current = storage_devices;
    int count = 0;
    
    while (current) {
        serial_puts("[");
        print_dec(count++);
        serial_puts("] ");
        serial_puts(current->name);
        serial_puts(" - Type: ");
        
        switch (current->type) {
            case STORAGE_TYPE_RAM:
                serial_puts("Neural RAM");
                break;
            case STORAGE_TYPE_DISK:
                serial_puts("Neural Disk");
                break;
            case STORAGE_TYPE_NETWORK:
                serial_puts("Neural Network");
                break;
            default:
                serial_puts("Unknown");
                break;
        }
        
        serial_puts(", Capacity: ");
        print_dec(current->capacity);
        serial_puts(" bytes\n");
        
        current = current->next;
    }
    
    if (count == 0) {
        serial_puts("  No neural storage devices registered\n");
    }
}

/* String utility function */
static char *str_cpy(char *dest, const char *src) {
    char *original_dest = dest;
    while ((*dest++ = *src++));
    return original_dest;
}

static int str_cmp(const char *s1, const char *s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(unsigned char*)s1 - *(unsigned char*)s2;
}