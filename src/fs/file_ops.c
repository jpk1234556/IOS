/* file_ops.c - Brandon Media OS File Operations Interface */
#include <stdint.h>
#include "kernel/fs.h"
#include "kernel/memory.h"
#include "kernel/process.h"

/* External functions */
extern void serial_puts(const char *s);
extern void print_hex(uint64_t num);
extern void print_dec(uint64_t num);
extern void *kmalloc(size_t size);
extern void kfree(void *ptr);
extern void memory_set(void *dst, int value, size_t size);
extern void memory_copy(void *dst, const void *src, size_t size);
extern struct process *process_get_current(void);

/* Global file descriptor table */
static struct file_descriptor *global_fd_table[FS_MAX_OPEN_FILES];
static int next_fd = 3;  /* 0, 1, 2 reserved for stdin, stdout, stderr */

/* String utility functions */
static size_t str_len(const char *s) {
    size_t len = 0;
    while (s[len]) len++;
    return len;
}

static int str_cmp(const char *s1, const char *s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(unsigned char*)s1 - *(unsigned char*)s2;
}

/* Initialize file operations */
void file_ops_init(void) {
    serial_puts("[NEXUS] Initializing neural file operation channels...\n");
    
    /* Clear global file descriptor table */
    memory_set(global_fd_table, 0, sizeof(global_fd_table));
    next_fd = 3;
    
    serial_puts("[FILE_OPS] Neural channels initialized\n");
}

/* Allocate a file descriptor */
struct file_descriptor *fd_allocate(struct process *proc) {
    if (!proc) return NULL;
    
    /* Find free file descriptor */
    int fd_num = -1;
    for (int i = 3; i < FS_MAX_OPEN_FILES; i++) {
        if (!global_fd_table[i]) {
            fd_num = i;
            break;
        }
    }
    
    if (fd_num == -1) {
        serial_puts("[ERROR] No free neural channels available\n");
        return NULL;
    }
    
    /* Allocate file descriptor structure */
    struct file_descriptor *fd = (struct file_descriptor *)kmalloc(sizeof(struct file_descriptor));
    if (!fd) {
        serial_puts("[ERROR] Failed to allocate neural channel\n");
        return NULL;
    }
    
    /* Initialize file descriptor */
    memory_set(fd, 0, sizeof(struct file_descriptor));
    fd->fd = fd_num;
    fd->owner = proc;
    
    /* Add to global table */
    global_fd_table[fd_num] = fd;
    
    serial_puts("[FILE_OPS] Neural channel allocated: ");
    print_dec(fd_num);
    serial_puts("\n");
    
    return fd;
}

/* Free a file descriptor */
void fd_free(struct file_descriptor *fd) {
    if (!fd) return;
    
    serial_puts("[FILE_OPS] Freeing neural channel: ");
    print_dec(fd->fd);
    serial_puts("\n");
    
    /* Remove from global table */
    if (fd->fd < FS_MAX_OPEN_FILES) {
        global_fd_table[fd->fd] = NULL;
    }
    
    /* Free the structure */
    kfree(fd);
}

/* Get file descriptor by number */
struct file_descriptor *fd_get(struct process *proc, int fd_num) {
    if (fd_num < 0 || fd_num >= FS_MAX_OPEN_FILES) {
        return NULL;
    }
    
    struct file_descriptor *fd = global_fd_table[fd_num];
    if (!fd || fd->owner != proc) {
        return NULL;
    }
    
    return fd;
}

/* Open a file */
int vfs_open(const char *path, uint32_t flags, uint32_t mode) {
    if (!path) {
        serial_puts("[ERROR] Invalid neural path\n");
        return -1;
    }
    
    serial_puts("[FILE_OPS] Opening neural channel to: ");
    serial_puts(path);
    serial_puts("\n");
    
    /* Get current process */
    struct process *proc = process_get_current();
    if (!proc) {
        serial_puts("[ERROR] No active neural process\n");
        return -1;
    }
    
    /* Resolve path to VFS node */
    extern struct vfs_node *vfs_resolve_path(const char *path);
    struct vfs_node *node = vfs_resolve_path(path);
    if (!node) {
        serial_puts("[ERROR] Neural path not found\n");
        return -1;
    }
    
    /* Allocate file descriptor */
    struct file_descriptor *fd = fd_allocate(proc);
    if (!fd) {
        return -1;
    }
    
    /* Initialize file descriptor */
    fd->node = node;
    fd->offset = 0;
    fd->flags = flags;
    fd->mode = mode;
    
    /* Call filesystem open operation */
    if (node->ops && node->ops->open) {
        int64_t result = node->ops->open(node, flags);
        if (result < 0) {
            serial_puts("[ERROR] Neural file open operation failed\n");
            fd_free(fd);
            return -1;
        }
    }
    
    /* Reference the node */
    extern void vfs_node_ref(struct vfs_node *node);
    vfs_node_ref(node);
    
    serial_puts("[SUCCESS] Neural channel opened: ");
    print_dec(fd->fd);
    serial_puts("\n");
    
    return fd->fd;
}

/* Close a file */
int vfs_close(int fd_num) {
    serial_puts("[FILE_OPS] Closing neural channel: ");
    print_dec(fd_num);
    serial_puts("\n");
    
    /* Get current process */
    struct process *proc = process_get_current();
    if (!proc) {
        serial_puts("[ERROR] No active neural process\n");
        return -1;
    }
    
    /* Get file descriptor */
    struct file_descriptor *fd = fd_get(proc, fd_num);
    if (!fd) {
        serial_puts("[ERROR] Invalid neural channel\n");
        return -1;
    }
    
    /* Call filesystem close operation */
    if (fd->node && fd->node->ops && fd->node->ops->close) {
        fd->node->ops->close(fd->node);
    }
    
    /* Unreference the node */
    if (fd->node) {
        extern void vfs_node_unref(struct vfs_node *node);
        vfs_node_unref(fd->node);
    }
    
    /* Free file descriptor */
    fd_free(fd);
    
    serial_puts("[SUCCESS] Neural channel closed\n");
    return 0;
}

/* Read from a file */
int64_t vfs_read(int fd_num, void *buffer, size_t count) {
    if (!buffer || count == 0) {
        return -1;
    }
    
    /* Get current process */
    struct process *proc = process_get_current();
    if (!proc) {
        return -1;
    }
    
    /* Get file descriptor */
    struct file_descriptor *fd = fd_get(proc, fd_num);
    if (!fd || !fd->node) {
        return -1;
    }
    
    /* Check read permissions */
    if (!(fd->flags & FS_PERM_READ)) {
        serial_puts("[ERROR] Neural channel not open for reading\n");
        return -1;
    }
    
    /* Call filesystem read operation */
    int64_t bytes_read = 0;
    if (fd->node->ops && fd->node->ops->read) {
        bytes_read = fd->node->ops->read(fd->node, buffer, count, fd->offset);
        if (bytes_read > 0) {
            fd->offset += bytes_read;
        }
    }
    
    serial_puts("[FILE_OPS] Neural data stream read: ");
    print_dec(bytes_read);
    serial_puts(" bytes\n");
    
    return bytes_read;
}

/* Write to a file */
int64_t vfs_write(int fd_num, const void *buffer, size_t count) {
    if (!buffer || count == 0) {
        return -1;
    }
    
    /* Get current process */
    struct process *proc = process_get_current();
    if (!proc) {
        return -1;
    }
    
    /* Get file descriptor */
    struct file_descriptor *fd = fd_get(proc, fd_num);
    if (!fd || !fd->node) {
        return -1;
    }
    
    /* Check write permissions */
    if (!(fd->flags & FS_PERM_WRITE)) {
        serial_puts("[ERROR] Neural channel not open for writing\n");
        return -1;
    }
    
    /* Call filesystem write operation */
    int64_t bytes_written = 0;
    if (fd->node->ops && fd->node->ops->write) {
        bytes_written = fd->node->ops->write(fd->node, buffer, count, fd->offset);
        if (bytes_written > 0) {
            fd->offset += bytes_written;
        }
    }
    
    serial_puts("[FILE_OPS] Neural data stream written: ");
    print_dec(bytes_written);
    serial_puts(" bytes\n");
    
    return bytes_written;
}

/* Seek in a file */
int64_t vfs_seek(int fd_num, int64_t offset, int whence) {
    /* Get current process */
    struct process *proc = process_get_current();
    if (!proc) {
        return -1;
    }
    
    /* Get file descriptor */
    struct file_descriptor *fd = fd_get(proc, fd_num);
    if (!fd || !fd->node) {
        return -1;
    }
    
    /* Calculate new offset */
    uint64_t new_offset;
    switch (whence) {
        case 0:  /* SEEK_SET */
            new_offset = offset;
            break;
        case 1:  /* SEEK_CUR */
            new_offset = fd->offset + offset;
            break;
        case 2:  /* SEEK_END */
            new_offset = fd->node->size + offset;
            break;
        default:
            return -1;
    }
    
    /* Update offset */
    fd->offset = new_offset;
    
    serial_puts("[FILE_OPS] Neural channel seek to: ");
    print_dec(new_offset);
    serial_puts("\n");
    
    return new_offset;
}

/* Get file status */
int vfs_stat(const char *path, struct file_stat *stat) {
    if (!path || !stat) {
        return -1;
    }
    
    /* Resolve path to VFS node */
    extern struct vfs_node *vfs_resolve_path(const char *path);
    struct vfs_node *node = vfs_resolve_path(path);
    if (!node) {
        return -1;
    }
    
    /* Fill stat structure */
    memory_set(stat, 0, sizeof(struct file_stat));
    stat->inode = node->inode;
    stat->type = node->type;
    stat->permissions = node->permissions;
    stat->uid = node->uid;
    stat->gid = node->gid;
    stat->size = node->size;
    stat->created_time = node->created_time;
    stat->modified_time = node->modified_time;
    stat->accessed_time = node->accessed_time;
    
    serial_puts("[FILE_OPS] Neural node status retrieved for: ");
    serial_puts(path);
    serial_puts("\n");
    
    return 0;
}

/* Get file status by descriptor */
int vfs_fstat(int fd_num, struct file_stat *stat) {
    if (!stat) {
        return -1;
    }
    
    /* Get current process */
    struct process *proc = process_get_current();
    if (!proc) {
        return -1;
    }
    
    /* Get file descriptor */
    struct file_descriptor *fd = fd_get(proc, fd_num);
    if (!fd || !fd->node) {
        return -1;
    }
    
    /* Fill stat structure */
    memory_set(stat, 0, sizeof(struct file_stat));
    stat->inode = fd->node->inode;
    stat->type = fd->node->type;
    stat->permissions = fd->node->permissions;
    stat->uid = fd->node->uid;
    stat->gid = fd->node->gid;
    stat->size = fd->node->size;
    stat->created_time = fd->node->created_time;
    stat->modified_time = fd->node->modified_time;
    stat->accessed_time = fd->node->accessed_time;
    
    serial_puts("[FILE_OPS] Neural channel status retrieved\n");
    
    return 0;
}