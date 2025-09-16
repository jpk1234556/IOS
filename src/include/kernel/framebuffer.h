/* framebuffer.h - Brandon Media OS Enhanced Neural Display Interface
 * GPU-Accelerated Graphics with 3D Parallax Support
 */

#ifndef KERNEL_FRAMEBUFFER_H
#define KERNEL_FRAMEBUFFER_H

#include <stdint.h>
#include <stdbool.h>
#include "kernel/hal.h"

/* GPU Types */
typedef enum {
    GPU_TYPE_SOFTWARE,     /* Software rendering only */
    GPU_TYPE_VGA,         /* Basic VGA compatibility */
    GPU_TYPE_VESA,        /* VESA BIOS Extensions */
    GPU_TYPE_INTEL,       /* Intel integrated graphics */
    GPU_TYPE_NVIDIA,      /* NVIDIA discrete graphics */
    GPU_TYPE_AMD,         /* AMD discrete graphics */
    GPU_TYPE_VIRTIO       /* VirtIO GPU (virtualized) */
} gpu_type_t;

/* Display Modes */
typedef struct {
    uint32_t width;
    uint32_t height;
    uint32_t bpp;         /* Bits per pixel */
    uint32_t refresh_rate;
    bool interlaced;
    char name[32];
} display_mode_t;

/* GPU Capabilities */
typedef struct {
    bool hardware_acceleration;
    bool double_buffering;
    bool triple_buffering;
    bool vsync_support;
    bool alpha_blending;
    bool texture_mapping;
    bool depth_buffering;
    uint32_t max_texture_size;
    uint32_t vram_size;
    uint32_t max_resolution_x;
    uint32_t max_resolution_y;
} gpu_capabilities_t;

/* Enhanced Framebuffer Device Structure */
typedef struct framebuffer_device {
    struct hal_device *hal_dev;
    struct pci_device *pci_dev;
    
    /* Display Properties */
    uint32_t *framebuffer;        /* Primary framebuffer */
    uint32_t *back_buffer;        /* Back buffer for double buffering */
    uint32_t *depth_buffer;       /* Z-buffer for 3D rendering */
    uint32_t width;
    uint32_t height;
    uint32_t pitch;               /* Bytes per scanline */
    uint32_t bpp;                /* Bits per pixel */
    uint32_t bytes_per_pixel;
    
    /* Color Masks */
    uint32_t red_mask;
    uint32_t green_mask;
    uint32_t blue_mask;
    uint32_t alpha_mask;
    uint8_t red_shift;
    uint8_t green_shift;
    uint8_t blue_shift;
    uint8_t alpha_shift;
    
    /* GPU Information */
    gpu_type_t gpu_type;
    gpu_capabilities_t capabilities;
    char gpu_name[64];
    uint32_t device_id;
    uint32_t vendor_id;
    
    /* Display Modes */
    display_mode_t *modes;
    uint32_t mode_count;
    uint32_t current_mode;
    
    /* Performance Counters */
    uint64_t frames_rendered;
    uint32_t last_frame_time;
    uint32_t fps;
    
    /* State */
    bool initialized;
    bool double_buffering_enabled;
    bool vsync_enabled;
    bool gpu_acceleration_enabled;
} framebuffer_device_t;

/* GPU Command Types */
typedef enum {
    GPU_CMD_CLEAR,
    GPU_CMD_DRAW_RECT,
    GPU_CMD_DRAW_LINE,
    GPU_CMD_DRAW_TRIANGLE,
    GPU_CMD_BLIT,
    GPU_CMD_COPY_BUFFER,
    GPU_CMD_SET_BLEND_MODE
} gpu_command_type_t;

/* GPU Command Structure */
typedef struct {
    gpu_command_type_t type;
    union {
        struct { uint32_t color; } clear;
        struct { int32_t x, y, width, height; uint32_t color; } rect;
        struct { int32_t x1, y1, x2, y2; uint32_t color; } line;
        struct { int32_t x1, y1, x2, y2, x3, y3; uint32_t color; } triangle;
        struct { uint32_t *src; int32_t sx, sy, dx, dy, width, height; } blit;
    } data;
} gpu_command_t;

/* Framebuffer Function Prototypes */
int framebuffer_init(void);
void framebuffer_shutdown(void);
void fb_print_info(void);
void fb_test_graphics(void);
framebuffer_device_t *framebuffer_get_device(void);

/* Display Mode Management */
int fb_enumerate_modes(void);
int fb_set_mode(uint32_t width, uint32_t height, uint32_t bpp);
int fb_get_current_mode(display_mode_t *mode);
void fb_list_modes(void);

/* Enhanced Graphics Primitives */
void fb_put_pixel(uint32_t x, uint32_t y, uint32_t color);
void fb_put_pixel_alpha(uint32_t x, uint32_t y, uint32_t color, uint8_t alpha);
void fb_fill_rect(uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t color);
void fb_draw_rect(uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t color, uint32_t thickness);
void fb_draw_line(int32_t x1, int32_t y1, int32_t x2, int32_t y2, uint32_t color);
void fb_draw_circle(int32_t cx, int32_t cy, uint32_t radius, uint32_t color);
void fb_fill_circle(int32_t cx, int32_t cy, uint32_t radius, uint32_t color);
void fb_draw_triangle(int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t x3, int32_t y3, uint32_t color);
void fb_fill_triangle(int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t x3, int32_t y3, uint32_t color);
void fb_clear_screen(uint32_t color);
void fb_clear_depth_buffer(float depth);

/* Buffer Management */
void fb_swap_buffers(void);
void fb_enable_double_buffering(bool enable);
void fb_enable_vsync(bool enable);
void fb_copy_buffer(uint32_t *src, uint32_t *dst, uint32_t width, uint32_t height);

/* Blitting and Texture Operations */
void fb_blit(uint32_t *src, int32_t sx, int32_t sy, int32_t dx, int32_t dy, uint32_t width, uint32_t height);
void fb_blit_scaled(uint32_t *src, int32_t sx, int32_t sy, int32_t sw, int32_t sh, 
                   int32_t dx, int32_t dy, int32_t dw, int32_t dh);
void fb_blit_alpha(uint32_t *src, int32_t sx, int32_t sy, int32_t dx, int32_t dy, 
                  uint32_t width, uint32_t height, uint8_t alpha);

/* 3D Support */
void fb_set_depth_test(bool enable);
bool fb_depth_test(uint32_t x, uint32_t y, float z);
void fb_write_depth(uint32_t x, uint32_t y, float z);

/* GPU Acceleration */
bool fb_is_gpu_accelerated(void);
void fb_enable_gpu_acceleration(bool enable);
int fb_submit_gpu_command(gpu_command_t *command);
void fb_flush_gpu_commands(void);

/* Neural Graphics Effects */
void fb_neural_matrix_effect(uint32_t time_ms);
void fb_neural_grid_pattern(uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t time_ms);
void fb_neural_pulse_effect(uint32_t cx, uint32_t cy, uint32_t radius, uint32_t time_ms);
void fb_cyberpunk_scanlines(uint32_t intensity);
void fb_holographic_overlay(uint32_t time_ms);

/* Text Rendering (Basic) */
void fb_draw_char(int32_t x, int32_t y, char c, uint32_t color, uint32_t bg_color);
void fb_draw_string(int32_t x, int32_t y, const char *str, uint32_t color, uint32_t bg_color);
void fb_set_font_scale(uint32_t scale);

/* Color Utilities */
uint32_t fb_color_from_rgb(uint8_t r, uint8_t g, uint8_t b);
uint32_t fb_color_from_rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a);
void fb_color_to_rgb(uint32_t color, uint8_t *r, uint8_t *g, uint8_t *b);
uint32_t fb_color_blend(uint32_t src, uint32_t dst, uint8_t alpha);
uint32_t fb_color_multiply(uint32_t color, float factor);

/* Performance Monitoring */
void fb_get_performance_stats(uint64_t *frames, uint32_t *fps, uint32_t *frame_time);
void fb_reset_performance_stats(void);

/* Hardware Detection */
int fb_detect_gpu(void);
const char *fb_get_gpu_name(void);
void fb_get_gpu_capabilities(gpu_capabilities_t *caps);

/* VESA BIOS Extensions */
int vesa_init(void);
int vesa_set_mode(uint32_t mode);
int vesa_get_mode_info(uint32_t mode, display_mode_t *info);

/* VirtIO GPU Driver */
int virtio_gpu_init(void);
void virtio_gpu_cleanup(void);

/* Color constants (Enhanced Neural Theme) */
#define NEURAL_BLACK        0x00000000
#define NEURAL_BLUE         0xFF0080FF
#define NEURAL_CYAN         0xFF00FFFF
#define NEURAL_WHITE        0xFFFFFFFF
#define NEURAL_GREEN        0xFF00FF00
#define NEURAL_RED          0xFFFF0000
#define NEURAL_PURPLE       0xFF800080
#define NEURAL_MATRIX       0xFF00AA00
#define NEURAL_DARK_BLUE    0xFF002040
#define NEURAL_LIGHT_BLUE   0xFF4080FF
#define NEURAL_GOLD         0xFFFFD700
#define NEURAL_SILVER       0xFFC0C0C0
#define NEURAL_TRANSPARENT  0x00000000

/* Alpha Blending Modes */
#define BLEND_MODE_NORMAL   0
#define BLEND_MODE_ADD      1
#define BLEND_MODE_MULTIPLY 2
#define BLEND_MODE_SCREEN   3

#endif /* KERNEL_FRAMEBUFFER_H */