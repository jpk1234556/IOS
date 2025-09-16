# Brandon Media OS - API Reference
## Neural SCADA 3D Parallax GUI System API

---

## 🎯 **Core GUI API**

### **System Management**

#### `gui_init()`
```c
int gui_init(void);
```
**Description**: Initializes the Neural GUI system with cyberpunk theme  
**Returns**: 0 on success, -1 on failure  
**Usage**:
```c
if (gui_init() == 0) {
    serial_puts("[SUCCESS] Neural GUI System initialized\n");
}
```

#### `gui_shutdown()`
```c
void gui_shutdown(void);
```
**Description**: Cleanly shuts down GUI system and frees all resources  
**Usage**: Call before kernel shutdown

#### `gui_update()`
```c
void gui_update(uint32_t delta_ms);
```
**Description**: Updates GUI system, animations, and input handling  
**Parameters**:
- `delta_ms`: Time elapsed since last update in milliseconds

#### `gui_render()`
```c
void gui_render(void);
```
**Description**: Renders all visible GUI layers with 3D parallax effects

---

## 🎮 **Widget Creation API**

### **Button Widgets**

#### `gui_create_button()`
```c
gui_widget_t *gui_create_button(const char *name, rect_t bounds, 
                               const char *text, widget_click_handler_t callback);
```
**Description**: Creates an interactive button with cyberpunk styling  
**Parameters**:
- `name`: Unique widget identifier
- `bounds`: Position and size rectangle
- `text`: Button label text
- `callback`: Function called when clicked

**Example**:
```c
gui_widget_t *start_btn = gui_create_button("start", 
    (rect_t){100, 200, 80, 35}, 
    "START", 
    on_start_clicked);
gui_add_widget(start_btn);
```

### **SCADA Gauges**

#### `gui_create_scada_gauge()`
```c
gui_widget_t *gui_create_scada_gauge(const char *name, rect_t bounds,
                                    float min_val, float max_val, const char *unit);
```
**Description**: Creates a circular analog gauge for SCADA monitoring  
**Parameters**:
- `name`: Unique widget identifier
- `bounds`: Position and size rectangle  
- `min_val`: Minimum scale value
- `max_val`: Maximum scale value
- `unit`: Measurement unit string (e.g., "°C", "PSI")

**Example**:
```c
gui_widget_t *temp_gauge = gui_create_scada_gauge("temperature",
    (rect_t){50, 50, 120, 120},
    0.0f, 100.0f, "°C");
gui_add_widget(temp_gauge);
```

#### `gui_update_gauge_value()`
```c
void gui_update_gauge_value(gui_widget_t *gauge, float value);
```
**Description**: Updates gauge needle position with smooth animation  
**Parameters**:
- `gauge`: Gauge widget pointer
- `value`: New value (will be clamped to min/max range)

#### `gui_set_gauge_alarm()`
```c
void gui_set_gauge_alarm(gui_widget_t *gauge, bool critical);
```
**Description**: Sets gauge alarm state (changes needle color)  
**Parameters**:
- `gauge`: Gauge widget pointer
- `critical`: true for red alarm, false for normal green

### **Neural Matrix**

#### `gui_create_neural_matrix()`
```c
gui_widget_t *gui_create_neural_matrix(const char *name, rect_t bounds,
                                      uint32_t width, uint32_t height);
```
**Description**: Creates animated neural matrix background effect  
**Parameters**:
- `name`: Unique widget identifier
- `bounds`: Position and size rectangle
- `width`: Matrix grid width in cells
- `height`: Matrix grid height in cells

### **Progress Bars**

#### `gui_create_progress_bar()`
```c
gui_widget_t *gui_create_progress_bar(const char *name, rect_t bounds, const char *label);
```
**Description**: Creates progress bar with percentage display  
**Parameters**:
- `name`: Unique widget identifier
- `bounds`: Position and size rectangle
- `label`: Progress bar label text

#### `gui_set_progress_value()`
```c
void gui_set_progress_value(gui_widget_t *progress_bar, float value);
```
**Description**: Sets progress value with smooth animation  
**Parameters**:
- `progress_bar`: Progress bar widget pointer
- `value`: Progress value (0.0 to 1.0)

### **Panels**

#### `gui_create_panel()`
```c
gui_widget_t *gui_create_panel(const char *name, rect_t bounds, color_rgba_t bg_color);
```
**Description**: Creates container panel with optional border  
**Parameters**:
- `name`: Unique widget identifier
- `bounds`: Position and size rectangle
- `bg_color`: Background color with alpha

### **Labels**

#### `gui_create_label()`
```c
gui_widget_t *gui_create_label(const char *name, rect_t bounds, const char *text);
```
**Description**: Creates text label widget  
**Parameters**:
- `name`: Unique widget identifier
- `bounds`: Position and size rectangle
- `text`: Label text content

---

## 🎭 **Animation API**

### **Animation Control**

#### `gui_start_animation()`
```c
void gui_start_animation(gui_widget_t *widget, animation_type_t type, 
                        uint32_t duration_ms, bool loop);
```
**Description**: Starts animation on widget  
**Parameters**:
- `widget`: Target widget pointer
- `type`: Animation type (ANIM_FADE, ANIM_GLOW, etc.)
- `duration_ms`: Animation duration in milliseconds
- `loop`: true for looping animation

**Animation Types**:
```c
typedef enum {
    ANIM_FADE,          // Alpha fade in/out
    ANIM_SLIDE,         // Position sliding
    ANIM_SCALE,         // Size scaling
    ANIM_GLOW,          // Neural glow effect
    ANIM_NEURAL_PULSE,  // Pulsing border
    ANIM_MATRIX_FLOW    // Matrix data flow
} animation_type_t;
```

#### `gui_stop_animation()`
```c
void gui_stop_animation(gui_widget_t *widget);
```
**Description**: Stops active animation on widget

### **Particle System**

#### `neural_particle_system_init()`
```c
void neural_particle_system_init(void);
```
**Description**: Initializes neural particle system for effects

#### `neural_spawn_particle()`
```c
void neural_spawn_particle(float x, float y, float z, uint32_t color);
```
**Description**: Spawns new particle at specified position

---

## 🎲 **3D Graphics API**

### **System Initialization**

#### `graphics_3d_init()`
```c
int graphics_3d_init(uint32_t width, uint32_t height, uint32_t *framebuffer);
```
**Description**: Initializes 3D graphics engine  
**Parameters**:
- `width`: Framebuffer width in pixels
- `height`: Framebuffer height in pixels
- `framebuffer`: Pointer to framebuffer memory

#### `graphics_3d_clear()`
```c
void graphics_3d_clear(uint32_t color);
```
**Description**: Clears framebuffer and Z-buffer

### **Vector Math**

#### `vec3_add()`
```c
vec3_t vec3_add(vec3_t a, vec3_t b);
```
**Description**: Adds two 3D vectors  
**Returns**: Sum vector

#### `vec3_normalize()`
```c
vec3_t vec3_normalize(vec3_t v);
```
**Description**: Normalizes vector to unit length

#### `vec3_cross()`
```c
vec3_t vec3_cross(vec3_t a, vec3_t b);
```
**Description**: Calculates cross product of two vectors

### **Matrix Operations**

#### `matrix4_multiply()`
```c
matrix4_t matrix4_multiply(matrix4_t a, matrix4_t b);
```
**Description**: Multiplies two 4x4 matrices

#### `matrix4_perspective()`
```c
matrix4_t matrix4_perspective(float fov, float aspect, float near_plane, float far_plane);
```
**Description**: Creates perspective projection matrix

### **Mesh Operations**

#### `mesh_create_neural_grid()`
```c
mesh_3d_t *mesh_create_neural_grid(uint32_t width, uint32_t height);
```
**Description**: Creates neural grid mesh for matrix effects

---

## 🖱️ **Input System API**

### **System Management**

#### `input_init()`
```c
int input_init(void);
```
**Description**: Initializes input system with PS/2 hardware support

#### `input_update()`
```c
void input_update(void);
```
**Description**: Updates input state and processes events

### **Event Handling**

#### `input_add_event_handler()`
```c
void input_add_event_handler(input_event_handler_t handler);
```
**Description**: Registers event handler function  
**Parameters**:
- `handler`: Function pointer to event handler

**Handler Signature**:
```c
typedef void (*input_event_handler_t)(input_event_t *event);
```

### **Mouse Functions**

#### `input_get_mouse_position()`
```c
void input_get_mouse_position(int32_t *x, int32_t *y);
```
**Description**: Gets current mouse position

#### `input_is_mouse_button_pressed()`
```c
bool input_is_mouse_button_pressed(mouse_button_t button);
```
**Description**: Checks if mouse button is currently pressed

### **Keyboard Functions**

#### `input_is_key_pressed()`
```c
bool input_is_key_pressed(key_code_t key);
```
**Description**: Checks if key is currently pressed

---

## ♿ **Accessibility API**

### **Accessibility Control**

#### `accessibility_init()`
```c
int accessibility_init(void);
```
**Description**: Initializes accessibility system with WCAG 2.1 support

#### `accessibility_set_reduced_motion()`
```c
void accessibility_set_reduced_motion(bool enabled);
```
**Description**: Enables/disables reduced motion for accessibility

#### `accessibility_set_high_contrast()`
```c
void accessibility_set_high_contrast(bool enabled);
```
**Description**: Enables/disables high contrast mode

#### `accessibility_set_ui_scale()`
```c
void accessibility_set_ui_scale(float scale);
```
**Description**: Sets UI scaling factor (0.5x to 3.0x)

### **Contrast Checking**

#### `accessibility_calculate_contrast_ratio()`
```c
float accessibility_calculate_contrast_ratio(color_rgba_t fg, color_rgba_t bg);
```
**Description**: Calculates WCAG contrast ratio between colors

#### `accessibility_check_contrast_compliance()`
```c
bool accessibility_check_contrast_compliance(color_rgba_t fg, color_rgba_t bg, bool large_text);
```
**Description**: Checks if colors meet WCAG AA standards

### **Keyboard Navigation**

#### `accessibility_tab_next()`
```c
void accessibility_tab_next(void);
```
**Description**: Moves focus to next interactive widget

#### `accessibility_tab_previous()`
```c
void accessibility_tab_previous(void);
```
**Description**: Moves focus to previous interactive widget

---

## 🎨 **Color & Theme API**

### **Color Constants**
```c
#define NEURAL_BLUE         0xFF0080FF  // Primary accent
#define NEURAL_CYAN         0xFF00FFFF  // Secondary accent
#define NEURAL_WHITE        0xFFFFFFFF  // Text/borders
#define NEURAL_DARK_BLUE    0xFF002040  // Background
#define NEURAL_GREEN        0xFF00FF00  // Success/normal
#define NEURAL_RED          0xFFFF0000  // Error/critical
#define NEURAL_MATRIX       0xFF00AA00  // Matrix effects
```

### **Color Utilities**

#### `gui_color_lerp()`
```c
color_rgba_t gui_color_lerp(color_rgba_t a, color_rgba_t b, float t);
```
**Description**: Interpolates between two colors  
**Parameters**:
- `a`: Start color
- `b`: End color  
- `t`: Interpolation factor (0.0 to 1.0)

#### `fb_color_from_rgba()`
```c
uint32_t fb_color_from_rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a);
```
**Description**: Creates 32-bit color from RGBA components

---

## 📐 **Geometry & Layout API**

### **Rectangle Operations**

#### `gui_rect_scale()`
```c
rect_t gui_rect_scale(rect_t rect, float scale);
```
**Description**: Scales rectangle by factor

#### `gui_point_in_rect()`
```c
bool gui_point_in_rect(point2d_t point, rect_t rect);
```
**Description**: Tests if point is inside rectangle

### **Data Structures**

#### `rect_t`
```c
typedef struct {
    int32_t x, y;           // Position
    int32_t width, height;  // Size
} rect_t;
```

#### `color_rgba_t`
```c
typedef struct {
    uint8_t r, g, b, a;     // Red, Green, Blue, Alpha
} color_rgba_t;
```

#### `vec3_t`
```c
typedef struct {
    float x, y, z;          // 3D coordinates
} vec3_t;
```

---

## 🎯 **Camera & Parallax API**

### **Camera Control**

#### `gui_set_camera_position()`
```c
void gui_set_camera_position(vec3_t position);
```
**Description**: Sets absolute camera position for parallax

#### `gui_move_camera()`
```c
void gui_move_camera(vec3_t delta);
```
**Description**: Adds velocity to camera movement

### **Layer Management**

#### `gui_set_layer_parallax()`
```c
void gui_set_layer_parallax(gui_layer_type_t layer, float factor);
```
**Description**: Sets parallax factor for specific layer

#### `gui_set_layer_visibility()`
```c
void gui_set_layer_visibility(gui_layer_type_t layer, bool visible);
```
**Description**: Shows/hides specific GUI layer

---

## 🔍 **Debugging API**

### **Performance Monitoring**

#### `graphics_3d_get_stats()`
```c
void graphics_3d_get_stats(uint32_t *triangles, uint32_t *vertices, 
                          uint32_t *pixels, uint32_t *frame_time);
```
**Description**: Gets rendering performance statistics

#### `fb_get_performance_stats()`
```c
void fb_get_performance_stats(uint64_t *frames, uint32_t *fps, uint32_t *frame_time);
```
**Description**: Gets framebuffer performance metrics

### **Debug Functions**

#### `gui_test()`
```c
void gui_test(void);
```
**Description**: Runs GUI system self-test

#### `accessibility_test()`
```c
void accessibility_test(void);
```
**Description**: Tests accessibility compliance

---

## ⚙️ **Configuration Constants**

### **System Limits**
```c
#define MAX_GUI_LAYERS          8       // Maximum parallax layers
#define MAX_WIDGETS_PER_LAYER   32      // Widgets per layer
#define MAX_PARTICLES           100     // Neural particles
#define TARGET_FPS              60      // Target frame rate
```

### **Performance Targets**
```c
#define FRAME_TIME_MS           16      // 60fps target
#define MAX_ANIMATION_DURATION  5000    // 5 second max
#define INPUT_QUEUE_SIZE        256     // Event queue size
```

---

## 🚀 **Usage Examples**

### **Complete SCADA Interface Setup**
```c
void setup_scada_interface(void) {
    // Initialize systems
    gui_init();
    input_init();
    accessibility_init();
    
    // Create main panel
    gui_widget_t *main_panel = gui_create_panel("main",
        (rect_t){100, 50, 600, 500},
        (color_rgba_t){0x00, 0x20, 0x40, 0xB0});
    gui_add_widget(main_panel);
    
    // Create temperature gauge
    gui_widget_t *temp_gauge = gui_create_scada_gauge("temp",
        (rect_t){150, 100, 120, 120},
        0.0f, 100.0f, "°C");
    gui_add_widget(temp_gauge);
    
    // Create control button
    gui_widget_t *start_btn = gui_create_button("start",
        (rect_t){200, 400, 80, 35},
        "START", on_start_clicked);
    gui_add_widget(start_btn);
    
    // Start neural glow animation
    gui_start_animation(start_btn, ANIM_GLOW, 2000, true);
}

// Main update loop
void main_loop(void) {
    uint32_t last_time = get_time_ms();
    
    while (1) {
        uint32_t current_time = get_time_ms();
        uint32_t delta = current_time - last_time;
        
        // Update systems
        input_update();
        gui_update(delta);
        
        // Render frame
        gui_render();
        fb_swap_buffers();
        
        last_time = current_time;
        
        // Target 60fps
        if (delta < 16) {
            delay_ms(16 - delta);
        }
    }
}
```

---

*Neural API interfaces active. Brandon Media OS development framework ready.*