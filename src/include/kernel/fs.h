/* fs.h - Brandon Media OS File System Interface */
#ifndef _FS_H
#define _FS_H

#include <stdint.h>
#include <stddef.h>

/* Brandon Media OS - Neural Data Matrix Definitions */

/* File types - Neural node classifications */
#define FS_TYPE_UNKNOWN     0   /* Unknown neural node */
#define FS_TYPE_REGULAR     1   /* Regular neural data file */
#define FS_TYPE_DIRECTORY   2   /* Neural directory node */
#define FS_TYPE_SYMLINK     3   /* Neural symbolic link */
#define FS_TYPE_DEVICE      4   /* Neural device interface */
#define FS_TYPE_PIPE        5   /* Neural data pipe */
#define FS_TYPE_SOCKET      6   /* Neural network socket */

/* File permissions - Neural access matrix */
#define FS_PERM_READ        0x001   /* Neural read access */
#define FS_PERM_WRITE       0x002   /* Neural write access */
#define FS_PERM_EXEC        0x004   /* Neural execution access */
#define FS_PERM_USER_ALL    0x007   /* All user neural permissions */
#define FS_PERM_GROUP_ALL   0x038   /* All group neural permissions */
#define FS_PERM_OTHER_ALL   0x1C0   /* All other neural permissions */
#define FS_PERM_DEFAULT     0x1A4   /* Default neural permissions (644) */

/* File flags */
#define FS_FLAG_HIDDEN      0x001   /* Hidden neural node */
#define FS_FLAG_SYSTEM      0x002   /* System neural file */
#define FS_FLAG_READONLY    0x004   /* Read-only neural data */
#define FS_FLAG_COMPRESSED  0x008   /* Compressed neural storage */
#define FS_FLAG_ENCRYPTED   0x010   /* Encrypted neural data */

/* Maximum path and name lengths */
#define FS_MAX_PATH         4096    /* Maximum neural path length */
#define FS_MAX_NAME         256     /* Maximum neural node name */
#define FS_MAX_OPEN_FILES   256     /* Maximum open neural channels */

/* File descriptor structure */
struct file_descriptor {
    uint32_t fd;                    /* File descriptor number */
    struct vfs_node *node;          /* VFS node pointer */
    uint64_t offset;                /* Current neural position */
    uint32_t flags;                 /* Open flags */
    uint32_t mode;                  /* Access mode */
    struct process *owner;          /* Owning process */
    struct file_descriptor *next;   /* Next in process FD list */
};

/* VFS node structure - Neural data node */
struct vfs_node {
    char name[FS_MAX_NAME];         /* Neural node name */
    uint32_t type;                  /* Neural node type */
    uint32_t permissions;           /* Neural access permissions */
    uint32_t flags;                 /* Neural node flags */
    uint64_t size;                  /* Neural data size */
    uint64_t created_time;          /* Neural creation timestamp */
    uint64_t modified_time;         /* Neural modification timestamp */
    uint64_t accessed_time;         /* Neural access timestamp */
    uint32_t uid;                   /* User ID */
    uint32_t gid;                   /* Group ID */
    uint32_t inode;                 /* Neural inode number */
    
    /* File operations */
    struct file_operations *ops;    /* Neural operation functions */
    
    /* Filesystem specific data */
    void *fs_data;                  /* Filesystem private data */
    struct filesystem *filesystem;  /* Parent filesystem */
    
    /* Directory entries */
    struct vfs_node *parent;        /* Parent neural directory */
    struct vfs_node *children;      /* Child neural nodes */
    struct vfs_node *next_sibling;  /* Next sibling node */
    
    /* Reference counting */
    uint32_t ref_count;             /* Neural reference count */
};

/* File operations structure */
struct file_operations {
    int64_t (*open)(struct vfs_node *node, uint32_t flags);
    int64_t (*close)(struct vfs_node *node);
    int64_t (*read)(struct vfs_node *node, void *buffer, uint64_t size, uint64_t offset);
    int64_t (*write)(struct vfs_node *node, const void *buffer, uint64_t size, uint64_t offset);
    int64_t (*seek)(struct vfs_node *node, uint64_t offset, int whence);
    int64_t (*ioctl)(struct vfs_node *node, uint32_t cmd, void *arg);
    int64_t (*truncate)(struct vfs_node *node, uint64_t size);
    int64_t (*flush)(struct vfs_node *node);
};

/* Directory operations structure */
struct directory_operations {
    struct vfs_node *(*lookup)(struct vfs_node *dir, const char *name);
    int64_t (*create)(struct vfs_node *dir, const char *name, uint32_t type, uint32_t permissions);
    int64_t (*remove)(struct vfs_node *dir, const char *name);
    int64_t (*rename)(struct vfs_node *dir, const char *old_name, const char *new_name);
    int64_t (*readdir)(struct vfs_node *dir, struct vfs_node **entries, uint32_t max_entries);
};

/* Filesystem structure */
struct filesystem {
    char name[32];                  /* Filesystem name */
    uint32_t magic;                 /* Filesystem magic number */
    struct vfs_node *root;          /* Root neural directory */
    struct file_operations *file_ops;      /* File operations */
    struct directory_operations *dir_ops;  /* Directory operations */
    void *private_data;             /* Filesystem private data */
    struct filesystem *next;        /* Next registered filesystem */
};

/* Mount point structure */
struct mount_point {
    char path[FS_MAX_PATH];         /* Neural mount path */
    struct filesystem *filesystem;  /* Mounted neural filesystem */
    struct vfs_node *mount_node;    /* Mount point neural node */
    uint32_t flags;                 /* Mount flags */
    struct mount_point *next;       /* Next mount point */
};

/* Directory entry structure */
struct dirent {
    uint32_t inode;                 /* Neural inode number */
    uint32_t type;                  /* Neural node type */
    uint32_t name_len;              /* Name length */
    char name[FS_MAX_NAME];         /* Neural node name */
};

/* File status structure */
struct file_stat {
    uint32_t inode;                 /* Neural inode number */
    uint32_t type;                  /* Neural node type */
    uint32_t permissions;           /* Neural permissions */
    uint32_t uid;                   /* User ID */
    uint32_t gid;                   /* Group ID */
    uint64_t size;                  /* Neural data size */
    uint64_t blocks;                /* Number of blocks */
    uint64_t created_time;          /* Neural creation time */
    uint64_t modified_time;         /* Neural modification time */
    uint64_t accessed_time;         /* Neural access time */
};

/* Brandon Media OS - VFS Functions */

/* VFS initialization and management */
void vfs_init(void);
void file_ops_init(void);
int ramfs_init(void);
void storage_init(void);
struct storage_device *storage_create_ram_device(const char *name, uint64_t size);
int storage_register_device(struct storage_device *device);
void storage_print_devices(void);
int vfs_register_filesystem(struct filesystem *fs);
int vfs_unregister_filesystem(struct filesystem *fs);
int vfs_mount(const char *path, struct filesystem *fs, uint32_t flags);
int vfs_unmount(const char *path);

/* File operations */
int vfs_open(const char *path, uint32_t flags, uint32_t mode);
int vfs_close(int fd);
int64_t vfs_read(int fd, void *buffer, size_t count);
int64_t vfs_write(int fd, const void *buffer, size_t count);
int64_t vfs_seek(int fd, int64_t offset, int whence);
int vfs_stat(const char *path, struct file_stat *stat);
int vfs_fstat(int fd, struct file_stat *stat);

/* Directory operations */
int vfs_mkdir(const char *path, uint32_t permissions);
int vfs_rmdir(const char *path);
int vfs_opendir(const char *path);
int vfs_readdir(int dirfd, struct dirent *entry);
int vfs_closedir(int dirfd);
int vfs_create_file(const char *path, uint32_t permissions);
int vfs_unlink(const char *path);

/* Path operations */
struct vfs_node *vfs_resolve_path(const char *path);
char *vfs_get_absolute_path(const char *path);
int vfs_path_exists(const char *path);

/* File descriptor management */
struct file_descriptor *fd_allocate(struct process *proc);
void fd_free(struct file_descriptor *fd);
struct file_descriptor *fd_get(struct process *proc, int fd);

/* VFS node operations */
struct vfs_node *vfs_node_create(const char *name, uint32_t type);
void vfs_node_destroy(struct vfs_node *node);
void vfs_node_ref(struct vfs_node *node);
void vfs_node_unref(struct vfs_node *node);
struct vfs_node *vfs_node_lookup(struct vfs_node *parent, const char *name);
int vfs_node_add_child(struct vfs_node *parent, struct vfs_node *child);
int vfs_node_remove_child(struct vfs_node *parent, const char *name);

/* Utility functions */
const char *vfs_get_type_name(uint32_t type);
int vfs_check_permissions(struct vfs_node *node, struct process *proc, uint32_t requested);
uint64_t vfs_get_current_time(void);

/* Error codes specific to filesystem */
#define FS_SUCCESS          0       /* Neural operation successful */
#define FS_ERROR_NOTFOUND   -1      /* Neural node not found */
#define FS_ERROR_EXISTS     -2      /* Neural node already exists */
#define FS_ERROR_NOSPACE    -3      /* No space in neural matrix */
#define FS_ERROR_PERM       -4      /* Neural permission denied */
#define FS_ERROR_INVAL      -5      /* Invalid neural argument */
#define FS_ERROR_ISDIR      -6      /* Is a neural directory */
#define FS_ERROR_NOTDIR     -7      /* Not a neural directory */
#define FS_ERROR_NOTEMPTY   -8      /* Neural directory not empty */
#define FS_ERROR_BUSY       -9      /* Neural resource busy */
#define FS_ERROR_IO         -10     /* Neural I/O error */

#endif /* _FS_H */