/* syscalls.h - Brandon Media OS System Call Interface */
#ifndef _SYSCALLS_H
#define _SYSCALLS_H

#include <stdint.h>
#include <stddef.h>

/* Brandon Media OS - Neural Interface Gateway Definitions */

/* System call numbers - Cyberpunk neural interface commands */
#define SYS_EXIT            0   /* Terminate neural process */
#define SYS_READ            1   /* Read from neural data stream */
#define SYS_WRITE           2   /* Write to neural output */
#define SYS_OPEN            3   /* Open neural file channel */
#define SYS_CLOSE           4   /* Close neural channel */
#define SYS_FORK            5   /* Spawn neural process clone */
#define SYS_EXEC            6   /* Execute neural program */
#define SYS_WAIT            7   /* Wait for neural process */
#define SYS_KILL            8   /* Terminate target process */
#define SYS_SLEEP           9   /* Enter temporal suspension */
#define SYS_YIELD           10  /* Yield neural processing */
#define SYS_GETPID          11  /* Get process matrix ID */
#define SYS_GETPPID         12  /* Get parent process ID */
#define SYS_MMAP            13  /* Map neural memory */
#define SYS_MUNMAP          14  /* Unmap neural memory */
#define SYS_BRK             15  /* Adjust heap boundary */
#define SYS_PIPE            16  /* Create neural data pipe */
#define SYS_DUP             17  /* Duplicate file descriptor */
#define SYS_IOCTL           18  /* I/O control operations */
#define SYS_STAT            19  /* Get file matrix status */
#define SYS_MKDIR           20  /* Create directory node */
#define SYS_RMDIR           21  /* Remove directory node */
#define SYS_CHDIR           22  /* Change working directory */
#define SYS_GETCWD          23  /* Get current directory */
#define SYS_SIGACTION       24  /* Signal handling setup */
#define SYS_SIGRETURN       25  /* Return from signal */
#define SYS_SOCKET          26  /* Create network socket */
#define SYS_BIND            27  /* Bind socket address */
#define SYS_LISTEN          28  /* Listen for connections */
#define SYS_ACCEPT          29  /* Accept connection */
#define SYS_CONNECT         30  /* Connect to remote */
#define SYS_SEND            31  /* Send network data */
#define SYS_RECV            32  /* Receive network data */

#define MAX_SYSCALL_NUM     32

/* System call error codes */
#define ESUCCESS            0   /* Neural operation successful */
#define EGENERIC           -1   /* Generic neural error */
#define ENOENT             -2   /* Matrix node not found */
#define ESRCH              -3   /* Process not in matrix */
#define EINTR              -4   /* Neural operation interrupted */
#define EIO                -5   /* Neural I/O error */
#define ENXIO              -6   /* Device not in matrix */
#define E2BIG              -7   /* Data too large for neural processing */
#define ENOEXEC            -8   /* Invalid neural executable */
#define EBADF              -9   /* Invalid neural channel */
#define ECHILD             -10  /* No child process in matrix */
#define EAGAIN             -11  /* Neural resource unavailable */
#define ENOMEM             -12  /* Neural memory exhausted */
#define EACCES             -13  /* Neural access denied */
#define EFAULT             -14  /* Matrix memory fault */
#define EBUSY              -15  /* Neural resource busy */
#define EEXIST             -16  /* Matrix node already exists */
#define EXDEV              -17  /* Cross-device neural link */
#define ENODEV             -18  /* Device not found in matrix */
#define ENOTDIR            -19  /* Not a directory node */
#define EISDIR             -20  /* Is a directory node */
#define EINVAL             -21  /* Invalid neural argument */
#define ENFILE             -22  /* Neural file table overflow */
#define EMFILE             -23  /* Too many open neural channels */
#define ENOTTY             -24  /* Not a neural terminal */
#define ETXTBSY            -25  /* Neural text file busy */
#define EFBIG              -26  /* Neural file too large */
#define ENOSPC             -27  /* No space in neural matrix */
#define ESPIPE             -28  /* Illegal neural seek */
#define EROFS              -29  /* Read-only neural filesystem */
#define EMLINK             -30  /* Too many neural links */
#define EPIPE              -31  /* Broken neural pipe */

/* File descriptors - Neural channels */
#define STDIN_FILENO        0   /* Standard input neural channel */
#define STDOUT_FILENO       1   /* Standard output neural channel */
#define STDERR_FILENO       2   /* Standard error neural channel */

/* File access modes */
#define O_RDONLY            0x0000  /* Read-only neural access */
#define O_WRONLY            0x0001  /* Write-only neural access */
#define O_RDWR              0x0002  /* Read-write neural access */
#define O_CREAT             0x0040  /* Create neural file */
#define O_EXCL              0x0080  /* Exclusive neural creation */
#define O_NOCTTY            0x0100  /* No controlling neural terminal */
#define O_TRUNC             0x0200  /* Truncate neural file */
#define O_APPEND            0x0400  /* Append to neural file */
#define O_NONBLOCK          0x0800  /* Non-blocking neural I/O */

/* Process execution flags */
#define EXEC_REPLACE        0x01    /* Replace current neural process */
#define EXEC_NEW_SPACE      0x02    /* Create new neural address space */
#define EXEC_INHERIT_FDS    0x04    /* Inherit neural file descriptors */

/* Memory mapping flags */
#define MMAP_SHARED         0x01    /* Shared neural memory */
#define MMAP_PRIVATE        0x02    /* Private neural memory */
#define MMAP_FIXED          0x10    /* Fixed neural address */
#define MMAP_ANONYMOUS      0x20    /* Anonymous neural memory */

/* Memory protection flags */
#define PROT_NONE           0x0     /* No neural access */
#define PROT_READ           0x1     /* Neural read access */
#define PROT_WRITE          0x2     /* Neural write access */
#define PROT_EXEC           0x4     /* Neural execute access */

/* Signal numbers */
#define SIGHUP              1       /* Neural hangup */
#define SIGINT              2       /* Neural interrupt */
#define SIGQUIT             3       /* Neural quit */
#define SIGILL              4       /* Illegal neural instruction */
#define SIGTRAP             5       /* Neural trace/breakpoint */
#define SIGABRT             6       /* Neural abort */
#define SIGFPE              8       /* Neural floating point exception */
#define SIGKILL             9       /* Neural kill (uncatchable) */
#define SIGSEGV             11      /* Neural segmentation violation */
#define SIGPIPE             13      /* Broken neural pipe */
#define SIGALRM             14      /* Neural alarm clock */
#define SIGTERM             15      /* Neural termination */
#define SIGUSR1             30      /* User-defined neural signal 1 */
#define SIGUSR2             31      /* User-defined neural signal 2 */

/* System call parameter structure */
struct syscall_params {
    uint64_t syscall_num;   /* System call number */
    uint64_t arg0;          /* First argument */
    uint64_t arg1;          /* Second argument */
    uint64_t arg2;          /* Third argument */
    uint64_t arg3;          /* Fourth argument */
    uint64_t arg4;          /* Fifth argument */
    uint64_t arg5;          /* Sixth argument */
} __attribute__((packed));

/* System call result structure */
struct syscall_result {
    int64_t return_value;   /* Return value */
    int32_t error_code;     /* Error code */
} __attribute__((packed));

/* File status structure */
struct file_stat {
    uint64_t size;          /* File size in bytes */
    uint64_t blocks;        /* Number of blocks */
    uint32_t mode;          /* File mode and permissions */
    uint32_t uid;           /* User ID */
    uint32_t gid;           /* Group ID */
    uint64_t atime;         /* Last access time */
    uint64_t mtime;         /* Last modification time */
    uint64_t ctime;         /* Last status change time */
};

/* Brandon Media OS - System Call Functions */

/* Core system call interface */
void syscalls_init(void);
int64_t syscall_handler(struct syscall_params *params);
void syscall_return_to_user(int64_t return_value, int32_t error_code);

/* System call implementations */
int64_t sys_exit(int32_t status);
int64_t sys_read(int32_t fd, void *buffer, size_t count);
int64_t sys_write(int32_t fd, const void *buffer, size_t count);
int64_t sys_open(const char *pathname, int32_t flags, uint32_t mode);
int64_t sys_close(int32_t fd);
int64_t sys_fork(void);
int64_t sys_exec(const char *pathname, char *const argv[], char *const envp[]);
int64_t sys_wait(int32_t *status);
int64_t sys_kill(uint32_t pid, int32_t signal);
int64_t sys_sleep(uint32_t milliseconds);
int64_t sys_yield(void);
int64_t sys_getpid(void);
int64_t sys_getppid(void);
int64_t sys_mmap(void *addr, size_t length, int32_t prot, int32_t flags, int32_t fd, uint64_t offset);
int64_t sys_munmap(void *addr, size_t length);
int64_t sys_brk(void *addr);
int64_t sys_pipe(int32_t pipefd[2]);

/* User mode support */
void enter_user_mode(uint64_t entry_point, uint64_t stack_pointer);
void setup_user_stack(struct process *proc, int argc, char *argv[], char *envp[]);
int load_elf_executable(const char *path, struct process *proc);

/* Error handling */
const char *syscall_strerror(int32_t error_code);
void syscall_set_errno(int32_t error_code);
int32_t syscall_get_errno(void);

/* System call macros for user space */
#define SYSCALL_0(num) \
    ({ \
        int64_t ret; \
        asm volatile("syscall" : "=a"(ret) : "a"(num) : "rcx", "r11", "memory"); \
        ret; \
    })

#define SYSCALL_1(num, arg1) \
    ({ \
        int64_t ret; \
        asm volatile("syscall" : "=a"(ret) : "a"(num), "D"(arg1) : "rcx", "r11", "memory"); \
        ret; \
    })

#define SYSCALL_2(num, arg1, arg2) \
    ({ \
        int64_t ret; \
        asm volatile("syscall" : "=a"(ret) : "a"(num), "D"(arg1), "S"(arg2) : "rcx", "r11", "memory"); \
        ret; \
    })

#define SYSCALL_3(num, arg1, arg2, arg3) \
    ({ \
        int64_t ret; \
        asm volatile("syscall" : "=a"(ret) : "a"(num), "D"(arg1), "S"(arg2), "d"(arg3) : "rcx", "r11", "memory"); \
        ret; \
    })

#endif /* _SYSCALLS_H */