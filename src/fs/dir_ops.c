/* dir_ops.c - Brandon Media OS Directory Operations */
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

static char *str_cpy(char *dest, const char *src) {
    char *original_dest = dest;
    while ((*dest++ = *src++));
    return original_dest;
}

static char *str_rchr(const char *s, int c) {
    const char *last = NULL;
    while (*s) {
        if (*s == c) {
            last = s;
        }
        s++;
    }
    return (char *)last;
}

/* Extract directory path and filename from full path */
static int split_path(const char *path, char *dir_path, char *filename) {
    if (!path || !dir_path || !filename) return -1;
    
    const char *last_slash = str_rchr(path, '/');
    if (!last_slash) {
        /* No directory, just filename */
        str_cpy(dir_path, "/");
        str_cpy(filename, path);
    } else if (last_slash == path) {
        /* Root directory */
        str_cpy(dir_path, "/");
        str_cpy(filename, last_slash + 1);
    } else {
        /* Copy directory part */
        size_t dir_len = last_slash - path;
        memory_copy(dir_path, path, dir_len);
        dir_path[dir_len] = '\0';
        str_cpy(filename, last_slash + 1);
    }
    
    return 0;
}

/* Create a directory */
int vfs_mkdir(const char *path, uint32_t permissions) {
    if (!path) {
        serial_puts("[ERROR] Invalid neural directory path\n");
        return -1;
    }
    
    serial_puts("[DIR_OPS] Creating neural directory: ");
    serial_puts(path);
    serial_puts("\n");
    
    /* Split path into directory and name */
    char dir_path[FS_MAX_PATH];
    char dir_name[FS_MAX_NAME];
    
    if (split_path(path, dir_path, dir_name) != 0) {
        serial_puts("[ERROR] Failed to parse neural directory path\n");
        return -1;
    }
    
    if (str_len(dir_name) == 0) {
        serial_puts("[ERROR] Empty neural directory name\n");
        return -1;
    }
    
    /* Find parent directory */
    extern struct vfs_node *vfs_resolve_path(const char *path);
    struct vfs_node *parent = vfs_resolve_path(dir_path);
    if (!parent) {
        serial_puts("[ERROR] Parent neural directory not found: ");
        serial_puts(dir_path);
        serial_puts("\n");
        return -1;
    }
    
    if (parent->type != FS_TYPE_DIRECTORY) {
        serial_puts("[ERROR] Parent is not a neural directory\n");
        return -1;
    }
    
    /* Check if directory already exists */
    extern struct vfs_node *vfs_node_lookup(struct vfs_node *parent, const char *name);
    if (vfs_node_lookup(parent, dir_name)) {
        serial_puts("[ERROR] Neural directory already exists\n");
        return -1;
    }
    
    /* Create directory using filesystem operations */
    if (parent->filesystem && parent->filesystem->dir_ops && parent->filesystem->dir_ops->create) {
        int64_t result = parent->filesystem->dir_ops->create(parent, dir_name, FS_TYPE_DIRECTORY, permissions);
        if (result < 0) {
            serial_puts("[ERROR] Failed to create neural directory\n");
            return -1;
        }
    } else {
        serial_puts("[ERROR] No neural directory operations available\n");
        return -1;
    }
    
    serial_puts("[SUCCESS] Neural directory created: ");
    serial_puts(path);
    serial_puts("\n");
    
    return 0;
}

/* Remove a directory */
int vfs_rmdir(const char *path) {
    if (!path) {
        serial_puts("[ERROR] Invalid neural directory path\n");
        return -1;
    }
    
    serial_puts("[DIR_OPS] Removing neural directory: ");
    serial_puts(path);
    serial_puts("\n");
    
    /* Split path into directory and name */
    char dir_path[FS_MAX_PATH];
    char dir_name[FS_MAX_NAME];
    
    if (split_path(path, dir_path, dir_name) != 0) {
        serial_puts("[ERROR] Failed to parse neural directory path\n");
        return -1;
    }
    
    /* Find parent directory */
    extern struct vfs_node *vfs_resolve_path(const char *path);
    struct vfs_node *parent = vfs_resolve_path(dir_path);
    if (!parent) {
        serial_puts("[ERROR] Parent neural directory not found\n");
        return -1;
    }
    
    /* Find the directory to remove */
    extern struct vfs_node *vfs_node_lookup(struct vfs_node *parent, const char *name);
    struct vfs_node *dir = vfs_node_lookup(parent, dir_name);
    if (!dir) {
        serial_puts("[ERROR] Neural directory not found\n");
        return -1;
    }
    
    if (dir->type != FS_TYPE_DIRECTORY) {
        serial_puts("[ERROR] Not a neural directory\n");
        return -1;
    }
    
    /* Check if directory is empty */
    if (dir->children) {
        serial_puts("[ERROR] Neural directory not empty\n");
        return -1;
    }
    
    /* Remove directory using filesystem operations */
    if (parent->filesystem && parent->filesystem->dir_ops && parent->filesystem->dir_ops->remove) {
        int64_t result = parent->filesystem->dir_ops->remove(parent, dir_name);
        if (result < 0) {
            serial_puts("[ERROR] Failed to remove neural directory\n");
            return -1;
        }
    } else {
        serial_puts("[ERROR] No neural directory operations available\n");
        return -1;
    }
    
    serial_puts("[SUCCESS] Neural directory removed: ");
    serial_puts(path);
    serial_puts("\n");
    
    return 0;
}

/* Open a directory for reading */
int vfs_opendir(const char *path) {
    if (!path) {
        serial_puts("[ERROR] Invalid neural directory path\n");
        return -1;
    }
    
    serial_puts("[DIR_OPS] Opening neural directory: ");
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
        serial_puts("[ERROR] Neural directory not found\n");
        return -1;
    }
    
    if (node->type != FS_TYPE_DIRECTORY) {
        serial_puts("[ERROR] Not a neural directory\n");
        return -1;
    }
    
    /* Allocate file descriptor for directory */
    extern struct file_descriptor *fd_allocate(struct process *proc);
    struct file_descriptor *fd = fd_allocate(proc);
    if (!fd) {
        return -1;
    }
    
    /* Initialize directory file descriptor */
    fd->node = node;
    fd->offset = 0;
    fd->flags = FS_PERM_READ;
    fd->mode = 0;
    
    /* Reference the node */
    extern void vfs_node_ref(struct vfs_node *node);
    vfs_node_ref(node);
    
    serial_puts("[SUCCESS] Neural directory opened: ");
    print_dec(fd->fd);
    serial_puts("\n");
    
    return fd->fd;
}

/* Read directory entries */
int vfs_readdir(int dirfd, struct dirent *entry) {
    if (!entry) {
        return -1;
    }
    
    /* Get current process */
    struct process *proc = process_get_current();
    if (!proc) {
        return -1;
    }
    
    /* Get file descriptor */
    extern struct file_descriptor *fd_get(struct process *proc, int fd);
    struct file_descriptor *fd = fd_get(proc, dirfd);
    if (!fd || !fd->node) {
        return -1;
    }
    
    if (fd->node->type != FS_TYPE_DIRECTORY) {
        serial_puts("[ERROR] Not a neural directory\n");
        return -1;
    }
    
    /* Find the nth child (based on offset) */
    struct vfs_node *current = fd->node->children;
    uint64_t index = 0;
    
    while (current && index < fd->offset) {
        current = current->next_sibling;
        index++;
    }
    
    if (!current) {
        /* End of directory */
        return 0;
    }
    
    /* Fill directory entry */
    memory_set(entry, 0, sizeof(struct dirent));
    entry->inode = current->inode;
    entry->type = current->type;
    entry->name_len = str_len(current->name);
    str_cpy(entry->name, current->name);
    
    /* Advance offset for next read */
    fd->offset++;
    
    serial_puts("[DIR_OPS] Neural directory entry read: ");
    serial_puts(current->name);
    serial_puts(" (inode: ");
    print_dec(current->inode);
    serial_puts(")\n");
    
    return 1;  /* Success, entry read */
}

/* Close directory */
int vfs_closedir(int dirfd) {
    serial_puts("[DIR_OPS] Closing neural directory channel: ");
    print_dec(dirfd);
    serial_puts("\n");
    
    /* Use regular file close operation */
    extern int vfs_close(int fd);
    return vfs_close(dirfd);
}

/* Create a file */
int vfs_create_file(const char *path, uint32_t permissions) {
    if (!path) {
        serial_puts("[ERROR] Invalid neural file path\n");
        return -1;
    }
    
    serial_puts("[DIR_OPS] Creating neural file: ");
    serial_puts(path);
    serial_puts("\n");
    
    /* Split path into directory and name */
    char dir_path[FS_MAX_PATH];
    char file_name[FS_MAX_NAME];
    
    if (split_path(path, dir_path, file_name) != 0) {
        serial_puts("[ERROR] Failed to parse neural file path\n");
        return -1;
    }
    
    if (str_len(file_name) == 0) {
        serial_puts("[ERROR] Empty neural file name\n");
        return -1;
    }
    
    /* Find parent directory */
    extern struct vfs_node *vfs_resolve_path(const char *path);
    struct vfs_node *parent = vfs_resolve_path(dir_path);
    if (!parent) {
        serial_puts("[ERROR] Parent neural directory not found: ");
        serial_puts(dir_path);
        serial_puts("\n");
        return -1;
    }
    
    if (parent->type != FS_TYPE_DIRECTORY) {
        serial_puts("[ERROR] Parent is not a neural directory\n");
        return -1;
    }
    
    /* Check if file already exists */
    extern struct vfs_node *vfs_node_lookup(struct vfs_node *parent, const char *name);
    if (vfs_node_lookup(parent, file_name)) {
        serial_puts("[ERROR] Neural file already exists\n");
        return -1;
    }
    
    /* Create file using filesystem operations */
    if (parent->filesystem && parent->filesystem->dir_ops && parent->filesystem->dir_ops->create) {
        int64_t result = parent->filesystem->dir_ops->create(parent, file_name, FS_TYPE_REGULAR, permissions);
        if (result < 0) {
            serial_puts("[ERROR] Failed to create neural file\n");
            return -1;
        }
    } else {
        serial_puts("[ERROR] No neural directory operations available\n");
        return -1;
    }
    
    serial_puts("[SUCCESS] Neural file created: ");
    serial_puts(path);
    serial_puts("\n");
    
    return 0;
}

/* Remove a file */
int vfs_unlink(const char *path) {
    if (!path) {
        serial_puts("[ERROR] Invalid neural file path\n");
        return -1;
    }
    
    serial_puts("[DIR_OPS] Removing neural file: ");
    serial_puts(path);
    serial_puts("\n");
    
    /* Split path into directory and name */
    char dir_path[FS_MAX_PATH];
    char file_name[FS_MAX_NAME];
    
    if (split_path(path, dir_path, file_name) != 0) {
        serial_puts("[ERROR] Failed to parse neural file path\n");
        return -1;
    }
    
    /* Find parent directory */
    extern struct vfs_node *vfs_resolve_path(const char *path);
    struct vfs_node *parent = vfs_resolve_path(dir_path);
    if (!parent) {
        serial_puts("[ERROR] Parent neural directory not found\n");
        return -1;
    }
    
    /* Find the file to remove */
    extern struct vfs_node *vfs_node_lookup(struct vfs_node *parent, const char *name);
    struct vfs_node *file = vfs_node_lookup(parent, file_name);
    if (!file) {
        serial_puts("[ERROR] Neural file not found\n");
        return -1;
    }
    
    if (file->type == FS_TYPE_DIRECTORY) {
        serial_puts("[ERROR] Cannot unlink neural directory - use rmdir\n");
        return -1;
    }
    
    /* Remove file using filesystem operations */
    if (parent->filesystem && parent->filesystem->dir_ops && parent->filesystem->dir_ops->remove) {
        int64_t result = parent->filesystem->dir_ops->remove(parent, file_name);
        if (result < 0) {
            serial_puts("[ERROR] Failed to remove neural file\n");
            return -1;
        }
    } else {
        serial_puts("[ERROR] No neural directory operations available\n");
        return -1;
    }
    
    serial_puts("[SUCCESS] Neural file removed: ");
    serial_puts(path);
    serial_puts("\n");
    
    return 0;
}