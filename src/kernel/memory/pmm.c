/* pmm.c - Brandon Media OS Physical Memory Manager */
#include <stdint.h>
#include "kernel/memory.h"

/* External functions */
extern void serial_puts(const char *s);
extern void print_hex(uint64_t num);

/* Physical memory bitmap */
static uint64_t *frame_bitmap = NULL;
static uint64_t total_frames = 0;
static uint64_t used_frames = 0;
static uint64_t bitmap_size = 0;

/* Memory regions */
static struct memory_region *memory_regions = NULL;
static size_t region_count = 0;

/* Bitmap manipulation macros */
#define BITMAP_SET(bitmap, bit)     ((bitmap)[(bit) / 64] |= (1ULL << ((bit) % 64)))
#define BITMAP_CLEAR(bitmap, bit)   ((bitmap)[(bit) / 64] &= ~(1ULL << ((bit) % 64)))
#define BITMAP_TEST(bitmap, bit)    ((bitmap)[(bit) / 64] & (1ULL << ((bit) % 64)))

/* Initialize physical memory manager */
void pmm_init(struct memory_region *regions, size_t region_cnt) {
    serial_puts("[MATRIX] Initializing physical memory nexus...\n");
    
    memory_regions = regions;
    region_count = region_cnt;
    
    /* Calculate total physical memory */
    uint64_t max_address = 0;
    for (size_t i = 0; i < region_count; i++) {
        uint64_t region_end = regions[i].base + regions[i].size;
        if (region_end > max_address) {
            max_address = region_end;
        }
        serial_puts("[NEXUS] Memory region detected: ");
        print_hex(regions[i].base);
        serial_puts(" - ");
        print_hex(region_end);
        serial_puts("\n");
    }
    
    /* Calculate frame count and bitmap size */
    total_frames = max_address / PAGE_SIZE;
    bitmap_size = (total_frames + 63) / 64;  /* Round up to 64-bit words */
    
    serial_puts("[MATRIX] Total memory frames: ");
    print_hex(total_frames);
    serial_puts("\n");
    
    /* Find suitable location for bitmap (after kernel) */
    uint64_t bitmap_location = KERNEL_PHYSICAL_END;
    bitmap_location = PAGE_ALIGN_UP(bitmap_location);
    frame_bitmap = (uint64_t *)bitmap_location;
    
    serial_puts("[NEXUS] Memory bitmap nexus at: ");
    print_hex(bitmap_location);
    serial_puts("\n");
    
    /* Initialize bitmap - mark all frames as used initially */
    memory_set(frame_bitmap, 0xFF, bitmap_size * sizeof(uint64_t));\n    \n    /* Mark available regions as free */\n    for (size_t i = 0; i < region_count; i++) {\n        if (regions[i].type == 1) {  /* Available memory */\n            uint64_t start_frame = regions[i].base / PAGE_SIZE;\n            uint64_t end_frame = (regions[i].base + regions[i].size) / PAGE_SIZE;\n            \n            for (uint64_t frame = start_frame; frame < end_frame; frame++) {\n                if (frame < total_frames) {\n                    BITMAP_CLEAR(frame_bitmap, frame);\n                }\n            }\n        }\n    }\n    \n    /* Mark kernel and bitmap area as used */\n    uint64_t kernel_start_frame = KERNEL_PHYSICAL_START / PAGE_SIZE;\n    uint64_t bitmap_end = bitmap_location + (bitmap_size * sizeof(uint64_t));\n    uint64_t bitmap_end_frame = PAGE_ALIGN_UP(bitmap_end) / PAGE_SIZE;\n    \n    for (uint64_t frame = 0; frame < bitmap_end_frame; frame++) {\n        if (frame < total_frames) {\n            BITMAP_SET(frame_bitmap, frame);\n            used_frames++;\n        }\n    }\n    \n    serial_puts(\"[MATRIX] Physical memory nexus initialized\\n\");\n    serial_puts(\"[STATS] Available frames: \");\n    print_hex(total_frames - used_frames);\n    serial_puts(\"\\n\");\n}\n\n/* Allocate a single physical frame */\nuint64_t pmm_alloc_frame(void) {\n    for (uint64_t frame = 0; frame < total_frames; frame++) {\n        if (!BITMAP_TEST(frame_bitmap, frame)) {\n            BITMAP_SET(frame_bitmap, frame);\n            used_frames++;\n            return FRAME_TO_ADDR(frame);\n        }\n    }\n    \n    serial_puts(\"[ERROR] Memory nexus exhausted - no free frames\\n\");\n    return 0;  /* Out of memory */\n}\n\n/* Free a physical frame */\nvoid pmm_free_frame(uint64_t frame_addr) {\n    uint64_t frame = ADDR_TO_FRAME(frame_addr);\n    \n    if (frame >= total_frames) {\n        serial_puts(\"[ERROR] Invalid frame address - memory corruption detected\\n\");\n        return;\n    }\n    \n    if (!BITMAP_TEST(frame_bitmap, frame)) {\n        serial_puts(\"[WARNING] Double free detected - frame already available\\n\");\n        return;\n    }\n    \n    BITMAP_CLEAR(frame_bitmap, frame);\n    used_frames--;\n}\n\n/* Allocate multiple contiguous frames */\nuint64_t pmm_alloc_frames(size_t count) {\n    if (count == 0) return 0;\n    if (count == 1) return pmm_alloc_frame();\n    \n    /* Find contiguous free frames */\n    for (uint64_t start_frame = 0; start_frame <= total_frames - count; start_frame++) {\n        bool found = true;\n        \n        /* Check if all frames in range are free */\n        for (size_t i = 0; i < count; i++) {\n            if (BITMAP_TEST(frame_bitmap, start_frame + i)) {\n                found = false;\n                break;\n            }\n        }\n        \n        if (found) {\n            /* Mark all frames as used */\n            for (size_t i = 0; i < count; i++) {\n                BITMAP_SET(frame_bitmap, start_frame + i);\n            }\n            used_frames += count;\n            return FRAME_TO_ADDR(start_frame);\n        }\n    }\n    \n    serial_puts(\"[ERROR] No contiguous memory block available\\n\");\n    return 0;\n}\n\n/* Free multiple contiguous frames */\nvoid pmm_free_frames(uint64_t frame_addr, size_t count) {\n    uint64_t start_frame = ADDR_TO_FRAME(frame_addr);\n    \n    for (size_t i = 0; i < count; i++) {\n        pmm_free_frame(FRAME_TO_ADDR(start_frame + i));\n    }\n}\n\n/* Get free memory in bytes */\nsize_t pmm_get_free_memory(void) {\n    return (total_frames - used_frames) * PAGE_SIZE;\n}\n\n/* Get used memory in bytes */\nsize_t pmm_get_used_memory(void) {\n    return used_frames * PAGE_SIZE;\n}