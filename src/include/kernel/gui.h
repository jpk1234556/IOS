/* gui.h - Brandon Media OS Neural GUI System
 * SCADA 3D Parallax Interface Architecture
 * Cyberpunk Aesthetic with Dark Blue/White Theme
 */

#ifndef KERNEL_GUI_H
#define KERNEL_GUI_H

#include <stdint.h>
#include <stdbool.h>
#include "kernel/framebuffer.h"

/* GUI Layer System for 3D Parallax Effects */
#define MAX_GUI_LAYERS 8
#define MAX_WIDGETS_PER_LAYER 32

/* Parallax Layer Types */
typedef enum {
    LAYER_BACKGROUND = 0,     /* Static background patterns */
    LAYER_MIDGROUND_FAR,      /* Slow-moving elements */
    LAYER_MIDGROUND_NEAR,     /* Medium-speed elements */
    LAYER_FOREGROUND,         /* Interactive UI elements */
    LAYER_HUD_OVERLAY,        /* Always-on-top HUD elements */
    LAYER_MODAL,              /* Modal dialogs and overlays */
    LAYER_DEBUG,              /* Debug information overlay */
    LAYER_CURSOR              /* Mouse cursor */
} gui_layer_type_t;

/* 3D Vector for Parallax Calculations */
typedef struct {
    float x, y, z;
} vec3_t;

/* 2D Point Structure */
typedef struct {
    int32_t x, y;
} point2d_t;

/* Rectangle Structure */
typedef struct {
    int32_t x, y, width, height;
} rect_t;

/* Color Structure with Alpha */
typedef struct {
    uint8_t r, g, b, a;
} color_rgba_t;

/* Parallax Layer Configuration */
typedef struct {
    gui_layer_type_t type;
    float parallax_factor;    /* 0.0 = static, 1.0 = full speed */
    float depth_offset;       /* Z-depth for 3D calculations */
    vec3_t offset;           /* Current offset position */
    bool visible;
    bool interactive;
    uint8_t opacity;         /* 0-255 */
} gui_layer_t;

/* Widget Types */
typedef enum {
    WIDGET_BUTTON,
    WIDGET_PANEL,
    WIDGET_LABEL,
    WIDGET_INDICATOR,
    WIDGET_GRAPH,
    WIDGET_SLIDER,
    WIDGET_PROGRESS_BAR,
    WIDGET_SCADA_GAUGE,
    WIDGET_NEURAL_MATRIX
} widget_type_t;

/* Widget State */
typedef enum {
    WIDGET_STATE_NORMAL,
    WIDGET_STATE_HOVER,
    WIDGET_STATE_ACTIVE,
    WIDGET_STATE_DISABLED,
    WIDGET_STATE_ERROR,
    WIDGET_STATE_WARNING
} widget_state_t;

/* Animation Types */
typedef enum {
    ANIM_NONE,
    ANIM_FADE,
    ANIM_SLIDE,
    ANIM_SCALE,
    ANIM_ROTATE,
    ANIM_GLOW,
    ANIM_NEURAL_PULSE,
    ANIM_MATRIX_FLOW
} animation_type_t;

/* Animation Structure */
typedef struct {
    animation_type_t type;
    uint32_t duration_ms;
    uint32_t elapsed_ms;
    float progress;          /* 0.0 to 1.0 */
    bool active;
    bool loop;
    void (*on_complete)(void *widget);
} animation_t;

/* Forward Declaration */
struct gui_widget;

/* Widget Event Handlers */
typedef void (*widget_click_handler_t)(struct gui_widget *widget);
typedef void (*widget_render_handler_t)(struct gui_widget *widget);
typedef void (*widget_update_handler_t)(struct gui_widget *widget, uint32_t delta_ms);

/* GUI Widget Base Structure */
typedef struct gui_widget {
    widget_type_t type;
    char name[32];
    rect_t bounds;
    gui_layer_type_t layer;
    widget_state_t state;
    color_rgba_t color;
    color_rgba_t bg_color;
    bool visible;
    bool interactive;
    float depth;             /* Z-depth for 3D rendering */
    
    /* Animation */
    animation_t animation;
    
    /* Event Handlers */
    widget_click_handler_t on_click;
    widget_render_handler_t render;
    widget_update_handler_t update;
    
    /* Widget-specific data */
    void *data;
    
    /* Linked list for layer management */
    struct gui_widget *next;
    struct gui_widget *prev;
} gui_widget_t;

/* SCADA Gauge Widget Data */
typedef struct {
    float min_value;
    float max_value;
    float current_value;
    float target_value;      /* For smooth animations */
    char unit[16];          /* e.g., "°C", "RPM", "%" */
    color_rgba_t needle_color;
    bool critical_alarm;
} scada_gauge_data_t;

/* Neural Matrix Widget Data */
typedef struct {
    uint32_t matrix_width;
    uint32_t matrix_height;
    float *data_matrix;
    uint32_t animation_phase;
    color_rgba_t primary_color;
    color_rgba_t secondary_color;
} neural_matrix_data_t;

/* GUI System State */
typedef struct {
    gui_layer_t layers[MAX_GUI_LAYERS];
    gui_widget_t *widgets[MAX_GUI_LAYERS][MAX_WIDGETS_PER_LAYER];
    uint32_t widget_count[MAX_GUI_LAYERS];
    
    /* Camera/View State for Parallax */
    vec3_t camera_position;
    vec3_t camera_velocity;
    float parallax_strength;
    
    /* Input State */
    point2d_t mouse_pos;
    bool mouse_buttons[3];   /* Left, Right, Middle */
    gui_widget_t *focused_widget;
    gui_widget_t *hovered_widget;
    
    /* Animation System */
    uint32_t frame_time_ms;
    uint32_t last_frame_time;
    
    /* Accessibility */
    bool reduced_motion;
    bool high_contrast;
    float ui_scale;
    
    /* Theme Colors (Cyberpunk Dark Blue/White) */
    color_rgba_t theme_primary;      /* Neural Blue */
    color_rgba_t theme_secondary;    /* Cyan */
    color_rgba_t theme_accent;       /* White */
    color_rgba_t theme_background;   /* Dark Blue */
    color_rgba_t theme_error;        /* Red */
    color_rgba_t theme_warning;      /* Orange */
    color_rgba_t theme_success;      /* Green */
    
    bool initialized;
} gui_system_t;

/* Core GUI Functions */
int gui_init(void);
void gui_shutdown(void);
void gui_update(uint32_t delta_ms);
void gui_render(void);
void gui_handle_input(void);

/* Layer Management */
void gui_set_layer_parallax(gui_layer_type_t layer, float factor);
void gui_set_layer_visibility(gui_layer_type_t layer, bool visible);
void gui_set_camera_position(vec3_t position);
void gui_move_camera(vec3_t delta);

/* Widget Management */
gui_widget_t *gui_create_widget(widget_type_t type, const char *name, rect_t bounds, gui_layer_type_t layer);
void gui_destroy_widget(gui_widget_t *widget);
void gui_add_widget(gui_widget_t *widget);
void gui_remove_widget(gui_widget_t *widget);
gui_widget_t *gui_find_widget(const char *name);
gui_widget_t *gui_widget_at_position(point2d_t pos);

/* Specific Widget Creators */
gui_widget_t *gui_create_button(const char *name, rect_t bounds, const char *text, widget_click_handler_t callback);
gui_widget_t *gui_create_panel(const char *name, rect_t bounds, color_rgba_t bg_color);
gui_widget_t *gui_create_label(const char *name, rect_t bounds, const char *text);
gui_widget_t *gui_create_scada_gauge(const char *name, rect_t bounds, float min_val, float max_val, const char *unit);
gui_widget_t *gui_create_neural_matrix(const char *name, rect_t bounds, uint32_t width, uint32_t height);

/* Animation System */
void gui_start_animation(gui_widget_t *widget, animation_type_t type, uint32_t duration_ms, bool loop);
void gui_stop_animation(gui_widget_t *widget);
void gui_update_animations(uint32_t delta_ms);

/* SCADA Functions */
void gui_update_gauge_value(gui_widget_t *gauge, float value);
void gui_set_gauge_alarm(gui_widget_t *gauge, bool critical);
void gui_update_neural_matrix(gui_widget_t *matrix, float *data);

/* Theme and Accessibility */
void gui_set_theme_colors(color_rgba_t primary, color_rgba_t secondary, color_rgba_t accent, color_rgba_t background);
void gui_enable_reduced_motion(bool enable);
void gui_set_high_contrast(bool enable);
void gui_set_ui_scale(float scale);

/* Utility Functions */
color_rgba_t gui_color_lerp(color_rgba_t a, color_rgba_t b, float t);
float gui_ease_in_out(float t);
rect_t gui_rect_scale(rect_t rect, float scale);
bool gui_point_in_rect(point2d_t point, rect_t rect);

/* Color Constants (Cyberpunk Theme) */
#define GUI_COLOR_NEURAL_BLUE    {0x00, 0x80, 0xFF, 0xFF}
#define GUI_COLOR_NEURAL_CYAN    {0x00, 0xFF, 0xFF, 0xFF}
#define GUI_COLOR_NEURAL_WHITE   {0xFF, 0xFF, 0xFF, 0xFF}
#define GUI_COLOR_NEURAL_BLACK   {0x00, 0x00, 0x00, 0xFF}
#define GUI_COLOR_NEURAL_GREEN   {0x00, 0xFF, 0x00, 0xFF}
#define GUI_COLOR_NEURAL_RED     {0xFF, 0x00, 0x00, 0xFF}
#define GUI_COLOR_NEURAL_PURPLE  {0x80, 0x00, 0x80, 0xFF}
#define GUI_COLOR_MATRIX_GREEN   {0x00, 0xAA, 0x00, 0xFF}
#define GUI_COLOR_DARK_BLUE      {0x00, 0x20, 0x40, 0xFF}

#endif /* KERNEL_GUI_H */