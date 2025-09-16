/* syscall.c - Brandon Media OS System Call Handler */
#include <stdint.h>
#include "kernel/syscalls.h"
#include "kernel/process.h"
#include "kernel/memory.h"
#include "kernel/interrupts.h"

/* External functions */
extern void serial_puts(const char *s);
extern void print_hex(uint64_t num);
extern void print_dec(uint64_t num);
extern struct process *process_get_current(void);
extern void process_sleep(uint32_t milliseconds);
extern void scheduler_yield(void);
extern void process_terminate(struct process *proc);

/* System call function pointer table */
typedef int64_t (*syscall_func_t)(uint64_t arg0, uint64_t arg1, uint64_t arg2, 
                                  uint64_t arg3, uint64_t arg4, uint64_t arg5);

/* Forward declarations */
static int64_t sys_invalid(uint64_t arg0, uint64_t arg1, uint64_t arg2, 
                          uint64_t arg3, uint64_t arg4, uint64_t arg5);

/* System call table - Neural interface command matrix */
static syscall_func_t syscall_table[MAX_SYSCALL_NUM + 1] = {
    (syscall_func_t)sys_exit,      /* 0: Neural process termination */
    (syscall_func_t)sys_read,      /* 1: Neural data stream read */
    (syscall_func_t)sys_write,     /* 2: Neural output write */
    (syscall_func_t)sys_open,      /* 3: Neural channel open */
    (syscall_func_t)sys_close,     /* 4: Neural channel close */
    (syscall_func_t)sys_fork,      /* 5: Neural process clone */
    (syscall_func_t)sys_exec,      /* 6: Neural program execute */
    (syscall_func_t)sys_wait,      /* 7: Neural process wait */
    (syscall_func_t)sys_kill,      /* 8: Neural process terminate */
    (syscall_func_t)sys_sleep,     /* 9: Temporal suspension */
    (syscall_func_t)sys_yield,     /* 10: Neural processing yield */
    (syscall_func_t)sys_getpid,    /* 11: Get process matrix ID */
    (syscall_func_t)sys_getppid,   /* 12: Get parent process ID */
    (syscall_func_t)sys_mmap,      /* 13: Neural memory mapping */
    (syscall_func_t)sys_munmap,    /* 14: Neural memory unmapping */
    (syscall_func_t)sys_brk,       /* 15: Heap boundary adjust */
    (syscall_func_t)sys_pipe,      /* 16: Neural data pipe */
    sys_invalid,                   /* 17: DUP - not implemented */
    sys_invalid,                   /* 18: IOCTL - not implemented */
    sys_invalid,                   /* 19: STAT - not implemented */
    sys_invalid,                   /* 20: MKDIR - not implemented */
    sys_invalid,                   /* 21: RMDIR - not implemented */
    sys_invalid,                   /* 22: CHDIR - not implemented */
    sys_invalid,                   /* 23: GETCWD - not implemented */
    sys_invalid,                   /* 24: SIGACTION - not implemented */
    sys_invalid,                   /* 25: SIGRETURN - not implemented */
    sys_invalid,                   /* 26: SOCKET - not implemented */
    sys_invalid,                   /* 27: BIND - not implemented */
    sys_invalid,                   /* 28: LISTEN - not implemented */
    sys_invalid,                   /* 29: ACCEPT - not implemented */
    sys_invalid,                   /* 30: CONNECT - not implemented */
    sys_invalid,                   /* 31: SEND - not implemented */
    sys_invalid,                   /* 32: RECV - not implemented */
};

/* System call statistics */
static struct {
    uint64_t total_calls;
    uint64_t call_counts[MAX_SYSCALL_NUM + 1];
    uint64_t error_counts;
} syscall_stats = {0};

/* Initialize system call interface */
void syscalls_init(void) {
    serial_puts("[NEXUS] Initializing neural interface gateway...\\n");
    
    /* Clear statistics */
    syscall_stats.total_calls = 0;
    syscall_stats.error_counts = 0;
    for (int i = 0; i <= MAX_SYSCALL_NUM; i++) {
        syscall_stats.call_counts[i] = 0;
    }
    
    serial_puts("[GATEWAY] Neural interface command matrix loaded\\n");
    serial_puts("[STATS] Available neural commands: ");
    print_dec(MAX_SYSCALL_NUM + 1);
    serial_puts("\\n");
    
    serial_puts("[NEXUS] Neural interface gateway online\\n");
}

/* Invalid system call handler */
static int64_t sys_invalid(uint64_t arg0, uint64_t arg1, uint64_t arg2, 
                          uint64_t arg3, uint64_t arg4, uint64_t arg5) {
    (void)arg0; (void)arg1; (void)arg2; (void)arg3; (void)arg4; (void)arg5;
    
    serial_puts("[ERROR] Invalid neural command - access denied\\n");
    syscall_stats.error_counts++;
    return -ENOSYS;
}

/* Main system call handler */
int64_t syscall_handler(struct syscall_params *params) {
    if (!params) {
        serial_puts("[ERROR] Null neural interface parameters\\n");
        return -EFAULT;
    }
    
    /* Validate system call number */
    if (params->syscall_num > MAX_SYSCALL_NUM) {
        serial_puts("[ERROR] Neural command out of range: ");
        print_dec(params->syscall_num);
        serial_puts("\\n");
        syscall_stats.error_counts++;
        return -ENOSYS;
    }
    
    /* Update statistics */
    syscall_stats.total_calls++;
    syscall_stats.call_counts[params->syscall_num]++;
    
    /* Log system call (for debugging) */
    serial_puts("[GATEWAY] Neural interface command: ");
    print_dec(params->syscall_num);
    serial_puts(" from PID: ");
    struct process *current = process_get_current();
    if (current) {
        print_dec(current->pid);
    } else {
        serial_puts("UNKNOWN");
    }
    serial_puts("\\n");
    
    /* Call the appropriate system call handler */
    syscall_func_t handler = syscall_table[params->syscall_num];
    int64_t result = handler(params->arg0, params->arg1, params->arg2,
                            params->arg3, params->arg4, params->arg5);
    
    /* Log result */
    if (result < 0) {
        serial_puts("[GATEWAY] Neural command failed with error: ");
        print_dec(-result);
        serial_puts("\\n");
        syscall_stats.error_counts++;
    }
    
    return result;
}

/* System call implementations */

/* Exit - terminate current process */
int64_t sys_exit(int32_t status) {
    struct process *current = process_get_current();
    if (!current) {
        return -ESRCH;
    }
    
    serial_puts("[EXIT] Neural process terminating: ");
    serial_puts(current->name);
    serial_puts(" with status: ");
    print_dec(status);
    serial_puts("\\n");
    
    /* Store exit status in process */
    current->cpu_time = status;  /* Reuse field for exit status */
    
    /* Terminate the process */
    process_terminate(current);
    
    /* This should not return */
    scheduler_yield();
    return 0;
}

/* Read from file descriptor */
int64_t sys_read(int32_t fd, void *buffer, size_t count) {
    serial_puts("[READ] Neural data stream read: FD ");
    print_dec(fd);
    serial_puts(", Count: ");
    print_dec(count);
    serial_puts("\\n");
    
    /* Simple implementation - only support stdin for now */
    if (fd == STDIN_FILENO) {
        /* For now, return 0 (EOF) */
        return 0;
    }
    
    return -EBADF;
}

/* Write to file descriptor */
int64_t sys_write(int32_t fd, const void *buffer, size_t count) {
    if (!buffer || count == 0) {
        return -EINVAL;
    }
    
    serial_puts("[WRITE] Neural output: FD ");
    print_dec(fd);
    serial_puts(", Count: ");
    print_dec(count);
    serial_puts("\\n");
    
    /* Support stdout and stderr */
    if (fd == STDOUT_FILENO || fd == STDERR_FILENO) {
        const char *data = (const char *)buffer;
        
        /* Write to serial console */
        serial_puts("[USER_OUTPUT] ");
        for (size_t i = 0; i < count; i++) {
            extern void serial_putc(char c);
            serial_putc(data[i]);
        }
        
        return count;
    }
    
    return -EBADF;
}

/* Open file */
int64_t sys_open(const char *pathname, int32_t flags, uint32_t mode) {
    (void)pathname; (void)flags; (void)mode;
    
    serial_puts("[OPEN] Neural channel open request\\n");
    
    /* Not implemented yet */
    return -ENOSYS;
}

/* Close file */
int64_t sys_close(int32_t fd) {
    serial_puts("[CLOSE] Neural channel close: FD ");
    print_dec(fd);
    serial_puts("\\n");
    
    /* Not implemented yet */
    return -ENOSYS;
}

/* Fork process */
int64_t sys_fork(void) {
    serial_puts("[FORK] Neural process cloning request\\n");
    
    /* Not implemented yet - complex operation */
    return -ENOSYS;
}

/* Execute program */
int64_t sys_exec(const char *pathname, char *const argv[], char *const envp[]) {
    (void)pathname; (void)argv; (void)envp;
    
    serial_puts("[EXEC] Neural program execution request\\n");
    
    /* Not implemented yet */
    return -ENOSYS;
}

/* Wait for child process */
int64_t sys_wait(int32_t *status) {
    (void)status;
    
    serial_puts("[WAIT] Neural process wait request\\n");
    
    /* Not implemented yet */
    return -ENOSYS;
}

/* Kill process */
int64_t sys_kill(uint32_t pid, int32_t signal) {
    serial_puts("[KILL] Neural process termination signal: PID ");
    print_dec(pid);
    serial_puts(", Signal: ");
    print_dec(signal);
    serial_puts("\\n");
    
    /* Not implemented yet */
    return -ENOSYS;
}

/* Sleep */
int64_t sys_sleep(uint32_t milliseconds) {
    serial_puts("[SLEEP] Neural temporal suspension: ");
    print_dec(milliseconds);
    serial_puts("ms\\n");
    
    process_sleep(milliseconds);
    return 0;
}

/* Yield CPU */
int64_t sys_yield(void) {
    serial_puts("[YIELD] Neural processing yield\\n");
    
    scheduler_yield();
    return 0;
}

/* Get process ID */
int64_t sys_getpid(void) {
    struct process *current = process_get_current();
    if (!current) {
        return -ESRCH;
    }
    
    serial_puts("[GETPID] Neural process matrix ID: ");
    print_dec(current->pid);
    serial_puts("\\n");
    
    return current->pid;
}

/* Get parent process ID */
int64_t sys_getppid(void) {
    struct process *current = process_get_current();
    if (!current) {
        return -ESRCH;
    }
    
    serial_puts("[GETPPID] Parent neural process ID: ");
    print_dec(current->ppid);
    serial_puts("\\n");
    
    return current->ppid;
}

/* Memory map */
int64_t sys_mmap(void *addr, size_t length, int32_t prot, int32_t flags, int32_t fd, uint64_t offset) {
    (void)addr; (void)length; (void)prot; (void)flags; (void)fd; (void)offset;
    
    serial_puts("[MMAP] Neural memory mapping request\\n");
    
    /* Not implemented yet */
    return -ENOSYS;
}

/* Memory unmap */
int64_t sys_munmap(void *addr, size_t length) {
    (void)addr; (void)length;
    
    serial_puts("[MUNMAP] Neural memory unmapping request\\n");
    
    /* Not implemented yet */
    return -ENOSYS;
}

/* Break (adjust heap) */
int64_t sys_brk(void *addr) {
    (void)addr;
    
    serial_puts("[BRK] Neural heap boundary adjustment\\n");
    
    /* Not implemented yet */
    return -ENOSYS;
}

/* Create pipe */
int64_t sys_pipe(int32_t pipefd[2]) {
    if (!pipefd) {
        return -EFAULT;
    }
    
    serial_puts("[PIPE] Neural data pipe creation\\n");
    
    /* Use existing IPC pipe creation */
    extern int ipc_create_pipe(uint32_t *read_fd, uint32_t *write_fd);
    uint32_t read_fd, write_fd;
    
    int result = ipc_create_pipe(&read_fd, &write_fd);
    if (result == 0) {
        pipefd[0] = read_fd;
        pipefd[1] = write_fd;
        return 0;
    }
    
    return -ENOSYS;
}

/* Error handling functions */
const char *syscall_strerror(int32_t error_code) {
    switch (error_code) {
        case ESUCCESS: return "Neural operation successful";
        case EGENERIC: return "Generic neural error";
        case ENOENT: return "Matrix node not found";
        case ESRCH: return "Process not in matrix";
        case EINTR: return "Neural operation interrupted";
        case EIO: return "Neural I/O error";
        case ENOMEM: return "Neural memory exhausted";
        case EACCES: return "Neural access denied";
        case EFAULT: return "Matrix memory fault";
        case EBUSY: return "Neural resource busy";
        case EEXIST: return "Matrix node already exists";
        case EINVAL: return "Invalid neural argument";
        case EBADF: return "Invalid neural channel";
        case ENOSYS: return "Neural function not implemented";
        default: return "Unknown neural error";
    }
}