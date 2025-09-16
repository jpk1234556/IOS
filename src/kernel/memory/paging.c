/* paging.c - Brandon Media OS x86_64 Paging System */
#include <stdint.h>
#include "kernel/memory.h"
#include "kernel/interrupts.h"

/* External functions */
extern void serial_puts(const char *s);
extern void print_hex(uint64_t num);

/* Current page directory */
static pml4_t *current_pml4 = NULL;
static pml4_t *kernel_pml4 = NULL;

/* Memory statistics */
static struct memory_stats mem_stats = {0};

/* Assembly functions for CR3 management */
extern void load_cr3(uint64_t pml4_physical);
extern uint64_t get_cr3(void);
extern void flush_tlb(void);

/* Brandon Media OS - Cyberpunk memory initialization */
void paging_init(void) {
    serial_puts("[MATRIX] Initializing neural memory interface...\n");
    serial_puts("[PAGING] Constructing virtual reality matrix...\n");
    
    /* Allocate kernel PML4 */
    kernel_pml4 = (pml4_t *)pmm_alloc_frame();
    if (!kernel_pml4) {
        serial_puts("[FATAL] Matrix memory allocation failed - system breach\n");
        return;
    }
    
    /* Clear kernel PML4 */
    memory_set(kernel_pml4, 0, PAGE_SIZE);
    current_pml4 = kernel_pml4;
    
    /* Identity map first 4MB for kernel */
    serial_puts("[MATRIX] Mapping kernel reality anchor points...\n");
    for (uint64_t addr = 0; addr < KERNEL_PHYSICAL_END; addr += PAGE_SIZE) {
        paging_map_page(kernel_pml4, addr, addr, 
                       PAGE_PRESENT | PAGE_WRITABLE | PAGE_GLOBAL);
    }
    
    /* Map kernel to higher half */
    serial_puts("[MATRIX] Creating higher-dimensional kernel space...\n");
    for (uint64_t addr = 0; addr < KERNEL_PHYSICAL_END; addr += PAGE_SIZE) {
        paging_map_page(kernel_pml4, KERNEL_VIRTUAL_BASE + addr, addr,
                       PAGE_PRESENT | PAGE_WRITABLE | PAGE_GLOBAL);
    }
    
    serial_puts("[MATRIX] Virtual reality matrix constructed successfully\n");
}

/* Enable paging with cyberpunk flair */
void paging_enable(void) {
    serial_puts("[NEXUS] Activating virtual memory matrix...\n");
    
    if (!kernel_pml4) {
        serial_puts("[ERROR] No PML4 matrix available - system unstable\n");
        return;
    }
    
    /* Load PML4 into CR3 */
    uint64_t pml4_physical = (uint64_t)kernel_pml4;
    load_cr3(pml4_physical);
    
    serial_puts("[NEXUS] Virtual memory matrix online - reality layer active\n");
}

/* Create new address space */
pml4_t *paging_create_address_space(void) {
    serial_puts("[MATRIX] Spawning new virtual reality instance...\n");
    
    /* Allocate new PML4 */
    pml4_t *new_pml4 = (pml4_t *)pmm_alloc_frame();
    if (!new_pml4) {
        serial_puts("[ERROR] Virtual reality spawn failed - memory shortage\n");
        return NULL;
    }
    
    /* Clear new PML4 */
    memory_set(new_pml4, 0, PAGE_SIZE);
    
    /* Copy kernel mappings (higher half) */
    for (int i = 256; i < 512; i++) {
        new_pml4->entries[i] = kernel_pml4->entries[i];
    }
    
    serial_puts("[MATRIX] New virtual reality instance created\n");
    return new_pml4;
}

/* Switch address space */
void paging_switch_address_space(pml4_t *pml4) {
    if (!pml4) {
        serial_puts("[ERROR] Invalid virtual reality matrix - switch aborted\n");
        return;
    }
    
    current_pml4 = pml4;
    load_cr3((uint64_t)pml4);
    serial_puts("[MATRIX] Reality matrix switched - entering new dimension\n");
}

/* Map virtual page to physical frame */
int paging_map_page(pml4_t *pml4, uint64_t virtual_addr, uint64_t physical_addr, uint64_t flags) {
    /* Extract page table indices */
    uint64_t pml4_idx = PML4_INDEX(virtual_addr);
    uint64_t pdpt_idx = PDPT_INDEX(virtual_addr);
    uint64_t pd_idx = PD_INDEX(virtual_addr);
    uint64_t pt_idx = PT_INDEX(virtual_addr);
    
    /* Get or create PDPT */
    pdpt_t *pdpt;
    if (!(pml4->entries[pml4_idx] & PAGE_PRESENT)) {
        pdpt = (pdpt_t *)pmm_alloc_frame();
        if (!pdpt) return -1;
        memory_set(pdpt, 0, PAGE_SIZE);
        pml4->entries[pml4_idx] = (uint64_t)pdpt | PAGE_PRESENT | PAGE_WRITABLE;
    } else {
        pdpt = (pdpt_t *)(pml4->entries[pml4_idx] & ~PAGE_MASK);
    }
    
    /* Get or create PD */
    pd_t *pd;
    if (!(pdpt->entries[pdpt_idx] & PAGE_PRESENT)) {
        pd = (pd_t *)pmm_alloc_frame();
        if (!pd) return -1;
        memory_set(pd, 0, PAGE_SIZE);
        pdpt->entries[pdpt_idx] = (uint64_t)pd | PAGE_PRESENT | PAGE_WRITABLE;
    } else {
        pd = (pd_t *)(pdpt->entries[pdpt_idx] & ~PAGE_MASK);
    }
    
    /* Get or create PT */
    pt_t *pt;
    if (!(pd->entries[pd_idx] & PAGE_PRESENT)) {
        pt = (pt_t *)pmm_alloc_frame();
        if (!pt) return -1;
        memory_set(pt, 0, PAGE_SIZE);
        pd->entries[pd_idx] = (uint64_t)pt | PAGE_PRESENT | PAGE_WRITABLE;
    } else {
        pt = (pt_t *)(pd->entries[pd_idx] & ~PAGE_MASK);
    }
    
    /* Map the page */
    pt->entries[pt_idx] = (physical_addr & ~PAGE_MASK) | flags;
    
    /* Flush TLB */
    flush_tlb();
    
    mem_stats.pages_allocated++;
    return 0;
}

/* Unmap virtual page */
int paging_unmap_page(pml4_t *pml4, uint64_t virtual_addr) {
    uint64_t pml4_idx = PML4_INDEX(virtual_addr);
    uint64_t pdpt_idx = PDPT_INDEX(virtual_addr);
    uint64_t pd_idx = PD_INDEX(virtual_addr);
    uint64_t pt_idx = PT_INDEX(virtual_addr);
    
    /* Check if page is mapped */
    if (!(pml4->entries[pml4_idx] & PAGE_PRESENT)) return -1;
    
    pdpt_t *pdpt = (pdpt_t *)(pml4->entries[pml4_idx] & ~PAGE_MASK);
    if (!(pdpt->entries[pdpt_idx] & PAGE_PRESENT)) return -1;
    
    pd_t *pd = (pd_t *)(pdpt->entries[pdpt_idx] & ~PAGE_MASK);
    if (!(pd->entries[pd_idx] & PAGE_PRESENT)) return -1;
    
    pt_t *pt = (pt_t *)(pd->entries[pd_idx] & ~PAGE_MASK);
    if (!(pt->entries[pt_idx] & PAGE_PRESENT)) return -1;
    
    /* Unmap the page */
    pt->entries[pt_idx] = 0;
    
    /* Flush TLB */
    flush_tlb();
    
    mem_stats.pages_freed++;
    return 0;
}

/* Get physical address from virtual address */
uint64_t paging_get_physical_address(pml4_t *pml4, uint64_t virtual_addr) {
    uint64_t pml4_idx = PML4_INDEX(virtual_addr);
    uint64_t pdpt_idx = PDPT_INDEX(virtual_addr);
    uint64_t pd_idx = PD_INDEX(virtual_addr);
    uint64_t pt_idx = PT_INDEX(virtual_addr);
    uint64_t offset = PAGE_OFFSET(virtual_addr);
    
    if (!(pml4->entries[pml4_idx] & PAGE_PRESENT)) return 0;
    
    pdpt_t *pdpt = (pdpt_t *)(pml4->entries[pml4_idx] & ~PAGE_MASK);
    if (!(pdpt->entries[pdpt_idx] & PAGE_PRESENT)) return 0;
    
    pd_t *pd = (pd_t *)(pdpt->entries[pdpt_idx] & ~PAGE_MASK);
    if (!(pd->entries[pd_idx] & PAGE_PRESENT)) return 0;
    
    pt_t *pt = (pt_t *)(pd->entries[pd_idx] & ~PAGE_MASK);
    if (!(pt->entries[pt_idx] & PAGE_PRESENT)) return 0;
    
    uint64_t physical_page = pt->entries[pt_idx] & ~PAGE_MASK;
    return physical_page + offset;
}

/* Get memory statistics */
struct memory_stats *memory_get_stats(void) {
    return &mem_stats;
}