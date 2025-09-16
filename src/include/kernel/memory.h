/* memory.h - Brandon Media OS Memory Management System */
#ifndef _MEMORY_H
#define _MEMORY_H

#include <stdint.h>
#include <stddef.h>

/* Brandon Media OS - Memory Matrix Definitions */

/* Page sizes and alignment */
#define PAGE_SIZE           4096
#define PAGE_SHIFT          12
#define PAGE_MASK           (PAGE_SIZE - 1)
#define PAGES_PER_TABLE     512
#define TABLES_PER_DIR      512

/* Virtual memory layout - Cyberpunk memory architecture */
#define KERNEL_VIRTUAL_BASE     0xFFFF800000000000ULL  /* -128TB: Kernel space */
#define USER_VIRTUAL_MAX        0x00007FFFFFFFFFFFULL  /* +128TB: User space max */
#define HEAP_VIRTUAL_BASE       0xFFFF900000000000ULL  /* Kernel heap start */
#define HEAP_VIRTUAL_END        0xFFFFA00000000000ULL  /* Kernel heap end */

/* Physical memory constants */
#define PHYSICAL_MEMORY_MAX     0x100000000ULL         /* 4GB max for now */
#define KERNEL_PHYSICAL_START   0x100000               /* 1MB */
#define KERNEL_PHYSICAL_END     0x400000               /* 4MB */

/* Page table entry flags - Neural interface permissions */
#define PAGE_PRESENT            (1ULL << 0)    /* Matrix node active */
#define PAGE_WRITABLE           (1ULL << 1)    /* Write access granted */
#define PAGE_USER               (1ULL << 2)    /* User space accessible */
#define PAGE_WRITE_THROUGH      (1ULL << 3)    /* Write-through cache */
#define PAGE_CACHE_DISABLED     (1ULL << 4)    /* Cache disabled */
#define PAGE_ACCESSED           (1ULL << 5)    /* Page accessed flag */
#define PAGE_DIRTY              (1ULL << 6)    /* Page modified flag */
#define PAGE_SIZE_FLAG          (1ULL << 7)    /* Large page (2MB/1GB) */
#define PAGE_GLOBAL             (1ULL << 8)    /* Global page */
#define PAGE_NO_EXECUTE         (1ULL << 63)   /* NX bit - execution blocked */

/* Memory protection flags */
#define MEM_READ                0x01
#define MEM_WRITE               0x02
#define MEM_EXECUTE             0x04
#define MEM_USER                0x08
#define MEM_KERNEL              0x10

/* Page table structures for x86_64 */
typedef struct {
    uint64_t entries[512];
} __attribute__((packed, aligned(4096))) page_table_t;

typedef page_table_t pml4_t;     /* Page Map Level 4 */
typedef page_table_t pdpt_t;     /* Page Directory Pointer Table */
typedef page_table_t pd_t;       /* Page Directory */
typedef page_table_t pt_t;       /* Page Table */

/* Memory region descriptor */
struct memory_region {
    uint64_t base;
    uint64_t size;
    uint32_t type;
    uint32_t flags;
    struct memory_region *next;
};

/* Memory statistics - Matrix diagnostics */
struct memory_stats {
    uint64_t total_physical;      /* Total physical memory */
    uint64_t available_physical;  /* Available physical memory */
    uint64_t used_physical;       /* Used physical memory */
    uint64_t kernel_heap_used;    /* Kernel heap usage */
    uint64_t kernel_heap_free;    /* Kernel heap free */
    uint64_t page_faults;         /* Page fault count */
    uint64_t pages_allocated;     /* Total pages allocated */
    uint64_t pages_freed;         /* Total pages freed */
};

/* Brandon Media OS - Memory Management Functions */

/* Paging system */
void paging_init(void);
void paging_enable(void);
void paging_disable(void);
pml4_t *paging_create_address_space(void);
void paging_destroy_address_space(pml4_t *pml4);
void paging_switch_address_space(pml4_t *pml4);
int paging_map_page(pml4_t *pml4, uint64_t virtual_addr, uint64_t physical_addr, uint64_t flags);
int paging_unmap_page(pml4_t *pml4, uint64_t virtual_addr);
uint64_t paging_get_physical_address(pml4_t *pml4, uint64_t virtual_addr);

/* Physical memory management */
void pmm_init(struct memory_region *regions, size_t region_count);
uint64_t pmm_alloc_frame(void);
void pmm_free_frame(uint64_t frame);
uint64_t pmm_alloc_frames(size_t count);
void pmm_free_frames(uint64_t frame, size_t count);
size_t pmm_get_free_memory(void);
size_t pmm_get_used_memory(void);

/* Virtual memory management */
void vmm_init(void);
void *vmm_alloc(size_t size, uint64_t flags);
void vmm_free(void *ptr);
void *vmm_map(uint64_t physical_addr, size_t size, uint64_t flags);
void vmm_unmap(void *virtual_addr, size_t size);

/* Kernel heap */
void heap_init(void);
void *kmalloc(size_t size);
void *kcalloc(size_t count, size_t size);
void *krealloc(void *ptr, size_t size);
void kfree(void *ptr);

/* Memory utilities */
void memory_set(void *dst, int value, size_t size);
void memory_copy(void *dst, const void *src, size_t size);
int memory_compare(const void *a, const void *b, size_t size);
struct memory_stats *memory_get_stats(void);

/* Page frame conversion macros */
#define FRAME_TO_ADDR(frame)    ((frame) * PAGE_SIZE)
#define ADDR_TO_FRAME(addr)     ((addr) / PAGE_SIZE)
#define PAGE_ALIGN_UP(addr)     (((addr) + PAGE_MASK) & ~PAGE_MASK)
#define PAGE_ALIGN_DOWN(addr)   ((addr) & ~PAGE_MASK)
#define IS_PAGE_ALIGNED(addr)   (((addr) & PAGE_MASK) == 0)

/* Virtual address breakdown macros for x86_64 */
#define PML4_INDEX(addr)        (((addr) >> 39) & 0x1FF)
#define PDPT_INDEX(addr)        (((addr) >> 30) & 0x1FF)
#define PD_INDEX(addr)          (((addr) >> 21) & 0x1FF)
#define PT_INDEX(addr)          (((addr) >> 12) & 0x1FF)
#define PAGE_OFFSET(addr)       ((addr) & 0xFFF)

#endif /* _MEMORY_H */