/* neural_app.h - Brandon Media OS Neural Application Framework
 * Enhanced 64-bit Userland Application Interface
 */

#ifndef NEURAL_APP_H
#define NEURAL_APP_H

#include <stdint.h>
#include <stddef.h>

/* Neural Application Framework Version */
#define NEURAL_APP_VERSION_MAJOR    1
#define NEURAL_APP_VERSION_MINOR    0
#define NEURAL_APP_VERSION_PATCH    0

/* Neural Application Types */
typedef enum {
    NEURAL_APP_TYPE_SYSTEM = 0,     /* System applications */
    NEURAL_APP_TYPE_UTILITY,        /* Utility applications */
    NEURAL_APP_TYPE_MEDIA,          /* Media applications */
    NEURAL_APP_TYPE_CYBERPUNK,      /* Cyberpunk-themed applications */
    NEURAL_APP_TYPE_DEVELOPER,      /* Developer tools */
    NEURAL_APP_TYPE_NETWORK,        /* Network applications */
    NEURAL_APP_TYPE_SECURITY,       /* Security applications */
    NEURAL_APP_TYPE_GAME            /* Game applications */
} neural_app_type_t;

/* Neural Application Priority */
typedef enum {
    NEURAL_PRIORITY_CRITICAL = 0,   /* Critical system priority */
    NEURAL_PRIORITY_HIGH = 1,       /* High priority */
    NEURAL_PRIORITY_NORMAL = 2,     /* Normal priority */
    NEURAL_PRIORITY_LOW = 3,        /* Low priority */
    NEURAL_PRIORITY_BACKGROUND = 4  /* Background priority */
} neural_priority_t;

/* Neural Application Status */
typedef enum {
    NEURAL_STATUS_INITIALIZING = 0,
    NEURAL_STATUS_RUNNING,
    NEURAL_STATUS_SUSPENDED,
    NEURAL_STATUS_TERMINATING,
    NEURAL_STATUS_ERROR
} neural_status_t;

/* Neural Application Permissions */
#define NEURAL_PERM_READ            0x01
#define NEURAL_PERM_WRITE           0x02
#define NEURAL_PERM_EXECUTE         0x04
#define NEURAL_PERM_NETWORK         0x08
#define NEURAL_PERM_GRAPHICS        0x10
#define NEURAL_PERM_SYSTEM          0x20
#define NEURAL_PERM_DEVICE          0x40
#define NEURAL_PERM_ADMIN           0x80

/* Neural Application Context */
struct neural_app_context {
    char name[64];
    char version[16];
    neural_app_type_t type;
    neural_priority_t priority;
    neural_status_t status;
    uint32_t permissions;
    uint64_t memory_limit;
    uint64_t cpu_limit;
    uint32_t pid;
    uint64_t start_time;
    uint64_t runtime;
    const char *description;
    const char *author;
    void *private_data;
};

/* Neural Application Interface */
struct neural_app_interface {
    int (*init)(struct neural_app_context *ctx);
    int (*run)(struct neural_app_context *ctx, int argc, char *argv[]);
    int (*suspend)(struct neural_app_context *ctx);
    int (*resume)(struct neural_app_context *ctx);
    int (*cleanup)(struct neural_app_context *ctx);
    void (*signal_handler)(struct neural_app_context *ctx, int signal);
};

/* Neural Application Registration */
struct neural_app_registration {
    const char *name;
    const char *description;
    neural_app_type_t type;
    uint32_t required_permissions;
    struct neural_app_interface interface;
};

/* Graphics Context for Neural Applications */
struct neural_graphics_context {
    uint32_t width;
    uint32_t height;
    uint32_t bpp;
    uint32_t *framebuffer;
    uint32_t pitch;
    int double_buffered;
    uint32_t foreground_color;
    uint32_t background_color;
    int cursor_x;
    int cursor_y;
};

/* Network Context for Neural Applications */
struct neural_network_context {
    int socket_fd;
    char local_ip[16];
    char remote_ip[16];
    uint16_t local_port;
    uint16_t remote_port;
    int protocol;  /* TCP=1, UDP=2 */
    int connected;
};

/* File System Context for Neural Applications */
struct neural_filesystem_context {
    char current_directory[256];
    int max_open_files;
    int open_file_count;
    void **open_files;
};

/* Neural Application Framework Functions */

/* Application Lifecycle */
int neural_app_register(struct neural_app_registration *reg);
int neural_app_initialize(struct neural_app_context *ctx);
int neural_app_run(struct neural_app_context *ctx, int argc, char *argv[]);
void neural_app_terminate(struct neural_app_context *ctx);
int neural_app_suspend(struct neural_app_context *ctx);
int neural_app_resume(struct neural_app_context *ctx);

/* Resource Management */
int neural_app_request_memory(struct neural_app_context *ctx, size_t size);
void neural_app_release_memory(struct neural_app_context *ctx, void *ptr);
int neural_app_request_permission(struct neural_app_context *ctx, uint32_t permission);
int neural_app_check_permission(struct neural_app_context *ctx, uint32_t permission);

/* Graphics Interface */
int neural_graphics_init(struct neural_graphics_context *gfx);
void neural_graphics_cleanup(struct neural_graphics_context *gfx);
void neural_graphics_clear(struct neural_graphics_context *gfx, uint32_t color);
void neural_graphics_put_pixel(struct neural_graphics_context *gfx, int x, int y, uint32_t color);
void neural_graphics_draw_rect(struct neural_graphics_context *gfx, int x, int y, int width, int height, uint32_t color);
void neural_graphics_draw_text(struct neural_graphics_context *gfx, int x, int y, const char *text, uint32_t color);
void neural_graphics_flip(struct neural_graphics_context *gfx);

/* Network Interface */
int neural_network_init(struct neural_network_context *net);
void neural_network_cleanup(struct neural_network_context *net);
int neural_network_connect(struct neural_network_context *net, const char *ip, uint16_t port);
int neural_network_listen(struct neural_network_context *net, uint16_t port);
int neural_network_send(struct neural_network_context *net, const void *data, size_t len);
int neural_network_receive(struct neural_network_context *net, void *buffer, size_t buffer_size);

/* File System Interface */
int neural_fs_init(struct neural_filesystem_context *fs);
void neural_fs_cleanup(struct neural_filesystem_context *fs);
int neural_fs_open(struct neural_filesystem_context *fs, const char *path, int flags);
void neural_fs_close(struct neural_filesystem_context *fs, int fd);
ssize_t neural_fs_read(struct neural_filesystem_context *fs, int fd, void *buffer, size_t count);
ssize_t neural_fs_write(struct neural_filesystem_context *fs, int fd, const void *data, size_t count);
int neural_fs_seek(struct neural_filesystem_context *fs, int fd, off_t offset, int whence);

/* Utility Functions */
void neural_log(neural_app_type_t type, const char *message);
void neural_error(const char *message);
void neural_debug(const char *message);
uint64_t neural_get_time(void);
void neural_sleep(uint32_t milliseconds);
int neural_random(void);

/* System Information */
int neural_get_system_info(char *buffer, size_t buffer_size);
int neural_get_cpu_info(char *buffer, size_t buffer_size);
int neural_get_memory_info(uint64_t *total, uint64_t *available, uint64_t *used);

/* Inter-Process Communication */
int neural_ipc_create_channel(const char *name);
int neural_ipc_connect_channel(const char *name);
int neural_ipc_send_message(int channel_id, const void *data, size_t size);
int neural_ipc_receive_message(int channel_id, void *buffer, size_t buffer_size);
void neural_ipc_close_channel(int channel_id);

/* Security Interface */
int neural_security_authenticate(const char *username, const char *password);
int neural_security_encrypt(const void *data, size_t size, void *output, const char *key);
int neural_security_decrypt(const void *data, size_t size, void *output, const char *key);
uint64_t neural_security_hash(const void *data, size_t size);

/* Cyberpunk UI Elements */
void neural_ui_draw_matrix_background(struct neural_graphics_context *gfx);
void neural_ui_draw_cyberpunk_border(struct neural_graphics_context *gfx, int x, int y, int width, int height);
void neural_ui_draw_neural_text(struct neural_graphics_context *gfx, int x, int y, const char *text);
void neural_ui_draw_holographic_button(struct neural_graphics_context *gfx, int x, int y, int width, int height, const char *label);
void neural_ui_draw_neon_line(struct neural_graphics_context *gfx, int x1, int y1, int x2, int y2, uint32_t color);

/* Audio Interface */
int neural_audio_init(void);
void neural_audio_cleanup(void);
int neural_audio_play_tone(uint32_t frequency, uint32_t duration);
int neural_audio_play_cyberpunk_sound(int sound_id);

/* Application Macros */
#define NEURAL_APP_DEFINE(name, desc, type, perms) \
    static struct neural_app_registration name##_registration = { \
        .name = #name, \
        .description = desc, \
        .type = type, \
        .required_permissions = perms, \
        .interface = { \
            .init = name##_init, \
            .run = name##_run, \
            .suspend = name##_suspend, \
            .resume = name##_resume, \
            .cleanup = name##_cleanup, \
            .signal_handler = name##_signal_handler \
        } \
    }

#define NEURAL_APP_REGISTER(name) \
    neural_app_register(&name##_registration)

/* Color Constants for Cyberpunk Theme */
#define NEURAL_COLOR_BLACK       0x00000000
#define NEURAL_COLOR_BLUE        0x000080FF
#define NEURAL_COLOR_CYAN        0x0000FFFF
#define NEURAL_COLOR_WHITE       0x00FFFFFF
#define NEURAL_COLOR_GREEN       0x0000FF00
#define NEURAL_COLOR_PURPLE      0x00800080
#define NEURAL_COLOR_MATRIX      0x0000AA00
#define NEURAL_COLOR_NEON_PINK   0x00FF00FF
#define NEURAL_COLOR_ELECTRIC    0x0000AAFF

#endif /* NEURAL_APP_H */