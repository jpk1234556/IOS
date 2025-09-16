/* vmm.c - Brandon Media OS Virtual Memory Manager */
#include <stdint.h>
#include "kernel/memory.h"

/* External functions */
extern void serial_puts(const char *s);
extern void print_hex(uint64_t num);

/* Virtual memory allocator state */
static uint64_t next_virtual_addr = HEAP_VIRTUAL_BASE;
static uint64_t heap_end = HEAP_VIRTUAL_BASE;

/* Initialize virtual memory manager */
void vmm_init(void) {
    serial_puts("[MATRIX] Initializing virtual memory nexus...\n");
    serial_puts("[VMM] Virtual reality space: ");
    print_hex(HEAP_VIRTUAL_BASE);
    serial_puts(" - ");
    print_hex(HEAP_VIRTUAL_END);
    serial_puts("\n");
    
    next_virtual_addr = HEAP_VIRTUAL_BASE;
    heap_end = HEAP_VIRTUAL_BASE;
    
    serial_puts("[MATRIX] Virtual memory nexus online\n");
}

/* Allocate virtual memory */
void *vmm_alloc(size_t size, uint64_t flags) {
    if (size == 0) return NULL;
    
    /* Align size to page boundary */
    size = PAGE_ALIGN_UP(size);
    
    /* Check if we have enough virtual space */
    if (next_virtual_addr + size > HEAP_VIRTUAL_END) {
        serial_puts("[ERROR] Virtual memory space exhausted - matrix overflow\n");
        return NULL;
    }
    
    uint64_t start_addr = next_virtual_addr;\n    \n    /* Allocate and map physical frames */\n    for (uint64_t addr = start_addr; addr < start_addr + size; addr += PAGE_SIZE) {\n        uint64_t physical_frame = pmm_alloc_frame();\n        if (!physical_frame) {\n            /* Cleanup on failure */\n            for (uint64_t cleanup_addr = start_addr; cleanup_addr < addr; cleanup_addr += PAGE_SIZE) {\n                uint64_t phys = paging_get_physical_address(current_pml4, cleanup_addr);\n                if (phys) {\n                    paging_unmap_page(current_pml4, cleanup_addr);\n                    pmm_free_frame(phys);\n                }\n            }\n            serial_puts(\"[ERROR] Physical memory allocation failed\\n\");\n            return NULL;\n        }\n        \n        if (paging_map_page(current_pml4, addr, physical_frame, flags) != 0) {\n            pmm_free_frame(physical_frame);\n            /* Cleanup on failure */\n            for (uint64_t cleanup_addr = start_addr; cleanup_addr < addr; cleanup_addr += PAGE_SIZE) {\n                uint64_t phys = paging_get_physical_address(current_pml4, cleanup_addr);\n                if (phys) {\n                    paging_unmap_page(current_pml4, cleanup_addr);\n                    pmm_free_frame(phys);\n                }\n            }\n            serial_puts(\"[ERROR] Virtual memory mapping failed\\n\");\n            return NULL;\n        }\n    }\n    \n    next_virtual_addr += size;\n    if (next_virtual_addr > heap_end) {\n        heap_end = next_virtual_addr;\n    }\n    \n    return (void *)start_addr;\n}\n\n/* Free virtual memory */\nvoid vmm_free(void *ptr) {\n    if (!ptr) return;\n    \n    /* Note: This is a simplified implementation */\n    /* In a full implementation, we'd track allocations and sizes */\n    serial_puts(\"[VMM] Memory deallocation requested\\n\");\n}\n\n/* Map physical memory to virtual address */\nvoid *vmm_map(uint64_t physical_addr, size_t size, uint64_t flags) {\n    if (size == 0) return NULL;\n    \n    size = PAGE_ALIGN_UP(size);\n    physical_addr = PAGE_ALIGN_DOWN(physical_addr);\n    \n    if (next_virtual_addr + size > HEAP_VIRTUAL_END) {\n        serial_puts(\"[ERROR] Virtual address space exhausted\\n\");\n        return NULL;\n    }\n    \n    uint64_t virtual_start = next_virtual_addr;\n    \n    for (uint64_t offset = 0; offset < size; offset += PAGE_SIZE) {\n        if (paging_map_page(current_pml4, virtual_start + offset, \n                           physical_addr + offset, flags) != 0) {\n            /* Cleanup on failure */\n            for (uint64_t cleanup_offset = 0; cleanup_offset < offset; cleanup_offset += PAGE_SIZE) {\n                paging_unmap_page(current_pml4, virtual_start + cleanup_offset);\n            }\n            serial_puts(\"[ERROR] Physical-to-virtual mapping failed\\n\");\n            return NULL;\n        }\n    }\n    \n    next_virtual_addr += size;\n    return (void *)virtual_start;\n}\n\n/* Unmap virtual memory */\nvoid vmm_unmap(void *virtual_addr, size_t size) {\n    if (!virtual_addr || size == 0) return;\n    \n    uint64_t start_addr = PAGE_ALIGN_DOWN((uint64_t)virtual_addr);\n    size = PAGE_ALIGN_UP(size);\n    \n    for (uint64_t addr = start_addr; addr < start_addr + size; addr += PAGE_SIZE) {\n        paging_unmap_page(current_pml4, addr);\n    }\n}