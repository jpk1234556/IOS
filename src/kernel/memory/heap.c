/* heap.c - Brandon Media OS Kernel Heap Manager */
#include <stdint.h>
#include "kernel/memory.h"

/* External functions */
extern void serial_puts(const char *s);
extern void print_hex(uint64_t num);

/* Simple heap block header */
struct heap_block {
    size_t size;
    int is_free;
    struct heap_block *next;
    struct heap_block *prev;
} __attribute__((packed));

/* Heap state */
static struct heap_block *heap_start = NULL;
static struct heap_block *heap_end = NULL;
static size_t total_heap_size = 0;
static size_t used_heap_size = 0;

#define HEAP_BLOCK_SIZE sizeof(struct heap_block)
#define MIN_BLOCK_SIZE  32

/* Initialize kernel heap */
void heap_init(void) {
    serial_puts("[MATRIX] Initializing kernel memory heap nexus...\n");
    
    /* Allocate initial heap space (1MB) */
    size_t initial_heap_size = 1024 * 1024;
    heap_start = (struct heap_block *)vmm_alloc(initial_heap_size, 
                                               PAGE_PRESENT | PAGE_WRITABLE);
    
    if (!heap_start) {
        serial_puts("[FATAL] Heap nexus initialization failed - matrix unstable\n");
        return;
    }
    
    /* Initialize first block */
    heap_start->size = initial_heap_size - HEAP_BLOCK_SIZE;
    heap_start->is_free = 1;
    heap_start->next = NULL;
    heap_start->prev = NULL;
    
    heap_end = heap_start;
    total_heap_size = initial_heap_size;
    used_heap_size = HEAP_BLOCK_SIZE;
    
    serial_puts("[NEXUS] Kernel heap matrix initialized: ");
    print_hex(initial_heap_size);
    serial_puts(" bytes\n");
}

/* Find free block of sufficient size */
static struct heap_block *find_free_block(size_t size) {
    struct heap_block *current = heap_start;
    
    while (current) {
        if (current->is_free && current->size >= size) {
            return current;
        }
        current = current->next;
    }
    
    return NULL;
}

/* Split block if it's too large */
static void split_block(struct heap_block *block, size_t size) {
    if (block->size > size + HEAP_BLOCK_SIZE + MIN_BLOCK_SIZE) {
        struct heap_block *new_block = (struct heap_block *)((uint8_t *)block + HEAP_BLOCK_SIZE + size);
        
        new_block->size = block->size - size - HEAP_BLOCK_SIZE;
        new_block->is_free = 1;
        new_block->next = block->next;
        new_block->prev = block;
        
        if (block->next) {
            block->next->prev = new_block;
        } else {
            heap_end = new_block;
        }
        
        block->next = new_block;
        block->size = size;
    }
}

/* Merge adjacent free blocks */
static void merge_free_blocks(struct heap_block *block) {
    /* Merge with next block */
    if (block->next && block->next->is_free) {
        block->size += block->next->size + HEAP_BLOCK_SIZE;
        
        if (block->next->next) {
            block->next->next->prev = block;
        } else {
            heap_end = block;
        }
        
        block->next = block->next->next;
    }
    
    /* Merge with previous block */
    if (block->prev && block->prev->is_free) {
        block->prev->size += block->size + HEAP_BLOCK_SIZE;
        
        if (block->next) {
            block->next->prev = block->prev;
        } else {
            heap_end = block->prev;
        }
        
        block->prev->next = block->next;
    }
}

/* Allocate memory from kernel heap */
void *kmalloc(size_t size) {
    if (size == 0) return NULL;
    
    /* Align size to 8-byte boundary */
    size = (size + 7) & ~7;
    
    struct heap_block *block = find_free_block(size);
    
    if (!block) {
        serial_puts("[ERROR] Heap nexus exhausted - memory matrix overflow\n");
        return NULL;
    }
    
    /* Split block if necessary */
    split_block(block, size);
    
    block->is_free = 0;
    used_heap_size += size + HEAP_BLOCK_SIZE;
    
    return (void *)((uint8_t *)block + HEAP_BLOCK_SIZE);
}

/* Allocate and zero memory */
void *kcalloc(size_t count, size_t size) {
    size_t total_size = count * size;
    void *ptr = kmalloc(total_size);
    
    if (ptr) {
        memory_set(ptr, 0, total_size);
    }
    
    return ptr;
}

/* Reallocate memory */
void *krealloc(void *ptr, size_t size) {
    if (!ptr) return kmalloc(size);
    if (size == 0) {
        kfree(ptr);
        return NULL;
    }
    
    struct heap_block *block = (struct heap_block *)((uint8_t *)ptr - HEAP_BLOCK_SIZE);
    
    /* If new size fits in current block */
    if (block->size >= size) {
        return ptr;
    }
    
    /* Allocate new block and copy data */
    void *new_ptr = kmalloc(size);
    if (!new_ptr) return NULL;
    
    memory_copy(new_ptr, ptr, block->size < size ? block->size : size);
    kfree(ptr);
    
    return new_ptr;
}

/* Free memory */
void kfree(void *ptr) {
    if (!ptr) return;
    
    struct heap_block *block = (struct heap_block *)((uint8_t *)ptr - HEAP_BLOCK_SIZE);
    
    if (block->is_free) {
        serial_puts("[WARNING] Double free detected in heap nexus\n");
        return;
    }
    
    block->is_free = 1;
    used_heap_size -= block->size + HEAP_BLOCK_SIZE;
    
    /* Merge with adjacent free blocks */
    merge_free_blocks(block);
}

/* Memory utility functions */
void memory_set(void *dst, int value, size_t size) {
    uint8_t *d = (uint8_t *)dst;
    for (size_t i = 0; i < size; i++) {
        d[i] = (uint8_t)value;
    }
}

void memory_copy(void *dst, const void *src, size_t size) {
    const uint8_t *s = (const uint8_t *)src;
    uint8_t *d = (uint8_t *)dst;
    for (size_t i = 0; i < size; i++) {
        d[i] = s[i];
    }
}

int memory_compare(const void *a, const void *b, size_t size) {
    const uint8_t *pa = (const uint8_t *)a;
    const uint8_t *pb = (const uint8_t *)b;
    
    for (size_t i = 0; i < size; i++) {
        if (pa[i] < pb[i]) return -1;
        if (pa[i] > pb[i]) return 1;
    }
    return 0;
}