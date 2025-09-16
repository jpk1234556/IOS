/* input.h - Brandon Media OS Neural Input System
 * Multi-Modal Input Handling (Mouse, Keyboard, Touch)
 */

#ifndef KERNEL_INPUT_H
#define KERNEL_INPUT_H

#include <stdint.h>
#include <stdbool.h>

/* Input Device Types */
typedef enum {
    INPUT_DEVICE_KEYBOARD,
    INPUT_DEVICE_MOUSE,
    INPUT_DEVICE_TOUCHSCREEN,
    INPUT_DEVICE_GAMEPAD,
    INPUT_DEVICE_STYLUS
} input_device_type_t;

/* Mouse Button Definitions */
typedef enum {
    MOUSE_BUTTON_LEFT = 0,
    MOUSE_BUTTON_RIGHT = 1,
    MOUSE_BUTTON_MIDDLE = 2,
    MOUSE_BUTTON_X1 = 3,
    MOUSE_BUTTON_X2 = 4
} mouse_button_t;

/* Key Codes (Subset) */
typedef enum {
    KEY_ESCAPE = 1,
    KEY_TAB = 15,
    KEY_ENTER = 28,
    KEY_SPACE = 57,
    KEY_LEFT_ARROW = 75,
    KEY_RIGHT_ARROW = 77,
    KEY_UP_ARROW = 72,
    KEY_DOWN_ARROW = 80,
    KEY_F1 = 59,
    KEY_F2 = 60,
    KEY_F3 = 61,
    KEY_F4 = 62
} key_code_t;

/* Input Event Types */
typedef enum {
    INPUT_EVENT_KEY_PRESS,
    INPUT_EVENT_KEY_RELEASE,
    INPUT_EVENT_MOUSE_MOVE,
    INPUT_EVENT_MOUSE_PRESS,
    INPUT_EVENT_MOUSE_RELEASE,
    INPUT_EVENT_MOUSE_WHEEL,
    INPUT_EVENT_TOUCH_START,
    INPUT_EVENT_TOUCH_MOVE,
    INPUT_EVENT_TOUCH_END
} input_event_type_t;

/* Input Event Structure */
typedef struct {
    input_event_type_t type;
    uint32_t timestamp;
    
    union {
        /* Keyboard Events */
        struct {
            key_code_t key;
            bool ctrl;
            bool alt;
            bool shift;
        } key;
        
        /* Mouse Events */
        struct {
            int32_t x, y;
            int32_t delta_x, delta_y;
            mouse_button_t button;
            int32_t wheel_delta;
        } mouse;
        
        /* Touch Events */
        struct {
            uint32_t touch_id;
            int32_t x, y;
            float pressure;
        } touch;
    } data;
} input_event_t;

/* Input Event Handler Function */
typedef void (*input_event_handler_t)(input_event_t *event);

/* Input Device Structure */
typedef struct input_device {
    input_device_type_t type;
    char name[64];
    bool connected;
    void *device_data;
    
    /* Device-specific functions */
    int (*init)(struct input_device *device);
    void (*update)(struct input_device *device);
    void (*cleanup)(struct input_device *device);
    
    struct input_device *next;
} input_device_t;

/* Mouse State */
typedef struct {
    int32_t x, y;
    int32_t delta_x, delta_y;
    bool buttons[5];         /* Support for 5 mouse buttons */
    int32_t wheel_delta;
    bool moved;
} mouse_state_t;

/* Keyboard State */
typedef struct {
    bool keys[256];          /* Key state array */
    bool ctrl, alt, shift;   /* Modifier keys */
    uint8_t last_scancode;
} keyboard_state_t;

/* Touch Point */
typedef struct {
    uint32_t id;
    int32_t x, y;
    float pressure;
    bool active;
} touch_point_t;

/* Touch State */
typedef struct {
    touch_point_t points[10]; /* Support for 10 touch points */
    uint32_t active_count;
} touch_state_t;

/* Input System State */
typedef struct {
    /* Device List */
    input_device_t *devices;
    uint32_t device_count;
    
    /* Current State */
    mouse_state_t mouse;
    keyboard_state_t keyboard;
    touch_state_t touch;
    
    /* Event Queue */
    input_event_t event_queue[256];
    uint32_t queue_head;
    uint32_t queue_tail;
    uint32_t queue_count;
    
    /* Event Handlers */
    input_event_handler_t handlers[16];
    uint32_t handler_count;
    
    /* Settings */
    float mouse_sensitivity;
    bool mouse_acceleration;
    uint32_t double_click_time;
    uint32_t double_click_distance;
    
    /* State */
    bool initialized;
    uint32_t last_update_time;
} input_system_t;

/* Core Input Functions */
int input_init(void);
void input_shutdown(void);
void input_update(void);

/* Device Management */
int input_register_device(input_device_t *device);
void input_unregister_device(input_device_t *device);
input_device_t *input_find_device(input_device_type_t type);

/* Event System */
void input_add_event_handler(input_event_handler_t handler);
void input_remove_event_handler(input_event_handler_t handler);
void input_queue_event(input_event_t *event);
input_event_t *input_get_next_event(void);
void input_process_events(void);

/* Mouse Functions */
void input_get_mouse_position(int32_t *x, int32_t *y);
bool input_is_mouse_button_pressed(mouse_button_t button);
bool input_was_mouse_button_clicked(mouse_button_t button);
void input_set_mouse_position(int32_t x, int32_t y);
void input_set_mouse_sensitivity(float sensitivity);

/* Keyboard Functions */
bool input_is_key_pressed(key_code_t key);
bool input_was_key_pressed(key_code_t key);
bool input_is_modifier_pressed(bool *ctrl, bool *alt, bool *shift);

/* Touch Functions */
uint32_t input_get_active_touches(void);
bool input_get_touch_point(uint32_t index, int32_t *x, int32_t *y, float *pressure);

/* Hardware Interrupt Handlers */
void keyboard_interrupt_handler(void);
void mouse_interrupt_handler(void);

/* PS/2 Keyboard Driver */
int ps2_keyboard_init(void);
void ps2_keyboard_handler(void);

/* PS/2 Mouse Driver */
int ps2_mouse_init(void);
void ps2_mouse_handler(void);

/* USB Input Device Support (Future) */
int usb_input_init(void);

/* Utility Functions */
uint32_t input_get_time_ms(void);
bool input_point_in_bounds(int32_t x, int32_t y, int32_t bx, int32_t by, int32_t bw, int32_t bh);

#endif /* KERNEL_INPUT_H */