/* vfs.c - Brandon Media OS Virtual File System Core */
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
extern int memory_compare(const void *a, const void *b, size_t size);
extern uint64_t timer_get_ticks(void);
extern struct process *process_get_current(void);

/* VFS global state */
static struct filesystem *registered_filesystems = NULL;
static struct mount_point *mount_points = NULL;
static struct vfs_node *vfs_root = NULL;
static uint32_t next_inode = 1;
static uint32_t next_fd = 3;  /* 0, 1, 2 reserved for stdin, stdout, stderr */

/* VFS statistics */
static struct {
    uint64_t nodes_created;
    uint64_t nodes_destroyed;
    uint64_t files_opened;
    uint64_t files_closed;
    uint64_t bytes_read;
    uint64_t bytes_written;
} vfs_stats = {0};

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

/* Initialize VFS */
void vfs_init(void) {
    serial_puts("[NEXUS] Initializing neural data matrix...\\n");
    
    /* Clear global state */
    registered_filesystems = NULL;
    mount_points = NULL;
    vfs_root = NULL;
    next_inode = 1;
    
    /* Create root directory */
    vfs_root = vfs_node_create("/", FS_TYPE_DIRECTORY);
    if (!vfs_root) {
        serial_puts("[FATAL] Failed to create neural root directory\\n");
        return;
    }
    
    vfs_root->permissions = FS_PERM_USER_ALL | FS_PERM_GROUP_ALL | FS_PERM_OTHER_ALL;
    vfs_root->parent = NULL;  /* Root has no parent */
    
    serial_puts("[MATRIX] Neural root directory created at: ");
    print_hex((uint64_t)vfs_root);
    serial_puts("\\n");
    
    /* Clear statistics */
    memory_set(&vfs_stats, 0, sizeof(vfs_stats));
    
    serial_puts("[NEXUS] Neural data matrix online\\n");
}

/* Register a filesystem */
int vfs_register_filesystem(struct filesystem *fs) {
    if (!fs) return FS_ERROR_INVAL;
    
    serial_puts("[VFS] Registering neural filesystem: ");
    serial_puts(fs->name);
    serial_puts("\\n");
    
    /* Add to registered filesystems list */
    fs->next = registered_filesystems;
    registered_filesystems = fs;
    
    serial_puts("[SUCCESS] Neural filesystem registered\\n");
    return FS_SUCCESS;
}

/* Create VFS node */
struct vfs_node *vfs_node_create(const char *name, uint32_t type) {
    if (!name) return NULL;
    
    struct vfs_node *node = (struct vfs_node *)kmalloc(sizeof(struct vfs_node));
    if (!node) {
        serial_puts("[ERROR] Neural node allocation failed\\n");
        return NULL;
    }
    
    /* Initialize node */
    memory_set(node, 0, sizeof(struct vfs_node));
    str_cpy(node->name, name);
    node->type = type;
    node->inode = next_inode++;
    node->ref_count = 1;
    
    /* Set timestamps */
    uint64_t current_time = timer_get_ticks();
    node->created_time = current_time;
    node->modified_time = current_time;
    node->accessed_time = current_time;
    
    /* Set default permissions */
    node->permissions = FS_PERM_DEFAULT;
    
    vfs_stats.nodes_created++;
    
    serial_puts("[MATRIX] Neural node created: ");
    serial_puts(name);
    serial_puts(" (inode: ");
    print_dec(node->inode);
    serial_puts(")\\n");
    
    return node;
}

/* Destroy VFS node */
void vfs_node_destroy(struct vfs_node *node) {
    if (!node) return;
    
    serial_puts("[MATRIX] Destroying neural node: ");
    serial_puts(node->name);
    serial_puts(" (inode: ");
    print_dec(node->inode);
    serial_puts(")\\n");
    
    /* Remove from parent if it has one */
    if (node->parent) {
        vfs_node_remove_child(node->parent, node->name);
    }
    
    /* Free the node */
    kfree(node);
    vfs_stats.nodes_destroyed++;
}

/* Reference VFS node */
void vfs_node_ref(struct vfs_node *node) {
    if (!node) return;
    node->ref_count++;
}

/* Unreference VFS node */
void vfs_node_unref(struct vfs_node *node) {
    if (!node) return;
    
    node->ref_count--;
    if (node->ref_count == 0) {
        vfs_node_destroy(node);
    }
}

/* Add child node to directory */
int vfs_node_add_child(struct vfs_node *parent, struct vfs_node *child) {
    if (!parent || !child) return FS_ERROR_INVAL;
    
    if (parent->type != FS_TYPE_DIRECTORY) {
        return FS_ERROR_NOTDIR;
    }
    
    /* Check if child already exists */
    if (vfs_node_lookup(parent, child->name)) {
        return FS_ERROR_EXISTS;
    }
    
    /* Add to children list */
    child->next_sibling = parent->children;
    parent->children = child;
    child->parent = parent;
    
    /* Update parent modification time */
    parent->modified_time = timer_get_ticks();
    
    serial_puts("[MATRIX] Neural node added to directory: ");
    serial_puts(child->name);
    serial_puts(" -> ");
    serial_puts(parent->name);
    serial_puts("\\n");
    
    return FS_SUCCESS;
}

/* Remove child node from directory */
int vfs_node_remove_child(struct vfs_node *parent, const char *name) {
    if (!parent || !name) return FS_ERROR_INVAL;
    
    if (parent->type != FS_TYPE_DIRECTORY) {
        return FS_ERROR_NOTDIR;
    }
    
    struct vfs_node *current = parent->children;
    struct vfs_node *prev = NULL;
    
    /* Find the child */
    while (current) {
        if (str_cmp(current->name, name) == 0) {
            /* Remove from list */
            if (prev) {
                prev->next_sibling = current->next_sibling;
            } else {
                parent->children = current->next_sibling;
            }
            
            current->parent = NULL;
            current->next_sibling = NULL;
            
            /* Update parent modification time */
            parent->modified_time = timer_get_ticks();
            
            serial_puts("[MATRIX] Neural node removed from directory: ");
            serial_puts(name);
            serial_puts("\\n");
            
            return FS_SUCCESS;
        }
        
        prev = current;
        current = current->next_sibling;
    }
    
    return FS_ERROR_NOTFOUND;
}

/* Lookup child node in directory */
struct vfs_node *vfs_node_lookup(struct vfs_node *parent, const char *name) {
    if (!parent || !name) return NULL;
    
    if (parent->type != FS_TYPE_DIRECTORY) {
        return NULL;
    }
    
    struct vfs_node *current = parent->children;
    while (current) {
        if (str_cmp(current->name, name) == 0) {
            /* Update access time */
            current->accessed_time = timer_get_ticks();
            return current;
        }
        current = current->next_sibling;
    }
    
    return NULL;
}

/* Resolve path to VFS node */
struct vfs_node *vfs_resolve_path(const char *path) {
    if (!path) return NULL;
    
    serial_puts("[VFS] Resolving neural path: ");
    serial_puts(path);
    serial_puts("\\n");
    
    /* Handle root path */
    if (str_cmp(path, "/") == 0) {
        return vfs_root;
    }
    
    /* Start from root */
    struct vfs_node *current = vfs_root;
    
    /* Skip leading slash */
    if (path[0] == '/') {
        path++;
    }
    
    /* Parse path components */
    char component[FS_MAX_NAME];
    int comp_len = 0;
    
    while (*path) {
        if (*path == '/' || comp_len >= FS_MAX_NAME - 1) {
            if (comp_len > 0) {
                component[comp_len] = 0;
                
                /* Lookup component in current directory */
                current = vfs_node_lookup(current, component);
                if (!current) {
                    serial_puts("[VFS] Neural path component not found: ");
                    serial_puts(component);
                    serial_puts("\\n");
                    return NULL;
                }
                
                comp_len = 0;
            }
            
            if (*path == '/') {
                path++;
            }
        } else {
            component[comp_len++] = *path++;
        }
    }
    
    /* Handle final component */
    if (comp_len > 0) {
        component[comp_len] = 0;
        current = vfs_node_lookup(current, component);
    }
    
    if (current) {
        serial_puts("[SUCCESS] Neural path resolved to inode: ");
        print_dec(current->inode);
        serial_puts("\\n");
    } else {
        serial_puts("[ERROR] Neural path resolution failed\\n");
    }
    
    return current;
}

/* Get type name string */
const char *vfs_get_type_name(uint32_t type) {
    switch (type) {
        case FS_TYPE_REGULAR:   return "Neural Data File";
        case FS_TYPE_DIRECTORY: return "Neural Directory";
        case FS_TYPE_SYMLINK:   return "Neural Symbolic Link";
        case FS_TYPE_DEVICE:    return "Neural Device Interface";
        case FS_TYPE_PIPE:      return "Neural Data Pipe";
        case FS_TYPE_SOCKET:    return "Neural Network Socket";
        default:                return "Unknown Neural Node";
    }
}

/* Check permissions */
int vfs_check_permissions(struct vfs_node *node, struct process *proc, uint32_t requested) {
    if (!node || !proc) return 0;
    
    /* Root can access everything */
    if (proc->pid == 0) return 1;
    
    /* Check user permissions */
    uint32_t user_perms = node->permissions & FS_PERM_USER_ALL;
    if ((user_perms & requested) == requested) {
        return 1;
    }
    
    return 0;
}

/* Get current time */
uint64_t vfs_get_current_time(void) {
    return timer_get_ticks();
}