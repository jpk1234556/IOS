/* ipc.c - Brandon Media OS Inter-Process Communication */
#include <stdint.h>
#include "kernel/process.h"
#include "kernel/memory.h"

/* External functions */
extern void serial_puts(const char *s);
extern void print_dec(uint64_t num);
extern void *kmalloc(size_t size);
extern void kfree(void *ptr);
extern void memory_copy(void *dst, const void *src, size_t size);
extern struct process *process_get_by_pid(uint32_t pid);
extern struct process *process_get_current(void);

/* Simple message structure */
struct ipc_message {
    uint32_t sender_pid;
    uint32_t size;
    uint8_t data[256];  /* Max message size */
    struct ipc_message *next;
};

/* Message queue structure */
struct message_queue {
    struct ipc_message *head;
    struct ipc_message *tail;
    uint32_t count;
};

/* Send message to process */
int ipc_send_message(uint32_t dest_pid, const void *data, size_t size) {
    if (size > 256) {
        serial_puts("[IPC] Message too large - neural bandwidth exceeded\\n");
        return -1;
    }
    
    struct process *dest = process_get_by_pid(dest_pid);
    if (!dest) {
        serial_puts("[IPC] Destination process not found in matrix\\n");
        return -1;
    }
    
    struct process *sender = process_get_current();
    if (!sender) {
        serial_puts("[IPC] No current process for message transmission\\n");
        return -1;
    }
    
    serial_puts("[IPC] Neural message transmission: ");
    print_dec(sender->pid);
    serial_puts(" -> ");
    print_dec(dest_pid);
    serial_puts("\\n");
    
    /* Allocate message */
    struct ipc_message *msg = (struct ipc_message *)kmalloc(sizeof(struct ipc_message));
    if (!msg) {
        serial_puts("[ERROR] Message allocation failed\\n");
        return -1;
    }
    
    /* Initialize message */
    msg->sender_pid = sender->pid;
    msg->size = size;
    msg->next = NULL;
    memory_copy(msg->data, data, size);
    
    /* Create message queue if needed */
    if (!dest->message_queue) {
        dest->message_queue = kmalloc(sizeof(struct message_queue));
        if (!dest->message_queue) {
            kfree(msg);
            return -1;
        }
        struct message_queue *queue = (struct message_queue *)dest->message_queue;
        queue->head = NULL;
        queue->tail = NULL;
        queue->count = 0;
    }
    
    /* Add message to queue */
    struct message_queue *queue = (struct message_queue *)dest->message_queue;
    if (queue->tail) {
        queue->tail->next = msg;
    } else {
        queue->head = msg;
    }
    queue->tail = msg;
    queue->count++;
    
    /* Wake up destination if blocked */
    if (dest->state == PROCESS_BLOCKED) {
        dest->state = PROCESS_READY;
        scheduler_add_process(dest);
    }
    
    serial_puts("[SUCCESS] Neural message transmitted\\n");
    return 0;
}

/* Receive message */
int ipc_receive_message(void *buffer, size_t max_size) {
    struct process *current = process_get_current();
    if (!current || !current->message_queue) {
        return -1;  /* No messages */
    }
    
    struct message_queue *queue = (struct message_queue *)current->message_queue;
    if (!queue->head) {
        return -1;  /* No messages */
    }
    
    /* Get first message */
    struct ipc_message *msg = queue->head;
    queue->head = msg->next;
    if (!queue->head) {
        queue->tail = NULL;
    }
    queue->count--;
    
    /* Copy message data */
    size_t copy_size = (msg->size < max_size) ? msg->size : max_size;
    memory_copy(buffer, msg->data, copy_size);
    
    serial_puts("[IPC] Neural message received from PID: ");
    print_dec(msg->sender_pid);
    serial_puts("\\n");
    
    /* Free message */
    kfree(msg);
    
    return copy_size;
}

/* Create pipe (simplified implementation) */
int ipc_create_pipe(uint32_t *read_fd, uint32_t *write_fd) {
    serial_puts("[IPC] Creating neural communication pipe\\n");
    
    /* Simple implementation - just return fake file descriptors */
    static uint32_t next_fd = 3;  /* 0, 1, 2 reserved for stdin, stdout, stderr */
    
    *read_fd = next_fd++;
    *write_fd = next_fd++;
    
    serial_puts("[SUCCESS] Neural pipe created - Read FD: ");
    print_dec(*read_fd);
    serial_puts(", Write FD: ");
    print_dec(*write_fd);
    serial_puts("\\n");
    
    return 0;
}