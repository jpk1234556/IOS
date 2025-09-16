/* framebuffer.c - Brandon Media OS Framebuffer Graphics Driver
 * Neural Display Interface Controller
 */

#include <stdint.h>
#include <stddef.h>
#include "kernel/memory.h"
#include "kernel/pci.h"
#include "kernel/hal.h"

/* VGA/VESA Constants */
#define VGA_TEXT_BUFFER     0xB8000
#define VGA_WIDTH           80
#define VGA_HEIGHT          25

/* Framebuffer Device Structure */
struct framebuffer_device {
    struct hal_device *hal_dev;
    struct pci_device *pci_dev;
    uint32_t *framebuffer;
    uint32_t width;
    uint32_t height;
    uint32_t pitch;
    uint32_t bpp;          /* Bits per pixel */
    uint32_t red_mask;
    uint32_t green_mask;
    uint32_t blue_mask;
    int initialized;
};

static struct framebuffer_device *fb_dev = NULL;

/* External functions */
extern void serial_puts(const char *s);
extern void print_hex(uint64_t num);
extern void print_dec(uint64_t num);

/* Color definitions for cyberpunk theme */
#define NEURAL_BLACK      0x00000000
#define NEURAL_BLUE       0x000080FF
#define NEURAL_CYAN       0x0000FFFF
#define NEURAL_WHITE      0x00FFFFFF
#define NEURAL_GREEN      0x0000FF00
#define NEURAL_PURPLE     0x00800080
#define NEURAL_MATRIX     0x0000AA00

/* Basic VGA text mode operations */
static void vga_clear_screen(void) {
    volatile uint16_t *vga_buffer = (volatile uint16_t *)VGA_TEXT_BUFFER;
    const uint16_t blank = 0x0F20;  /* White on black space */
    
    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        vga_buffer[i] = blank;
    }
}

static void vga_put_char(int x, int y, char c, uint8_t color) {
    if (x >= VGA_WIDTH || y >= VGA_HEIGHT) return;
    
    volatile uint16_t *vga_buffer = (volatile uint16_t *)VGA_TEXT_BUFFER;
    int index = y * VGA_WIDTH + x;
    vga_buffer[index] = ((uint16_t)color << 8) | (uint8_t)c;
}

static void vga_put_string(int x, int y, const char *str, uint8_t color) {
    int pos_x = x;
    int pos_y = y;
    
    for (int i = 0; str[i] != '\0'; i++) {
        if (str[i] == '\n') {
            pos_y++;
            pos_x = x;
            if (pos_y >= VGA_HEIGHT) break;
            continue;
        }
        
        if (pos_x >= VGA_WIDTH) {
            pos_y++;
            pos_x = x;
            if (pos_y >= VGA_HEIGHT) break;
        }
        
        vga_put_char(pos_x, pos_y, str[i], color);
        pos_x++;
    }
}

/* Framebuffer operations */
static void fb_put_pixel(uint32_t x, uint32_t y, uint32_t color) {
    if (!fb_dev || !fb_dev->framebuffer || !fb_dev->initialized) return;
    if (x >= fb_dev->width || y >= fb_dev->height) return;
    
    uint32_t *pixel = fb_dev->framebuffer + (y * fb_dev->width + x);
    *pixel = color;
}

static void fb_fill_rect(uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t color) {
    for (uint32_t row = y; row < y + height && row < fb_dev->height; row++) {
        for (uint32_t col = x; col < x + width && col < fb_dev->width; col++) {
            fb_put_pixel(col, row, color);
        }
    }
}

static void fb_clear_screen(uint32_t color) {
    if (!fb_dev || !fb_dev->framebuffer || !fb_dev->initialized) return;
    
    fb_fill_rect(0, 0, fb_dev->width, fb_dev->height, color);
}

/* Draw cyberpunk-style neural pattern */
static void fb_draw_neural_pattern(void) {
    if (!fb_dev || !fb_dev->initialized) return;
    
    /* Draw grid pattern */
    for (uint32_t x = 0; x < fb_dev->width; x += 20) {
        for (uint32_t y = 0; y < fb_dev->height; y++) {
            fb_put_pixel(x, y, NEURAL_BLUE);
        }
    }
    
    for (uint32_t y = 0; y < fb_dev->height; y += 20) {
        for (uint32_t x = 0; x < fb_dev->width; x++) {
            fb_put_pixel(x, y, NEURAL_BLUE);
        }
    }
    
    /* Draw some neural connection lines */
    for (uint32_t i = 0; i < fb_dev->width; i += 5) {
        uint32_t wave_y = fb_dev->height / 2 + (i % 40) - 20;
        if (wave_y < fb_dev->height) {
            fb_put_pixel(i, wave_y, NEURAL_CYAN);
        }
    }
}

/* Initialize basic framebuffer (VGA text mode for now) */
static int fb_init_device(struct hal_device *hal_dev) {
    serial_puts("[NEURAL-GFX] Initializing neural display interface...\n");
    
    /* Allocate device structure */
    fb_dev = (struct framebuffer_device *)kmalloc(sizeof(struct framebuffer_device));
    if (!fb_dev) {
        serial_puts("[NEURAL-GFX] Failed to allocate device structure\n");
        return -1;
    }
    
    fb_dev->hal_dev = hal_dev;
    fb_dev->pci_dev = hal_dev->pci_dev;
    
    /* For now, we'll use VGA text mode as our "framebuffer" */
    fb_dev->framebuffer = (uint32_t *)VGA_TEXT_BUFFER;
    fb_dev->width = VGA_WIDTH;
    fb_dev->height = VGA_HEIGHT;
    fb_dev->pitch = VGA_WIDTH * 2;
    fb_dev->bpp = 16;
    fb_dev->initialized = 1;
    
    hal_dev->device_data = fb_dev;
    
    /* Clear screen with cyberpunk theme */
    vga_clear_screen();
    
    /* Display neural interface boot message */
    vga_put_string(10, 5, "NEURAL DISPLAY INTERFACE ONLINE", 0x09);  /* Blue text */
    vga_put_string(15, 7, "Brandon Media OS v0.1", 0x0F);            /* White text */
    vga_put_string(12, 9, "Cyberpunk Graphics Mode Active", 0x0B);   /* Cyan text */
    
    /* Draw some neural patterns */
    for (int y = 15; y < 20; y++) {
        for (int x = 10; x < 70; x++) {
            if ((x + y) % 3 == 0) {
                vga_put_char(x, y, '.', 0x02);  /* Green dots */
            }
        }
    }
    
    serial_puts("[NEURAL-GFX] Neural display interface initialized\n");
    return 0;
}

/* Start framebuffer device */
static int fb_start_device(struct hal_device *hal_dev) {
    if (!fb_dev || !fb_dev->initialized) {
        return -1;
    }
    
    serial_puts("[NEURAL-GFX] Starting neural display interface...\n");
    
    /* Refresh display with neural patterns */
    vga_put_string(10, 22, "NEURAL MATRIX ACTIVE - GRAPHICS ONLINE", 0x0A);
    
    serial_puts("[NEURAL-GFX] Neural display interface started\n");
    return 0;
}

/* Stop framebuffer device */
static int fb_stop_device(struct hal_device *hal_dev) {
    if (!fb_dev) {
        return -1;
    }
    
    serial_puts("[NEURAL-GFX] Stopping neural display interface...\n");
    
    /* Clear screen */
    vga_clear_screen();
    vga_put_string(25, 12, "NEURAL DISPLAY OFFLINE", 0x04);  /* Red text */
    
    serial_puts("[NEURAL-GFX] Neural display interface stopped\n");
    return 0;
}

/* Reset framebuffer device */
static int fb_reset_device(struct hal_device *hal_dev) {
    serial_puts("[NEURAL-GFX] Resetting neural display interface...\n");
    
    if (fb_stop_device(hal_dev) != 0) {
        return -1;
    }
    
    return fb_init_device(hal_dev);
}

/* Cleanup framebuffer device */
static void fb_cleanup_device(struct hal_device *hal_dev) {
    if (!fb_dev) {
        return;
    }
    
    serial_puts("[NEURAL-GFX] Cleaning up neural display interface...\n");
    
    /* Clear screen */
    vga_clear_screen();
    
    /* Free device structure */
    kfree(fb_dev);
    fb_dev = NULL;
    
    serial_puts("[NEURAL-GFX] Neural display interface cleanup complete\n");
}

/* Print graphics information */
void fb_print_info(void) {
    if (!fb_dev) {
        serial_puts("[NEURAL-GFX] No neural display interface available\n");
        return;
    }
    
    serial_puts("[NEURAL-GFX] === Display Interface Information ===\n");
    serial_puts("[INFO] Resolution: ");
    print_dec(fb_dev->width);
    serial_puts("x");
    print_dec(fb_dev->height);
    serial_puts("\n");
    
    serial_puts("[INFO] Bits per pixel: ");
    print_dec(fb_dev->bpp);
    serial_puts("\n");
    
    serial_puts("[INFO] Framebuffer address: ");
    print_hex((uint64_t)fb_dev->framebuffer);
    serial_puts("\n");
    
    serial_puts("[NEURAL-GFX] === End Display Information ===\n");
}

/* Initialize framebuffer driver */
void framebuffer_init(void) {
    serial_puts("[NEURAL-GFX] Initializing neural display driver...\n");
    
    /* Find VGA/Graphics device */
    struct pci_device *gfx_dev = pci_find_device_by_class(PCI_CLASS_DISPLAY, PCI_SUBCLASS_VGA);
    
    /* Create HAL device for graphics */
    struct hal_device *hal_dev = hal_create_device(DEVICE_TYPE_DISPLAY, 
                                                   "Neural Display Interface", 
                                                   "Neural Graphics Corporation");
    if (!hal_dev) {
        serial_puts("[NEURAL-GFX] Failed to create HAL device\n");
        return;
    }
    
    hal_dev->pci_dev = gfx_dev;  /* May be NULL for VGA */
    hal_dev->init = fb_init_device;
    hal_dev->start = fb_start_device;
    hal_dev->stop = fb_stop_device;
    hal_dev->reset = fb_reset_device;
    hal_dev->cleanup = fb_cleanup_device;
    
    /* Register device with HAL */
    if (hal_register_device(hal_dev) != 0) {
        serial_puts("[NEURAL-GFX] Failed to register HAL device\n");
        kfree(hal_dev);
        return;
    }
    
    serial_puts("[NEURAL-GFX] Neural display driver initialized\n");
}

/* Test graphics functions */
void fb_test_graphics(void) {
    if (!fb_dev || !fb_dev->initialized) {
        serial_puts("[NEURAL-GFX] Graphics interface not available for testing\n");
        return;
    }
    
    serial_puts("[NEURAL-GFX] Testing neural graphics interface...\n");
    
    /* Draw cyberpunk test pattern */
    vga_put_string(5, 2, "=== NEURAL GRAPHICS TEST ===", 0x0F);
    vga_put_string(5, 4, "Cyberpunk Theme Active", 0x09);
    vga_put_string(5, 5, "Matrix Interface Online", 0x0A);
    vga_put_string(5, 6, "Neural Patterns Loading...", 0x0B);
    
    /* Create a simple progress bar effect */
    for (int i = 0; i < 50; i++) {
        vga_put_char(10 + i, 8, '=', 0x02);
        /* Small delay simulation */
        for (volatile int j = 0; j < 100000; j++);
    }
    
    vga_put_string(10, 10, "Neural Graphics Test Complete", 0x0E);
    
    serial_puts("[NEURAL-GFX] Graphics test completed\n");
}

/* Get framebuffer device */
struct framebuffer_device *framebuffer_get_device(void) {
    return fb_dev;
}