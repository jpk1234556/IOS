/* user_mode.c - Brandon Media OS User Mode Support */
#include <stdint.h>
#include "kernel/syscalls.h"
#include "kernel/process.h"
#include "kernel/memory.h"

/* External functions */
extern void serial_puts(const char *s);
extern void print_hex(uint64_t num);
extern void print_dec(uint64_t num);
extern void setup_syscall_msrs(void);
extern void user_mode_test(void);
extern struct process *process_create(const char *name, void (*entry_point)(void), process_priority_t priority);
extern void scheduler_add_process(struct process *proc);

/* User mode initialization */
void user_mode_init(void) {
    serial_puts("[NEXUS] Initializing neural user mode interface...\\n");
    
    /* Setup MSR registers for syscall/sysret */
    setup_syscall_msrs();
    
    serial_puts("[USER_MODE] Neural syscall/sysret interface configured\\n");
    serial_puts("[USER_MODE] User code segment: 0x1B (Ring 3)\\n");
    serial_puts("[USER_MODE] User data segment: 0x23 (Ring 3)\\n");
    
    serial_puts("[NEXUS] Neural user mode interface online\\n");
}

/* Create user mode process */
struct process *create_user_process(const char *name, void (*entry_point)(void)) {
    serial_puts("[USER_MODE] Creating neural user process: ");
    serial_puts(name);
    serial_puts("\\n");
    
    /* Create process with user priority */
    struct process *proc = process_create(name, entry_point, PRIORITY_NORMAL);
    if (!proc) {
        serial_puts("[ERROR] Failed to create neural user process\\n");
        return NULL;
    }
    
    /* Modify process context for user mode */
    proc->context.cs = 0x1B;        /* User code segment (Ring 3) */
    proc->context.ds = 0x23;        /* User data segment (Ring 3) */
    proc->context.es = 0x23;
    proc->context.fs = 0x23;
    proc->context.gs = 0x23;
    proc->context.ss = 0x23;        /* User stack segment (Ring 3) */
    proc->context.rflags |= 0x200;  /* Enable interrupts */
    
    /* Allocate user stack in user space */
    uint64_t user_stack_base = 0x7FFFF000;  /* User space stack */
    uint64_t user_stack_size = 4096;        /* 4KB user stack */
    
    /* Map user stack with user permissions */
    uint64_t physical_stack = pmm_alloc_frame();
    if (!physical_stack) {
        serial_puts("[ERROR] Failed to allocate user stack frame\\n");
        process_destroy(proc);
        return NULL;
    }
    
    if (paging_map_page(proc->page_directory, user_stack_base, physical_stack,
                       PAGE_PRESENT | PAGE_WRITABLE | PAGE_USER) != 0) {
        serial_puts("[ERROR] Failed to map user stack\\n");
        pmm_free_frame(physical_stack);
        process_destroy(proc);
        return NULL;
    }
    
    /* Set user stack pointer */
    proc->context.rsp = user_stack_base + user_stack_size - 16;  /* 16-byte aligned */
    
    serial_puts("[SUCCESS] Neural user process created - PID: ");
    print_dec(proc->pid);
    serial_puts("\\n");
    
    return proc;
}

/* Setup user stack with arguments */
void setup_user_stack(struct process *proc, int argc, char *argv[], char *envp[]) {
    if (!proc) return;
    
    serial_puts("[USER_MODE] Setting up neural user stack with arguments\\n");
    
    /* For now, just log the setup - full implementation would copy argv/envp */
    serial_puts("[STACK] Arguments count: ");
    print_dec(argc);
    serial_puts("\\n");
    
    (void)argv;  /* Suppress unused parameter warning */
    (void)envp;  /* Suppress unused parameter warning */
    
    /* TODO: Implement full argument copying to user stack */
}

/* Simple ELF header structure */
struct elf_header {
    uint8_t e_ident[16];    /* ELF identification */
    uint16_t e_type;        /* Object file type */
    uint16_t e_machine;     /* Machine type */
    uint32_t e_version;     /* Object file version */
    uint64_t e_entry;       /* Entry point address */
    uint64_t e_phoff;       /* Program header offset */
    uint64_t e_shoff;       /* Section header offset */
    uint32_t e_flags;       /* Processor-specific flags */
    uint16_t e_ehsize;      /* ELF header size */
    uint16_t e_phentsize;   /* Program header entry size */
    uint16_t e_phnum;       /* Number of program header entries */
    uint16_t e_shentsize;   /* Section header entry size */
    uint16_t e_shnum;       /* Number of section header entries */
    uint16_t e_shstrndx;    /* String table index */
} __attribute__((packed));

/* Load ELF executable (simplified) */
int load_elf_executable(const char *path, struct process *proc) {
    if (!path || !proc) {
        return -EINVAL;
    }
    
    serial_puts("[ELF] Loading neural executable: ");
    serial_puts(path);
    serial_puts("\\n");
    
    /* For now, return not implemented */
    serial_puts("[ELF] Neural executable loader not yet implemented\\n");
    
    return -ENOSYS;
}

/* Test user mode functionality */
void test_user_mode(void) {
    serial_puts("[TEST] Testing neural user mode interface...\\n");
    
    /* Create user mode test process */
    struct process *user_proc = create_user_process("user_test", user_mode_test);
    if (!user_proc) {
        serial_puts("[ERROR] Failed to create neural user test process\\n");
        return;
    }
    
    /* Add to scheduler */
    scheduler_add_process(user_proc);
    
    serial_puts("[SUCCESS] Neural user mode test process spawned\\n");
}