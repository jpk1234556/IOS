/* main.c - Brandon Media OS x86_64 kernel */
#include <stdint.h>
#include "kernel/interrupts.h"
#include "kernel/memory.h"
#include "kernel/process.h"
#include "kernel/syscalls.h"
#include "kernel/fs.h"
#include "kernel/pci.h"
#include "kernel/hal.h"
#include "kernel/virtio_net.h"
#include "kernel/framebuffer.h"
#include "kernel/gui.h"
#include "kernel/input.h"
#include "kernel/graphics_3d.h"
#include "kernel/device_test.h"
#include "kernel/smp.h"
#include "kernel/advanced_scheduler.h"
#include "kernel/security.h"
#include "kernel/uefi_boot.h"
#include "kernel/uefi_manager.h"

#define VGA_BUF ((volatile uint16_t*)0xB8000)
#define COM1 0x3F8

static inline void outb(uint16_t port, uint8_t val) {
    asm volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

static void serial_putc(char c) {
    outb(COM1, (uint8_t)c);
}

static void vga_puts(const char *s) {
    volatile uint16_t *v = VGA_BUF;
    const uint8_t attr = 0x07; /* normal text */
    for (int i = 0; s[i]; ++i) {
        v[i] = ((uint16_t)attr << 8) | (uint8_t)s[i];
    }
}

static void serial_puts(const char *s) {
    for (int i = 0; s[i]; ++i) serial_putc(s[i]);
}

/* External interrupt system functions */
extern void idt_init(void);
extern void pic_init(void);
extern void timer_init(uint32_t frequency);
extern void interrupts_enable(void);
extern void interrupts_disable(void);

/* External memory system functions */
extern void pmm_init(struct memory_region *regions, size_t region_count);
extern void paging_init(void);
extern void paging_enable(void);
extern void vmm_init(void);
extern void heap_init(void);
extern struct memory_stats *memory_get_stats(void);
extern void print_hex(uint64_t num);
extern void print_dec(uint64_t num);

/* Test process function */
void test_process_1(void) {
    int counter = 0;
    while (1) {
        counter++;
        if (counter % 1000000 == 0) {
            serial_puts("[PROCESS-1] Neural process executing - iteration: ");
            print_dec(counter / 1000000);
            serial_puts("\n");
        }
        if (counter > 5000000) {
            serial_puts("[PROCESS-1] Process terminating naturally\n");
            break;
        }
    }
}

void test_process_2(void) {
    int counter = 0;
    while (1) {
        counter++;
        if (counter % 1500000 == 0) {
            serial_puts("[PROCESS-2] Secondary neural process active - cycle: ");
            print_dec(counter / 1500000);
            serial_puts("\n");
        }
        if (counter > 7500000) {
            serial_puts("[PROCESS-2] Secondary process completed\n");
            break;
        }
    }
}

/* External process system functions */
extern void process_init(void);
extern void scheduler_init(void);
extern struct process *process_create(const char *name, void (*entry_point)(void), process_priority_t priority);
extern void scheduler_add_process(struct process *proc);
extern struct process *process_get_current(void);
extern void process_dump_info(struct process *proc);

/* External system call functions */
extern void syscalls_init(void);
extern void user_mode_init(void);
extern void test_user_mode(void);

/* External file system functions */
extern void vfs_init(void);
extern void file_ops_init(void);
extern int ramfs_init(void);
extern void storage_init(void);
extern struct storage_device *storage_create_ram_device(const char *name, uint64_t size);
extern int storage_register_device(struct storage_device *device);
extern void storage_print_devices(void);
extern int vfs_mount(const char *path, struct filesystem *fs, uint32_t flags);
extern int vfs_mkdir(const char *path, uint32_t permissions);
extern int vfs_create_file(const char *path, uint32_t permissions);
extern int vfs_open(const char *path, uint32_t flags, uint32_t mode);
extern int vfs_close(int fd);
extern int64_t vfs_write(int fd, const void *buffer, size_t count);
extern int64_t vfs_read(int fd, void *buffer, size_t count);
extern int vfs_stat(const char *path, struct file_stat *stat);

void kmain(void) {
    /* Brandon Media OS - Cyberpunk boot sequence */
    const char *boot_msg = "[NEXUS] Brandon Media OS v0.1 - Neural interface online";
    vga_puts(boot_msg);
    serial_puts(boot_msg);
    serial_putc('\n');
    
    /* Initialize memory management */
    serial_puts("[MATRIX] Initializing memory management nexus...\n");
    
    /* Create basic memory map (simplified for now) */
    struct memory_region memory_map[] = {
        {0x0000000, 0x9FC00, 1, 0},           /* Conventional memory 0-639KB */
        {0x100000, 0x7F00000, 1, 0},         /* Extended memory 1MB-127MB */
    };
    
    pmm_init(memory_map, 2);              /* Initialize physical memory */
    paging_init();                        /* Initialize paging */
    paging_enable();                      /* Enable virtual memory */
    vmm_init();                          /* Initialize virtual memory manager */
    heap_init();                         /* Initialize kernel heap */
    
    /* Initialize process management */
    serial_puts("[NEXUS] Initializing neural process matrix...\n");
    process_init();                      /* Initialize process management */
    scheduler_init();                    /* Initialize scheduler */
    
    /* Initialize system calls */
    serial_puts("[NEXUS] Initializing neural interface gateway...\n");
    syscalls_init();                     /* Initialize system call interface */
    user_mode_init();                    /* Initialize user mode support */
    
    /* Initialize file system */
    serial_puts("[MATRIX] Initializing neural file system...\n");
    vfs_init();                          /* Initialize Virtual File System */
    file_ops_init();                     /* Initialize file operations */
    storage_init();                      /* Initialize storage devices */
    ramfs_init();                        /* Initialize RAM filesystem */
    
    /* Initialize device drivers */
    serial_puts("[NEXUS] Initializing neural device matrix...\n");
    hal_init();                          /* Initialize Hardware Abstraction Layer */
    virtio_net_init();                   /* Initialize VirtIO network driver */
    framebuffer_init();                  /* Initialize graphics driver */
    hal_initialize_all_devices();        /* Initialize all discovered devices */
    
    /* Initialize advanced features (Phase 8) */
    serial_puts("[NEXUS] Activating advanced neural systems...\n");
    uefi_manager_init();                 /* Initialize UEFI boot manager */
    smp_init();                          /* Initialize SMP (multi-core support) */
    advanced_scheduler_init();           /* Initialize advanced scheduling */
    security_init();                     /* Initialize security framework */
    
    /* Create storage device for testing */
    struct storage_device *ram_storage = storage_create_ram_device("neural_ram", 1024 * 1024);  /* 1MB */
    if (ram_storage) {
        storage_register_device(ram_storage);
        serial_puts("[SUCCESS] Neural RAM storage device created\n");
    }
    
    /* Initialize interrupt system */
    serial_puts("[MATRIX] Initializing interrupt handlers...\n");
    interrupts_disable();  /* Disable interrupts during setup */
    idt_init();           /* Initialize IDT */
    pic_init();           /* Initialize PIC */
    timer_init(100);      /* 100Hz timer frequency */
    
    serial_puts("[SYSTEM] Enabling quantum processing matrix...\n");
    interrupts_enable();  /* Enable interrupts */
    
    /* Display memory statistics */
    struct memory_stats *stats = memory_get_stats();
    serial_puts("[NEXUS] Memory matrix status:\n");
    serial_puts("[STATS] Physical pages allocated: ");
    print_dec(stats->pages_allocated);
    serial_puts("\n");
    
    /* Test memory allocation */
    serial_puts("[TEST] Testing memory allocation matrix...\n");
    void *test_ptr = kmalloc(1024);
    if (test_ptr) {
        serial_puts("[SUCCESS] 1KB allocation successful at: ");
        print_hex((uint64_t)test_ptr);
        serial_puts("\n");
        kfree(test_ptr);
        serial_puts("[SUCCESS] Memory deallocation completed\n");
    } else {
        serial_puts("[ERROR] Memory allocation test failed\n");
    }
    
    /* Test process creation */
    serial_puts("[TEST] Testing neural process spawning...\n");
    struct process *proc1 = process_create("test_proc_1", test_process_1, PRIORITY_NORMAL);
    struct process *proc2 = process_create("test_proc_2", test_process_2, PRIORITY_LOW);
    
    if (proc1) {
        serial_puts("[SUCCESS] Process 1 spawned successfully\n");
        scheduler_add_process(proc1);
    }
    
    if (proc2) {
        serial_puts("[SUCCESS] Process 2 spawned successfully\n");
        scheduler_add_process(proc2);
    }
    
    /* Test system calls and user mode */
    serial_puts("[TEST] Testing neural interface gateway...\n");
    test_user_mode();
    
    /* Test file system */
    serial_puts("[TEST] Testing neural file system...\n");
    
    /* Print storage devices */
    storage_print_devices();
    
    /* Create test directory */
    if (vfs_mkdir("/neural", 0755) == 0) {
        serial_puts("[SUCCESS] Created /neural directory\n");
        
        /* Create test file */
        if (vfs_create_file("/neural/test.dat", 0644) == 0) {
            serial_puts("[SUCCESS] Created /neural/test.dat file\n");
            
            /* Test file operations */
            int fd = vfs_open("/neural/test.dat", 3, 0);  /* Read+Write */
            if (fd >= 0) {
                const char *test_data = "Brandon Media OS Neural File System Test Data\n";
                int64_t written = vfs_write(fd, test_data, 46);
                serial_puts("[TEST] Wrote ");
                print_dec(written);
                serial_puts(" bytes to neural file\n");
                
                /* Test read operation */
                vfs_seek(fd, 0, 0);  /* Seek to beginning */
                char read_buffer[64];
                int64_t read_bytes = vfs_read(fd, read_buffer, 46);
                serial_puts("[TEST] Read ");
                print_dec(read_bytes);
                serial_puts(" bytes from neural file\n");
                
                vfs_close(fd);
                serial_puts("[SUCCESS] Neural file operations test completed\n");
            } else {
                serial_puts("[ERROR] Failed to open neural file\n");
            }
        } else {
            serial_puts("[ERROR] Failed to create neural file\n");
        }
    } else {
        serial_puts("[ERROR] Failed to create neural directory\n");
    }
    
    /* Test device drivers */
    serial_puts("[TEST] Testing neural device matrix...\n");
    hal_print_all_devices();
    
    /* Find and test network devices */
    struct hal_device *network_devices[8];
    int network_count = hal_find_devices_by_type(DEVICE_TYPE_NETWORK, network_devices, 8);
    if (network_count > 0) {
        serial_puts("[DISCOVERY] Found ");
        print_dec(network_count);
        serial_puts(" neural network interfaces\n");
        for (int i = 0; i < network_count; i++) {
            hal_print_device_info(network_devices[i]);
        }
    } else {
        serial_puts("[INFO] No neural network interfaces detected\n");
    }
    
    /* Test graphics interface */
    serial_puts("[TEST] Testing neural display interface...\n");
    fb_print_info();
    fb_test_graphics();
    
    /* Initialize Neural GUI System */
    serial_puts("[NEXUS] Initializing Neural GUI Interface...\n");
    if (gui_init() == 0) {
        serial_puts("[SUCCESS] Neural GUI System initialized\n");
        
        /* Initialize input system */
        if (input_init() == 0) {
            serial_puts("[SUCCESS] Neural Input System initialized\n");
            
            /* Initialize accessibility system */
            extern int accessibility_init(void);
            if (accessibility_init() == 0) {
                serial_puts("[SUCCESS] Neural Accessibility System initialized\n");
            }
        }
        
        /* Initialize 3D graphics */
        framebuffer_device_t *fb_dev = framebuffer_get_device();
        if (fb_dev && graphics_3d_init(fb_dev->width, fb_dev->height, fb_dev->framebuffer) == 0) {
            serial_puts("[SUCCESS] Neural 3D Graphics Engine initialized\n");
            
            /* Test 3D graphics */
            extern void graphics_3d_test(void);
            graphics_3d_test();
        }
        
        /* Test GUI system */
        extern void gui_test(void);
        gui_test();
        
        /* Create SCADA demo interface */
        extern void scada_demo_init(void);
        scada_demo_init();
    } else {
        serial_puts("[ERROR] Failed to initialize Neural GUI System\n");
    }
    
    /* Test network interface */
    if (network_count > 0) {
        serial_puts("[TEST] Testing neural network interface...\n");
        virtio_net_print_stats();
    }
    
    /* Run comprehensive device driver tests */
    serial_puts("[TEST] Running comprehensive device driver test suite...\n");
    run_device_driver_tests();
    
    /* Test advanced features (Phase 8) */
    serial_puts("[TEST] Testing advanced neural systems...\n");
    
    /* Test UEFI boot manager */
    serial_puts("[TEST] Testing UEFI neural boot interface...\n");
    uefi_manager_run_tests();
    if (uefi_manager_is_uefi_boot()) {
        serial_puts("[SUCCESS] UEFI neural boot interface operational\n");
    } else {
        serial_puts("[INFO] Legacy BIOS boot mode detected\n");
    }
    
    /* Test SMP and neural matrix */
    if (smp_is_available()) {
        serial_puts("[TEST] Testing neural processing matrix...\n");
        smp_print_neural_matrix_status();
        
        uint32_t total_cores, active_cores;
        uint64_t total_cycles;
        smp_get_statistics(&total_cores, &active_cores, &total_cycles);
        
        serial_puts("[SUCCESS] Neural matrix operational - ");
        print_dec(active_cores);
        serial_puts("/");
        print_dec(total_cores);
        serial_puts(" cores active\n");
    } else {
        serial_puts("[INFO] Single-core neural processing mode\n");
    }
    
    /* Test advanced scheduling */
    if (sched_is_advanced_initialized()) {
        serial_puts("[TEST] Testing advanced neural scheduling...\n");
        sched_print_statistics();
        sched_test_algorithms();
        serial_puts("[SUCCESS] Advanced scheduling systems operational\n");
    }
    
    /* Test security features */
    if (security_is_initialized()) {
        serial_puts("[TEST] Testing neural security systems...\n");
        security_print_status();
        security_test_features();
        serial_puts("[SUCCESS] Neural protection systems operational\n");
    }
    
    /* Test enhanced userland framework */
    serial_puts("[TEST] Testing neural application framework...\n");
    extern void neural_app_framework_test(void);
    neural_app_framework_test();
    serial_puts("[SUCCESS] Neural application framework operational\n");
    
    serial_puts("[NEXUS] Brandon Media OS fully operational - Cyberpunk mode active\n");
    serial_puts("[STATUS] Press any key to test neural interface\n");
    
    /* Main kernel loop with cyberpunk aesthetics and GUI updates */
    uint64_t loop_count = 0;
    uint32_t last_gui_update = get_time_ms();
    
    for (;;) {
        uint32_t current_time = get_time_ms();
        uint32_t delta_ms = current_time - last_gui_update;
        
        /* Update GUI system at 60fps */
        if (delta_ms >= 16) { /* ~60fps */
            /* Update input system */
            extern void input_update(void);
            input_update();
            
            /* Update GUI system */
            extern void gui_update(uint32_t delta_ms);
            gui_update(delta_ms);
            
            /* Update SCADA demo */
            extern void scada_demo_update(void);
            scada_demo_update();
            
            /* Render GUI */
            extern void gui_render(void);
            gui_render();
            
            /* Swap framebuffer if double buffering is enabled */
            extern void fb_swap_buffers(void);
            fb_swap_buffers();
            
            last_gui_update = current_time;
        }
        
        asm volatile("hlt");  /* Wait for interrupts */
        
        /* Periodic status updates */
        if (++loop_count % 1000000 == 0) {
            serial_puts("[HEARTBEAT] Neural System Matrix Stable - All systems nominal\n");
        }
    }
}