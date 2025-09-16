/* ramfs.c - Brandon Media OS RAM File System */
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

/* RamFS file data structure */
struct ramfs_file_data {
    uint8_t *data;          /* Neural data buffer */
    uint64_t size;          /* Current neural data size */
    uint64_t capacity;      /* Neural buffer capacity */
};

/* RamFS filesystem data */
struct ramfs_data {
    uint64_t total_size;    /* Total neural memory used */
    uint64_t file_count;    /* Number of neural files */
    uint64_t dir_count;     /* Number of neural directories */
};

/* Forward declarations */
static int64_t ramfs_open(struct vfs_node *node, uint32_t flags);
static int64_t ramfs_close(struct vfs_node *node);
static int64_t ramfs_read(struct vfs_node *node, void *buffer, uint64_t size, uint64_t offset);
static int64_t ramfs_write(struct vfs_node *node, const void *buffer, uint64_t size, uint64_t offset);
static int64_t ramfs_truncate(struct vfs_node *node, uint64_t size);
static struct vfs_node *ramfs_lookup(struct vfs_node *dir, const char *name);
static int64_t ramfs_create(struct vfs_node *dir, const char *name, uint32_t type, uint32_t permissions);
static int64_t ramfs_remove(struct vfs_node *dir, const char *name);

/* RamFS file operations */
static struct file_operations ramfs_file_ops = {
    .open = ramfs_open,
    .close = ramfs_close,
    .read = ramfs_read,
    .write = ramfs_write,
    .truncate = ramfs_truncate,
};

/* RamFS directory operations */
static struct directory_operations ramfs_dir_ops = {
    .lookup = ramfs_lookup,
    .create = ramfs_create,
    .remove = ramfs_remove,
};

/* RamFS filesystem structure */
static struct filesystem ramfs_filesystem = {
    .name = "ramfs",
    .magic = 0x52414D46,  /* "RAMF" */
    .file_ops = &ramfs_file_ops,
    .dir_ops = &ramfs_dir_ops,
};

/* Initialize RamFS */
int ramfs_init(void) {
    serial_puts("[NEXUS] Initializing neural memory filesystem...\n");
    
    /* Allocate filesystem private data */
    struct ramfs_data *ramfs_data = (struct ramfs_data *)kmalloc(sizeof(struct ramfs_data));
    if (!ramfs_data) {
        serial_puts("[ERROR] Failed to allocate neural memory filesystem data\n");
        return -1;
    }
    
    /* Initialize filesystem data */
    memory_set(ramfs_data, 0, sizeof(struct ramfs_data));
    ramfs_filesystem.private_data = ramfs_data;
    
    /* Create root directory for RamFS */
    ramfs_filesystem.root = vfs_node_create("ramfs_root", FS_TYPE_DIRECTORY);
    if (!ramfs_filesystem.root) {
        serial_puts("[ERROR] Failed to create neural filesystem root\n");
        kfree(ramfs_data);
        return -1;
    }
    
    ramfs_filesystem.root->ops = &ramfs_file_ops;
    ramfs_filesystem.root->filesystem = &ramfs_filesystem;
    ramfs_data->dir_count = 1;
    
    /* Register the filesystem */
    extern int vfs_register_filesystem(struct filesystem *fs);
    if (vfs_register_filesystem(&ramfs_filesystem) != 0) {
        serial_puts("[ERROR] Failed to register neural memory filesystem\n");
        vfs_node_destroy(ramfs_filesystem.root);
        kfree(ramfs_data);
        return -1;
    }
    
    serial_puts("[RAMFS] Neural memory filesystem registered\n");
    serial_puts("[RAMFS] Root directory: ");
    print_hex((uint64_t)ramfs_filesystem.root);
    serial_puts("\n");
    
    return 0;
}

/* Open file */
static int64_t ramfs_open(struct vfs_node *node, uint32_t flags) {
    if (!node) return -1;
    
    serial_puts("[RAMFS] Opening neural file: ");
    serial_puts(node->name);
    serial_puts("\n");
    
    /* For directories, no special handling needed */
    if (node->type == FS_TYPE_DIRECTORY) {
        return 0;
    }
    
    /* For regular files, ensure file data exists */
    if (node->type == FS_TYPE_REGULAR && !node->fs_data) {
        struct ramfs_file_data *file_data = (struct ramfs_file_data *)kmalloc(sizeof(struct ramfs_file_data));
        if (!file_data) {
            serial_puts("[ERROR] Failed to allocate neural file data\n");
            return -1;
        }
        
        memory_set(file_data, 0, sizeof(struct ramfs_file_data));
        node->fs_data = file_data;
    }
    
    /* Update access time */
    extern uint64_t vfs_get_current_time(void);
    node->accessed_time = vfs_get_current_time();
    
    return 0;
}

/* Close file */
static int64_t ramfs_close(struct vfs_node *node) {
    if (!node) return -1;
    
    serial_puts("[RAMFS] Closing neural file: ");
    serial_puts(node->name);
    serial_puts("\n");
    
    /* No special handling needed for close */
    return 0;
}

/* Read from file */
static int64_t ramfs_read(struct vfs_node *node, void *buffer, uint64_t size, uint64_t offset) {
    if (!node || !buffer) return -1;
    
    if (node->type != FS_TYPE_REGULAR) {
        return -1;  /* Can only read regular files */
    }
    
    struct ramfs_file_data *file_data = (struct ramfs_file_data *)node->fs_data;
    if (!file_data || !file_data->data) {
        return 0;  /* Empty file */
    }
    
    /* Check bounds */
    if (offset >= file_data->size) {
        return 0;  /* EOF */
    }
    
    /* Calculate actual read size */
    uint64_t available = file_data->size - offset;
    uint64_t read_size = (size < available) ? size : available;
    
    /* Copy data */
    memory_copy(buffer, file_data->data + offset, read_size);
    
    /* Update access time */
    extern uint64_t vfs_get_current_time(void);
    node->accessed_time = vfs_get_current_time();
    
    serial_puts("[RAMFS] Neural data read: ");
    print_dec(read_size);
    serial_puts(" bytes from offset ");
    print_dec(offset);
    serial_puts("\n");
    
    return read_size;
}

/* Write to file */
static int64_t ramfs_write(struct vfs_node *node, const void *buffer, uint64_t size, uint64_t offset) {
    if (!node || !buffer || size == 0) return -1;
    
    if (node->type != FS_TYPE_REGULAR) {
        return -1;  /* Can only write to regular files */
    }
    
    struct ramfs_file_data *file_data = (struct ramfs_file_data *)node->fs_data;
    if (!file_data) {
        /* Create file data if it doesn't exist */
        file_data = (struct ramfs_file_data *)kmalloc(sizeof(struct ramfs_file_data));
        if (!file_data) {
            serial_puts("[ERROR] Failed to allocate neural file data\n");
            return -1;
        }
        memory_set(file_data, 0, sizeof(struct ramfs_file_data));
        node->fs_data = file_data;
    }
    
    /* Calculate required size */
    uint64_t required_size = offset + size;
    
    /* Expand buffer if necessary */
    if (required_size > file_data->capacity) {
        uint64_t new_capacity = required_size * 2;  /* Double capacity */
        uint8_t *new_data = (uint8_t *)kmalloc(new_capacity);
        if (!new_data) {
            serial_puts("[ERROR] Failed to expand neural data buffer\n");
            return -1;
        }
        
        /* Copy existing data */
        if (file_data->data && file_data->size > 0) {
            memory_copy(new_data, file_data->data, file_data->size);
            kfree(file_data->data);
        }
        
        file_data->data = new_data;
        file_data->capacity = new_capacity;
    }
    
    /* Write data */
    memory_copy(file_data->data + offset, buffer, size);
    
    /* Update file size if necessary */
    if (required_size > file_data->size) {
        file_data->size = required_size;
        node->size = file_data->size;
    }
    
    /* Update modification time */
    extern uint64_t vfs_get_current_time(void);
    node->modified_time = vfs_get_current_time();
    
    serial_puts("[RAMFS] Neural data written: ");
    print_dec(size);
    serial_puts(" bytes at offset ");
    print_dec(offset);
    serial_puts("\n");
    
    return size;
}

/* Truncate file */
static int64_t ramfs_truncate(struct vfs_node *node, uint64_t size) {
    if (!node) return -1;
    
    if (node->type != FS_TYPE_REGULAR) {
        return -1;
    }
    
    struct ramfs_file_data *file_data = (struct ramfs_file_data *)node->fs_data;
    if (!file_data) {
        /* Create empty file data */
        file_data = (struct ramfs_file_data *)kmalloc(sizeof(struct ramfs_file_data));
        if (!file_data) return -1;
        memory_set(file_data, 0, sizeof(struct ramfs_file_data));
        node->fs_data = file_data;
    }
    
    /* Update size */
    file_data->size = size;
    node->size = size;
    
    /* Update modification time */
    extern uint64_t vfs_get_current_time(void);
    node->modified_time = vfs_get_current_time();
    
    serial_puts("[RAMFS] Neural file truncated to ");
    print_dec(size);
    serial_puts(" bytes\n");
    
    return 0;
}

/* Lookup file in directory */
static struct vfs_node *ramfs_lookup(struct vfs_node *dir, const char *name) {
    if (!dir || !name) return NULL;
    
    extern struct vfs_node *vfs_node_lookup(struct vfs_node *parent, const char *name);
    return vfs_node_lookup(dir, name);
}

/* Create file or directory */
static int64_t ramfs_create(struct vfs_node *dir, const char *name, uint32_t type, uint32_t permissions) {
    if (!dir || !name) return -1;
    
    serial_puts("[RAMFS] Creating neural node: ");
    serial_puts(name);
    serial_puts(" (type: ");
    extern const char *vfs_get_type_name(uint32_t type);
    serial_puts(vfs_get_type_name(type));
    serial_puts(")\n");
    
    /* Create new node */
    extern struct vfs_node *vfs_node_create(const char *name, uint32_t type);
    struct vfs_node *new_node = vfs_node_create(name, type);
    if (!new_node) {
        serial_puts("[ERROR] Failed to create neural node\n");
        return -1;
    }
    
    new_node->permissions = permissions;
    new_node->ops = &ramfs_file_ops;
    new_node->filesystem = dir->filesystem;
    
    /* Add to parent directory */
    extern int vfs_node_add_child(struct vfs_node *parent, struct vfs_node *child);
    if (vfs_node_add_child(dir, new_node) != 0) {
        serial_puts("[ERROR] Failed to add neural node to directory\n");
        vfs_node_destroy(new_node);
        return -1;
    }
    
    /* Update filesystem statistics */
    struct ramfs_data *ramfs_data = (struct ramfs_data *)dir->filesystem->private_data;
    if (type == FS_TYPE_DIRECTORY) {
        ramfs_data->dir_count++;
    } else {
        ramfs_data->file_count++;
    }
    
    serial_puts("[SUCCESS] Neural node created in memory matrix\n");
    return 0;
}

/* Remove file or directory */
static int64_t ramfs_remove(struct vfs_node *dir, const char *name) {
    if (!dir || !name) return -1;
    
    serial_puts("[RAMFS] Removing neural node: ");
    serial_puts(name);
    serial_puts("\n");
    
    /* Find the node */
    struct vfs_node *node = ramfs_lookup(dir, name);
    if (!node) {
        return -1;
    }
    
    /* Check if directory is empty */
    if (node->type == FS_TYPE_DIRECTORY && node->children) {
        serial_puts("[ERROR] Neural directory not empty\n");
        return -1;
    }
    
    /* Free file data if it exists */
    if (node->fs_data) {
        struct ramfs_file_data *file_data = (struct ramfs_file_data *)node->fs_data;
        if (file_data->data) {
            kfree(file_data->data);
        }
        kfree(file_data);
    }
    
    /* Remove from parent directory */
    extern int vfs_node_remove_child(struct vfs_node *parent, const char *name);
    vfs_node_remove_child(dir, name);
    
    /* Update filesystem statistics */
    struct ramfs_data *ramfs_data = (struct ramfs_data *)dir->filesystem->private_data;
    if (node->type == FS_TYPE_DIRECTORY) {
        ramfs_data->dir_count--;
    } else {
        ramfs_data->file_count--;
    }
    
    /* Destroy the node */
    vfs_node_destroy(node);
    
    serial_puts("[SUCCESS] Neural node removed from memory matrix\n");
    return 0;
}

/* Get filesystem information */
struct filesystem *ramfs_get_filesystem(void) {
    return &ramfs_filesystem;
}/* ramfs.c - Brandon Media OS RAM File System */
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

/* RamFS file data structure */
struct ramfs_file_data {
    uint8_t *data;          /* Neural data buffer */
    uint64_t size;          /* Current neural data size */
    uint64_t capacity;      /* Neural buffer capacity */
};

/* RamFS filesystem data */
struct ramfs_data {
    uint64_t total_size;    /* Total neural memory used */
    uint64_t file_count;    /* Number of neural files */
    uint64_t dir_count;     /* Number of neural directories */
};

/* Forward declarations */
static int64_t ramfs_open(struct vfs_node *node, uint32_t flags);
static int64_t ramfs_close(struct vfs_node *node);
static int64_t ramfs_read(struct vfs_node *node, void *buffer, uint64_t size, uint64_t offset);
static int64_t ramfs_write(struct vfs_node *node, const void *buffer, uint64_t size, uint64_t offset);
static int64_t ramfs_truncate(struct vfs_node *node, uint64_t size);
static struct vfs_node *ramfs_lookup(struct vfs_node *dir, const char *name);
static int64_t ramfs_create(struct vfs_node *dir, const char *name, uint32_t type, uint32_t permissions);
static int64_t ramfs_remove(struct vfs_node *dir, const char *name);

/* RamFS file operations */
static struct file_operations ramfs_file_ops = {
    .open = ramfs_open,
    .close = ramfs_close,
    .read = ramfs_read,
    .write = ramfs_write,
    .truncate = ramfs_truncate,
};

/* RamFS directory operations */
static struct directory_operations ramfs_dir_ops = {
    .lookup = ramfs_lookup,
    .create = ramfs_create,
    .remove = ramfs_remove,
};

/* RamFS filesystem structure */
static struct filesystem ramfs_filesystem = {
    .name = "ramfs",
    .magic = 0x52414D46,  /* "RAMF" */
    .file_ops = &ramfs_file_ops,
    .dir_ops = &ramfs_dir_ops,
};

/* Initialize RamFS */
int ramfs_init(void) {
    serial_puts("[NEXUS] Initializing neural memory filesystem...\n");
    
    /* Allocate filesystem private data */
    struct ramfs_data *ramfs_data = (struct ramfs_data *)kmalloc(sizeof(struct ramfs_data));
    if (!ramfs_data) {
        serial_puts("[ERROR] Failed to allocate neural memory filesystem data\n");
        return -1;
    }
    
    /* Initialize filesystem data */
    memory_set(ramfs_data, 0, sizeof(struct ramfs_data));
    ramfs_filesystem.private_data = ramfs_data;
    
    /* Create root directory for RamFS */
    ramfs_filesystem.root = vfs_node_create("ramfs_root", FS_TYPE_DIRECTORY);
    if (!ramfs_filesystem.root) {
        serial_puts("[ERROR] Failed to create neural filesystem root\n");
        kfree(ramfs_data);
        return -1;
    }
    
    ramfs_filesystem.root->ops = &ramfs_file_ops;
    ramfs_filesystem.root->filesystem = &ramfs_filesystem;
    ramfs_data->dir_count = 1;
    
    /* Register the filesystem */
    extern int vfs_register_filesystem(struct filesystem *fs);
    if (vfs_register_filesystem(&ramfs_filesystem) != 0) {
        serial_puts("[ERROR] Failed to register neural memory filesystem\n");
        vfs_node_destroy(ramfs_filesystem.root);
        kfree(ramfs_data);
        return -1;
    }
    
    serial_puts("[RAMFS] Neural memory filesystem registered\n");
    serial_puts("[RAMFS] Root directory: ");
    print_hex((uint64_t)ramfs_filesystem.root);
    serial_puts("\n");
    
    return 0;
}

/* Open file */
static int64_t ramfs_open(struct vfs_node *node, uint32_t flags) {
    if (!node) return -1;
    
    serial_puts("[RAMFS] Opening neural file: ");
    serial_puts(node->name);
    serial_puts("\n");
    
    /* For directories, no special handling needed */
    if (node->type == FS_TYPE_DIRECTORY) {
        return 0;
    }
    
    /* For regular files, ensure file data exists */
    if (node->type == FS_TYPE_REGULAR && !node->fs_data) {
        struct ramfs_file_data *file_data = (struct ramfs_file_data *)kmalloc(sizeof(struct ramfs_file_data));
        if (!file_data) {
            serial_puts("[ERROR] Failed to allocate neural file data\n");
            return -1;
        }
        
        memory_set(file_data, 0, sizeof(struct ramfs_file_data));
        node->fs_data = file_data;
    }
    
    /* Update access time */
    extern uint64_t vfs_get_current_time(void);
    node->accessed_time = vfs_get_current_time();
    
    return 0;
}

/* Close file */
static int64_t ramfs_close(struct vfs_node *node) {
    if (!node) return -1;
    
    serial_puts("[RAMFS] Closing neural file: ");
    serial_puts(node->name);
    serial_puts("\n");
    
    /* No special handling needed for close */
    return 0;
}

/* Read from file */
static int64_t ramfs_read(struct vfs_node *node, void *buffer, uint64_t size, uint64_t offset) {
    if (!node || !buffer) return -1;
    
    if (node->type != FS_TYPE_REGULAR) {
        return -1;  /* Can only read regular files */
    }
    
    struct ramfs_file_data *file_data = (struct ramfs_file_data *)node->fs_data;
    if (!file_data || !file_data->data) {
        return 0;  /* Empty file */
    }
    
    /* Check bounds */
    if (offset >= file_data->size) {
        return 0;  /* EOF */
    }
    
    /* Calculate actual read size */
    uint64_t available = file_data->size - offset;
    uint64_t read_size = (size < available) ? size : available;
    
    /* Copy data */
    memory_copy(buffer, file_data->data + offset, read_size);
    
    /* Update access time */
    extern uint64_t vfs_get_current_time(void);
    node->accessed_time = vfs_get_current_time();
    
    serial_puts("[RAMFS] Neural data read: ");
    print_dec(read_size);
    serial_puts(" bytes from offset ");
    print_dec(offset);
    serial_puts("\n");
    
    return read_size;
}

/* Write to file */
static int64_t ramfs_write(struct vfs_node *node, const void *buffer, uint64_t size, uint64_t offset) {
    if (!node || !buffer || size == 0) return -1;
    
    if (node->type != FS_TYPE_REGULAR) {
        return -1;  /* Can only write to regular files */
    }
    
    struct ramfs_file_data *file_data = (struct ramfs_file_data *)node->fs_data;
    if (!file_data) {
        /* Create file data if it doesn't exist */
        file_data = (struct ramfs_file_data *)kmalloc(sizeof(struct ramfs_file_data));
        if (!file_data) {
            serial_puts("[ERROR] Failed to allocate neural file data\n");
            return -1;
        }
        memory_set(file_data, 0, sizeof(struct ramfs_file_data));
        node->fs_data = file_data;
    }
    
    /* Calculate required size */
    uint64_t required_size = offset + size;
    
    /* Expand buffer if necessary */
    if (required_size > file_data->capacity) {
        uint64_t new_capacity = required_size * 2;  /* Double capacity */
        uint8_t *new_data = (uint8_t *)kmalloc(new_capacity);
        if (!new_data) {
            serial_puts("[ERROR] Failed to expand neural data buffer\n");
            return -1;
        }
        
        /* Copy existing data */
        if (file_data->data && file_data->size > 0) {
            memory_copy(new_data, file_data->data, file_data->size);
            kfree(file_data->data);
        }
        
        file_data->data = new_data;
        file_data->capacity = new_capacity;
    }
    
    /* Write data */
    memory_copy(file_data->data + offset, buffer, size);
    
    /* Update file size if necessary */
    if (required_size > file_data->size) {
        file_data->size = required_size;
        node->size = file_data->size;
    }
    
    /* Update modification time */
    extern uint64_t vfs_get_current_time(void);
    node->modified_time = vfs_get_current_time();
    
    serial_puts("[RAMFS] Neural data written: ");
    print_dec(size);
    serial_puts(" bytes at offset ");
    print_dec(offset);
    serial_puts("\n");
    
    return size;
}

/* Truncate file */
static int64_t ramfs_truncate(struct vfs_node *node, uint64_t size) {
    if (!node) return -1;
    
    if (node->type != FS_TYPE_REGULAR) {
        return -1;
    }
    
    struct ramfs_file_data *file_data = (struct ramfs_file_data *)node->fs_data;
    if (!file_data) {
        /* Create empty file data */
        file_data = (struct ramfs_file_data *)kmalloc(sizeof(struct ramfs_file_data));
        if (!file_data) return -1;
        memory_set(file_data, 0, sizeof(struct ramfs_file_data));
        node->fs_data = file_data;
    }
    
    /* Update size */
    file_data->size = size;
    node->size = size;
    
    /* Update modification time */
    extern uint64_t vfs_get_current_time(void);
    node->modified_time = vfs_get_current_time();
    
    serial_puts("[RAMFS] Neural file truncated to ");
    print_dec(size);
    serial_puts(" bytes\n");
    
    return 0;
}

/* Lookup file in directory */
static struct vfs_node *ramfs_lookup(struct vfs_node *dir, const char *name) {
    if (!dir || !name) return NULL;
    
    extern struct vfs_node *vfs_node_lookup(struct vfs_node *parent, const char *name);
    return vfs_node_lookup(dir, name);
}

/* Create file or directory */
static int64_t ramfs_create(struct vfs_node *dir, const char *name, uint32_t type, uint32_t permissions) {
    if (!dir || !name) return -1;
    
    serial_puts("[RAMFS] Creating neural node: ");
    serial_puts(name);
    serial_puts(" (type: ");
    extern const char *vfs_get_type_name(uint32_t type);
    serial_puts(vfs_get_type_name(type));
    serial_puts(")\n");
    
    /* Create new node */
    extern struct vfs_node *vfs_node_create(const char *name, uint32_t type);
    struct vfs_node *new_node = vfs_node_create(name, type);
    if (!new_node) {
        serial_puts("[ERROR] Failed to create neural node\n");
        return -1;
    }
    
    new_node->permissions = permissions;
    new_node->ops = &ramfs_file_ops;
    new_node->filesystem = dir->filesystem;
    
    /* Add to parent directory */
    extern int vfs_node_add_child(struct vfs_node *parent, struct vfs_node *child);
    if (vfs_node_add_child(dir, new_node) != 0) {
        serial_puts("[ERROR] Failed to add neural node to directory\n");
        vfs_node_destroy(new_node);
        return -1;
    }
    
    /* Update filesystem statistics */
    struct ramfs_data *ramfs_data = (struct ramfs_data *)dir->filesystem->private_data;
    if (type == FS_TYPE_DIRECTORY) {
        ramfs_data->dir_count++;
    } else {
        ramfs_data->file_count++;
    }
    
    serial_puts("[SUCCESS] Neural node created in memory matrix\n");
    return 0;
}

/* Remove file or directory */
static int64_t ramfs_remove(struct vfs_node *dir, const char *name) {
    if (!dir || !name) return -1;
    
    serial_puts("[RAMFS] Removing neural node: ");
    serial_puts(name);
    serial_puts("\n");
    
    /* Find the node */
    struct vfs_node *node = ramfs_lookup(dir, name);
    if (!node) {
        return -1;
    }
    
    /* Check if directory is empty */
    if (node->type == FS_TYPE_DIRECTORY && node->children) {
        serial_puts("[ERROR] Neural directory not empty\n");
        return -1;
    }
    
    /* Free file data if it exists */
    if (node->fs_data) {
        struct ramfs_file_data *file_data = (struct ramfs_file_data *)node->fs_data;
        if (file_data->data) {
            kfree(file_data->data);
        }
        kfree(file_data);
    }
    
    /* Remove from parent directory */
    extern int vfs_node_remove_child(struct vfs_node *parent, const char *name);
    vfs_node_remove_child(dir, name);
    
    /* Update filesystem statistics */
    struct ramfs_data *ramfs_data = (struct ramfs_data *)dir->filesystem->private_data;
    if (node->type == FS_TYPE_DIRECTORY) {
        ramfs_data->dir_count--;
    } else {
        ramfs_data->file_count--;
    }
    
    /* Destroy the node */
    vfs_node_destroy(node);
    
    serial_puts("[SUCCESS] Neural node removed from memory matrix\n");
    return 0;
}

/* Get filesystem information */
struct filesystem *ramfs_get_filesystem(void) {
    return &ramfs_filesystem;
}