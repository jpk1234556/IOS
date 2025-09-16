/* input.c - Brandon Media OS Neural Input System Implementation
 * Multi-Modal Input Handling with Hardware Support
 */

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include "kernel/input.h"
#include "kernel/interrupts.h"
#include "kernel/memory.h"

/* External functions */
extern void serial_puts(const char *s);
extern void print_dec(uint64_t num);
extern uint32_t get_time_ms(void);
extern void outb(uint16_t port, uint8_t val);
extern uint8_t inb(uint16_t port);

/* Global input system state */
static input_system_t input_system;
static bool input_initialized = false;

/* PS/2 Controller Ports */
#define PS2_DATA_PORT    0x60
#define PS2_STATUS_PORT  0x64
#define PS2_COMMAND_PORT 0x64

/* PS/2 Controller Commands */
#define PS2_CMD_READ_CONFIG     0x20
#define PS2_CMD_WRITE_CONFIG    0x60
#define PS2_CMD_DISABLE_PORT2   0xA7
#define PS2_CMD_ENABLE_PORT2    0xA8
#define PS2_CMD_TEST_PORT2      0xA9
#define PS2_CMD_TEST_CONTROLLER 0xAA
#define PS2_CMD_TEST_PORT1      0xAB
#define PS2_CMD_DISABLE_PORT1   0xAD
#define PS2_CMD_ENABLE_PORT1    0xAE

/* Keyboard scan codes (basic set) */
static const key_code_t scancode_map[256] = {
    [0x01] = KEY_ESCAPE,
    [0x0F] = KEY_TAB,
    [0x1C] = KEY_ENTER,
    [0x39] = KEY_SPACE,
    [0x4B] = KEY_LEFT_ARROW,
    [0x4D] = KEY_RIGHT_ARROW,
    [0x48] = KEY_UP_ARROW,
    [0x50] = KEY_DOWN_ARROW,
    [0x3B] = KEY_F1,
    [0x3C] = KEY_F2,
    [0x3D] = KEY_F3,
    [0x3E] = KEY_F4,
    /* Add more mappings as needed */
};

/* Initialize Input System */
int input_init(void) {
    if (input_initialized) {
        return 0;
    }
    
    serial_puts("[NEURAL-INPUT] Initializing Neural Input System...\n");
    
    /* Clear input system state */
    memset(&input_system, 0, sizeof(input_system_t));
    
    /* Initialize settings */
    input_system.mouse_sensitivity = 1.0f;
    input_system.mouse_acceleration = false;
    input_system.double_click_time = 300; /* 300ms */
    input_system.double_click_distance = 5; /* 5 pixels */
    
    /* Initialize PS/2 controller */
    if (ps2_init() != 0) {
        serial_puts("[NEURAL-INPUT] Failed to initialize PS/2 controller\n");
        return -1;
    }
    
    /* Initialize keyboard */
    if (ps2_keyboard_init() != 0) {
        serial_puts("[NEURAL-INPUT] Failed to initialize keyboard\n");
    } else {
        serial_puts("[SUCCESS] PS/2 keyboard initialized\n");
    }
    
    /* Initialize mouse */
    if (ps2_mouse_init() != 0) {
        serial_puts("[NEURAL-INPUT] Failed to initialize mouse\n");
    } else {
        serial_puts("[SUCCESS] PS/2 mouse initialized\n");
    }
    
    input_system.initialized = true;
    input_initialized = true;
    
    serial_puts("[NEURAL-INPUT] Neural Input System initialized\n");
    return 0;
}

/* Shutdown Input System */
void input_shutdown(void) {
    if (!input_initialized) {
        return;
    }
    
    serial_puts("[NEURAL-INPUT] Shutting down Neural Input System...\n");
    
    /* Cleanup devices */
    input_device_t *device = input_system.devices;
    while (device) {
        input_device_t *next = device->next;
        if (device->cleanup) {
            device->cleanup(device);
        }
        kfree(device);
        device = next;
    }
    
    input_system.initialized = false;
    input_initialized = false;
    
    serial_puts("[NEURAL-INPUT] Neural Input System shutdown complete\n");
}

/* Update Input System */
void input_update(void) {
    if (!input_initialized) {
        return;
    }
    
    input_system.last_update_time = get_time_ms();
    
    /* Update all devices */
    input_device_t *device = input_system.devices;
    while (device) {
        if (device->connected && device->update) {
            device->update(device);
        }
        device = device->next;
    }
    
    /* Process queued events */
    input_process_events();
}

/* Register Input Device */
int input_register_device(input_device_t *device) {
    if (!input_initialized || !device) {
        return -1;
    }
    
    /* Add to device list */
    device->next = input_system.devices;
    input_system.devices = device;
    input_system.device_count++;
    
    /* Initialize device */
    if (device->init) {
        return device->init(device);
    }
    
    return 0;
}

/* Find Device by Type */
input_device_t *input_find_device(input_device_type_t type) {
    if (!input_initialized) {
        return NULL;
    }
    
    input_device_t *device = input_system.devices;
    while (device) {
        if (device->type == type && device->connected) {
            return device;
        }
        device = device->next;
    }
    
    return NULL;
}

/* Queue Input Event */
void input_queue_event(input_event_t *event) {
    if (!input_initialized || !event) {
        return;
    }
    
    /* Check if queue is full */
    if (input_system.queue_count >= 256) {
        /* Drop oldest event */
        input_system.queue_head = (input_system.queue_head + 1) % 256;
        input_system.queue_count--;
    }
    
    /* Add event to queue */
    input_system.event_queue[input_system.queue_tail] = *event;
    input_system.queue_tail = (input_system.queue_tail + 1) % 256;
    input_system.queue_count++;
}

/* Get Next Event */
input_event_t *input_get_next_event(void) {
    if (!input_initialized || input_system.queue_count == 0) {
        return NULL;
    }
    
    input_event_t *event = &input_system.event_queue[input_system.queue_head];
    input_system.queue_head = (input_system.queue_head + 1) % 256;
    input_system.queue_count--;
    
    return event;
}

/* Process Events */
void input_process_events(void) {
    input_event_t *event;
    while ((event = input_get_next_event()) != NULL) {
        /* Call event handlers */
        for (uint32_t i = 0; i < input_system.handler_count; i++) {
            if (input_system.handlers[i]) {
                input_system.handlers[i](event);
            }
        }
        
        /* Update input state based on event */
        switch (event->type) {
            case INPUT_EVENT_KEY_PRESS:
                input_system.keyboard.keys[event->data.key.key] = true;
                input_system.keyboard.ctrl = event->data.key.ctrl;
                input_system.keyboard.alt = event->data.key.alt;
                input_system.keyboard.shift = event->data.key.shift;
                break;
                
            case INPUT_EVENT_KEY_RELEASE:
                input_system.keyboard.keys[event->data.key.key] = false;
                break;
                
            case INPUT_EVENT_MOUSE_MOVE:
                input_system.mouse.delta_x = event->data.mouse.delta_x;
                input_system.mouse.delta_y = event->data.mouse.delta_y;
                input_system.mouse.x = event->data.mouse.x;
                input_system.mouse.y = event->data.mouse.y;
                input_system.mouse.moved = true;
                break;
                
            case INPUT_EVENT_MOUSE_PRESS:
                input_system.mouse.buttons[event->data.mouse.button] = true;
                break;
                
            case INPUT_EVENT_MOUSE_RELEASE:
                input_system.mouse.buttons[event->data.mouse.button] = false;
                break;
                
            case INPUT_EVENT_MOUSE_WHEEL:
                input_system.mouse.wheel_delta = event->data.mouse.wheel_delta;
                break;
                
            default:
                break;
        }
    }
}

/* Add Event Handler */
void input_add_event_handler(input_event_handler_t handler) {
    if (!input_initialized || !handler || input_system.handler_count >= 16) {
        return;
    }
    
    input_system.handlers[input_system.handler_count++] = handler;
}

/* Mouse Functions */
void input_get_mouse_position(int32_t *x, int32_t *y) {
    if (!input_initialized) {
        return;
    }
    
    if (x) *x = input_system.mouse.x;
    if (y) *y = input_system.mouse.y;
}

bool input_is_mouse_button_pressed(mouse_button_t button) {
    if (!input_initialized || button >= 5) {
        return false;
    }
    
    return input_system.mouse.buttons[button];
}

bool input_is_key_pressed(key_code_t key) {
    if (!input_initialized || key >= 256) {
        return false;
    }
    
    return input_system.keyboard.keys[key];
}

/* PS/2 Initialization */
static int ps2_init(void) {
    serial_puts("[PS2] Initializing PS/2 controller...\n");
    
    /* Disable devices */
    outb(PS2_COMMAND_PORT, PS2_CMD_DISABLE_PORT1);
    outb(PS2_COMMAND_PORT, PS2_CMD_DISABLE_PORT2);
    
    /* Flush output buffer */
    inb(PS2_DATA_PORT);
    
    /* Test controller */
    outb(PS2_COMMAND_PORT, PS2_CMD_TEST_CONTROLLER);
    uint8_t response = inb(PS2_DATA_PORT);
    if (response != 0x55) {
        serial_puts("[ERROR] PS/2 controller self-test failed\n");
        return -1;
    }
    
    /* Enable first port */
    outb(PS2_COMMAND_PORT, PS2_CMD_ENABLE_PORT1);
    
    serial_puts("[SUCCESS] PS/2 controller initialized\n");
    return 0;
}

/* PS/2 Keyboard Initialization */
int ps2_keyboard_init(void) {
    /* Create keyboard device */
    input_device_t *kbd_device = (input_device_t *)kmalloc(sizeof(input_device_t));
    if (!kbd_device) {
        return -1;
    }
    
    memset(kbd_device, 0, sizeof(input_device_t));
    kbd_device->type = INPUT_DEVICE_KEYBOARD;
    strcpy(kbd_device->name, "PS/2 Keyboard");
    kbd_device->connected = true;
    
    /* Register device */
    if (input_register_device(kbd_device) != 0) {
        kfree(kbd_device);
        return -1;
    }
    
    /* Register keyboard interrupt handler */
    extern void register_interrupt_handler(uint8_t irq, void (*handler)(void));
    register_interrupt_handler(1, keyboard_interrupt_handler);
    
    return 0;
}

/* PS/2 Mouse Initialization */
int ps2_mouse_init(void) {
    /* Create mouse device */
    input_device_t *mouse_device = (input_device_t *)kmalloc(sizeof(input_device_t));
    if (!mouse_device) {
        return -1;
    }
    
    memset(mouse_device, 0, sizeof(input_device_t));
    mouse_device->type = INPUT_DEVICE_MOUSE;
    strcpy(mouse_device->name, "PS/2 Mouse");
    mouse_device->connected = true;
    
    /* Register device */
    if (input_register_device(mouse_device) != 0) {
        kfree(mouse_device);
        return -1;
    }
    
    /* Register mouse interrupt handler */
    extern void register_interrupt_handler(uint8_t irq, void (*handler)(void));
    register_interrupt_handler(12, mouse_interrupt_handler);
    
    return 0;
}

/* Keyboard Interrupt Handler */
void keyboard_interrupt_handler(void) {
    uint8_t scancode = inb(PS2_DATA_PORT);
    
    /* Determine if key press or release */
    bool key_released = (scancode & 0x80) != 0;
    if (key_released) {
        scancode &= 0x7F; /* Remove release flag */
    }
    
    /* Look up key code */
    key_code_t key = scancode_map[scancode];
    if (key == 0) {
        return; /* Unknown key */
    }
    
    /* Create input event */
    input_event_t event;
    event.type = key_released ? INPUT_EVENT_KEY_RELEASE : INPUT_EVENT_KEY_PRESS;
    event.timestamp = get_time_ms();
    event.data.key.key = key;
    event.data.key.ctrl = input_system.keyboard.ctrl;
    event.data.key.alt = input_system.keyboard.alt;
    event.data.key.shift = input_system.keyboard.shift;
    
    /* Queue event */
    input_queue_event(&event);
}

/* Mouse Interrupt Handler */
void mouse_interrupt_handler(void) {
    static uint8_t mouse_packet[3];
    static int packet_index = 0;
    
    uint8_t data = inb(PS2_DATA_PORT);
    mouse_packet[packet_index++] = data;
    
    /* Wait for complete packet (3 bytes) */
    if (packet_index < 3) {
        return;
    }
    
    packet_index = 0;
    
    /* Parse mouse packet */
    uint8_t flags = mouse_packet[0];
    int8_t delta_x = (int8_t)mouse_packet[1];
    int8_t delta_y = (int8_t)mouse_packet[2];
    
    /* Apply mouse sensitivity */
    delta_x = (int8_t)(delta_x * input_system.mouse_sensitivity);
    delta_y = (int8_t)(delta_y * input_system.mouse_sensitivity);
    
    /* Update mouse position */
    input_system.mouse.x += delta_x;
    input_system.mouse.y -= delta_y; /* Invert Y axis */
    
    /* Clamp to screen bounds */
    if (input_system.mouse.x < 0) input_system.mouse.x = 0;
    if (input_system.mouse.y < 0) input_system.mouse.y = 0;
    
    /* Create move event */
    input_event_t move_event;
    move_event.type = INPUT_EVENT_MOUSE_MOVE;
    move_event.timestamp = get_time_ms();
    move_event.data.mouse.x = input_system.mouse.x;
    move_event.data.mouse.y = input_system.mouse.y;
    move_event.data.mouse.delta_x = delta_x;
    move_event.data.mouse.delta_y = delta_y;
    
    input_queue_event(&move_event);
    
    /* Handle button events */
    for (int i = 0; i < 3; i++) {
        bool button_pressed = (flags & (1 << i)) != 0;
        bool was_pressed = input_system.mouse.buttons[i];
        
        if (button_pressed != was_pressed) {
            input_event_t button_event;
            button_event.type = button_pressed ? INPUT_EVENT_MOUSE_PRESS : INPUT_EVENT_MOUSE_RELEASE;
            button_event.timestamp = get_time_ms();
            button_event.data.mouse.x = input_system.mouse.x;
            button_event.data.mouse.y = input_system.mouse.y;
            button_event.data.mouse.button = (mouse_button_t)i;
            
            input_queue_event(&button_event);
        }
    }
}

/* Get Current Time (placeholder) */
uint32_t input_get_time_ms(void) {
    return get_time_ms();
}

/* Test Input System */
void input_test(void) {
    if (!input_initialized) {
        serial_puts("[NEURAL-INPUT] Input system not initialized\n");
        return;
    }
    
    serial_puts("[NEURAL-INPUT] Testing Neural Input System...\n");
    
    /* Display current input state */
    serial_puts("[INFO] Mouse position: ");
    print_dec(input_system.mouse.x);
    serial_puts(", ");
    print_dec(input_system.mouse.y);
    serial_puts("\n");
    
    serial_puts("[INFO] Mouse buttons: ");
    for (int i = 0; i < 3; i++) {
        serial_puts(input_system.mouse.buttons[i] ? "1" : "0");
    }
    serial_puts("\n");
    
    serial_puts("[INFO] Device count: ");
    print_dec(input_system.device_count);
    serial_puts("\n");
    
    serial_puts("[NEURAL-INPUT] Input test completed\n");
}