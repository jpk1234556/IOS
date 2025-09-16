/* gui_widgets.c - Brandon Media OS GUI Widget System
 * SCADA Controls and Neural Interface Components
 */

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <math.h>
#include "kernel/gui.h"
#include "kernel/framebuffer.h"
#include "kernel/memory.h"

/* External functions */
extern void serial_puts(const char *s);
extern uint32_t get_time_ms(void);

/* Button Widget Data */
typedef struct {
    char text[64];
    widget_click_handler_t callback;
    bool pressed;
    uint32_t press_time;
} button_data_t;

/* Panel Widget Data */
typedef struct {
    char title[64];
    bool has_border;
    uint32_t border_color;
    float border_thickness;
} panel_data_t;

/* Label Widget Data */
typedef struct {
    char text[256];
    uint32_t font_size;
    bool bold;
    bool italic;
} label_data_t;

/* Progress Bar Widget Data */
typedef struct {
    float value;        /* 0.0 to 1.0 */
    float target_value; /* For smooth animations */
    char label[64];
    bool show_percentage;
    uint32_t bar_color;
    uint32_t bg_color;
} progress_bar_data_t;

/* Forward declarations for render functions */
static void render_button(gui_widget_t *widget);
static void render_panel(gui_widget_t *widget);
static void render_label(gui_widget_t *widget);
static void render_scada_gauge(gui_widget_t *widget);
static void render_neural_matrix(gui_widget_t *widget);
static void render_progress_bar(gui_widget_t *widget);

/* Update functions */
static void update_button(gui_widget_t *widget, uint32_t delta_ms);
static void update_scada_gauge(gui_widget_t *widget, uint32_t delta_ms);
static void update_neural_matrix(gui_widget_t *widget, uint32_t delta_ms);
static void update_progress_bar(gui_widget_t *widget, uint32_t delta_ms);

/* Create Button Widget */
gui_widget_t *gui_create_button(const char *name, rect_t bounds, const char *text, widget_click_handler_t callback) {
    gui_widget_t *widget = gui_create_widget(WIDGET_BUTTON, name, bounds, LAYER_FOREGROUND);
    if (!widget) {
        return NULL;
    }
    
    /* Allocate button data */
    button_data_t *data = (button_data_t *)kmalloc(sizeof(button_data_t));
    if (!data) {
        gui_destroy_widget(widget);
        return NULL;
    }
    
    /* Initialize button data */
    memset(data, 0, sizeof(button_data_t));
    strncpy(data->text, text, sizeof(data->text) - 1);
    data->callback = callback;
    data->pressed = false;
    data->press_time = 0;
    
    /* Set widget properties */
    widget->data = data;
    widget->render = render_button;
    widget->update = update_button;
    widget->on_click = callback;
    widget->color = (color_rgba_t)GUI_COLOR_NEURAL_BLUE;
    widget->bg_color = (color_rgba_t)GUI_COLOR_DARK_BLUE;
    
    return widget;
}

/* Create Panel Widget */
gui_widget_t *gui_create_panel(const char *name, rect_t bounds, color_rgba_t bg_color) {
    gui_widget_t *widget = gui_create_widget(WIDGET_PANEL, name, bounds, LAYER_MIDGROUND_NEAR);
    if (!widget) {
        return NULL;
    }
    
    /* Allocate panel data */
    panel_data_t *data = (panel_data_t *)kmalloc(sizeof(panel_data_t));
    if (!data) {
        gui_destroy_widget(widget);
        return NULL;
    }
    
    /* Initialize panel data */
    memset(data, 0, sizeof(panel_data_t));
    strcpy(data->title, "Neural Panel");
    data->has_border = true;
    data->border_color = fb_color_from_rgba(bg_color.r, bg_color.g, bg_color.b, 255);
    data->border_thickness = 2.0f;
    
    /* Set widget properties */
    widget->data = data;
    widget->render = render_panel;
    widget->bg_color = bg_color;
    widget->interactive = false;
    
    return widget;
}

/* Create Label Widget */
gui_widget_t *gui_create_label(const char *name, rect_t bounds, const char *text) {
    gui_widget_t *widget = gui_create_widget(WIDGET_LABEL, name, bounds, LAYER_FOREGROUND);
    if (!widget) {
        return NULL;
    }
    
    /* Allocate label data */
    label_data_t *data = (label_data_t *)kmalloc(sizeof(label_data_t));
    if (!data) {
        gui_destroy_widget(widget);
        return NULL;
    }
    
    /* Initialize label data */
    memset(data, 0, sizeof(label_data_t));
    strncpy(data->text, text, sizeof(data->text) - 1);
    data->font_size = 12;
    data->bold = false;
    data->italic = false;
    
    /* Set widget properties */
    widget->data = data;
    widget->render = render_label;
    widget->color = (color_rgba_t)GUI_COLOR_NEURAL_WHITE;
    widget->interactive = false;
    
    return widget;
}

/* Create SCADA Gauge Widget */
gui_widget_t *gui_create_scada_gauge(const char *name, rect_t bounds, float min_val, float max_val, const char *unit) {
    gui_widget_t *widget = gui_create_widget(WIDGET_SCADA_GAUGE, name, bounds, LAYER_FOREGROUND);
    if (!widget) {
        return NULL;
    }
    
    /* Allocate gauge data */
    scada_gauge_data_t *data = (scada_gauge_data_t *)kmalloc(sizeof(scada_gauge_data_t));
    if (!data) {
        gui_destroy_widget(widget);
        return NULL;
    }
    
    /* Initialize gauge data */
    memset(data, 0, sizeof(scada_gauge_data_t));
    data->min_value = min_val;
    data->max_value = max_val;
    data->current_value = min_val;
    data->target_value = min_val;
    strncpy(data->unit, unit, sizeof(data->unit) - 1);
    data->needle_color = (color_rgba_t)GUI_COLOR_NEURAL_RED;
    data->critical_alarm = false;
    
    /* Set widget properties */
    widget->data = data;
    widget->render = render_scada_gauge;
    widget->update = update_scada_gauge;
    widget->color = (color_rgba_t)GUI_COLOR_NEURAL_CYAN;
    widget->bg_color = (color_rgba_t)GUI_COLOR_DARK_BLUE;
    
    return widget;
}

/* Create Neural Matrix Widget */
gui_widget_t *gui_create_neural_matrix(const char *name, rect_t bounds, uint32_t width, uint32_t height) {
    gui_widget_t *widget = gui_create_widget(WIDGET_NEURAL_MATRIX, name, bounds, LAYER_BACKGROUND);
    if (!widget) {
        return NULL;
    }
    
    /* Allocate matrix data */
    neural_matrix_data_t *data = (neural_matrix_data_t *)kmalloc(sizeof(neural_matrix_data_t));
    if (!data) {
        gui_destroy_widget(widget);
        return NULL;
    }
    
    /* Initialize matrix data */
    memset(data, 0, sizeof(neural_matrix_data_t));
    data->matrix_width = width;
    data->matrix_height = height;
    data->data_matrix = (float *)kmalloc(width * height * sizeof(float));
    data->animation_phase = 0;
    data->primary_color = (color_rgba_t)GUI_COLOR_MATRIX_GREEN;
    data->secondary_color = (color_rgba_t)GUI_COLOR_NEURAL_CYAN;
    
    /* Initialize matrix with random data */
    if (data->data_matrix) {
        for (uint32_t i = 0; i < width * height; i++) {
            data->data_matrix[i] = (float)(i % 256) / 255.0f;
        }
    }
    
    /* Set widget properties */
    widget->data = data;
    widget->render = render_neural_matrix;
    widget->update = update_neural_matrix;
    widget->interactive = false;
    
    return widget;
}

/* Create Progress Bar Widget */
gui_widget_t *gui_create_progress_bar(const char *name, rect_t bounds, const char *label) {
    gui_widget_t *widget = gui_create_widget(WIDGET_PROGRESS_BAR, name, bounds, LAYER_FOREGROUND);
    if (!widget) {
        return NULL;
    }
    
    /* Allocate progress bar data */
    progress_bar_data_t *data = (progress_bar_data_t *)kmalloc(sizeof(progress_bar_data_t));
    if (!data) {
        gui_destroy_widget(widget);
        return NULL;
    }
    
    /* Initialize progress bar data */
    memset(data, 0, sizeof(progress_bar_data_t));
    data->value = 0.0f;
    data->target_value = 0.0f;
    strncpy(data->label, label, sizeof(data->label) - 1);
    data->show_percentage = true;
    data->bar_color = fb_color_from_rgba(0x00, 0xFF, 0x00, 0xFF); /* Green */
    data->bg_color = fb_color_from_rgba(0x00, 0x20, 0x40, 0xFF);  /* Dark blue */
    
    /* Set widget properties */
    widget->data = data;
    widget->render = render_progress_bar;
    widget->update = update_progress_bar;
    widget->interactive = false;
    
    return widget;
}

/* Render Button Widget */
static void render_button(gui_widget_t *widget) {
    if (!widget || !widget->data || !widget->visible) {
        return;
    }
    
    button_data_t *data = (button_data_t *)widget->data;
    framebuffer_device_t *fb = framebuffer_get_device();
    
    if (!fb) {
        return;
    }
    
    /* Determine button color based on state */
    uint32_t button_color;
    uint32_t text_color = fb_color_from_rgba(widget->color.r, widget->color.g, widget->color.b, widget->color.a);
    
    switch (widget->state) {
        case WIDGET_STATE_HOVER:
            button_color = fb_color_multiply(fb_color_from_rgba(widget->bg_color.r, widget->bg_color.g, widget->bg_color.b, widget->bg_color.a), 1.2f);
            break;
        case WIDGET_STATE_ACTIVE:
            button_color = fb_color_multiply(fb_color_from_rgba(widget->bg_color.r, widget->bg_color.g, widget->bg_color.b, widget->bg_color.a), 0.8f);
            break;
        case WIDGET_STATE_DISABLED:
            button_color = fb_color_multiply(fb_color_from_rgba(widget->bg_color.r, widget->bg_color.g, widget->bg_color.b, widget->bg_color.a), 0.5f);
            text_color = fb_color_multiply(text_color, 0.5f);
            break;
        default:
            button_color = fb_color_from_rgba(widget->bg_color.r, widget->bg_color.g, widget->bg_color.b, widget->bg_color.a);
            break;
    }
    
    /* Draw button background */
    fb_fill_rect(widget->bounds.x, widget->bounds.y, widget->bounds.width, widget->bounds.height, button_color);
    
    /* Draw button border */
    fb_draw_rect(widget->bounds.x, widget->bounds.y, widget->bounds.width, widget->bounds.height, text_color, 2);
    
    /* Draw button text (simplified) */
    int32_t text_x = widget->bounds.x + widget->bounds.width / 2 - (strlen(data->text) * 4);
    int32_t text_y = widget->bounds.y + widget->bounds.height / 2 - 6;
    fb_draw_string(text_x, text_y, data->text, text_color, 0);
    
    /* Add neural glow effect if hovered */
    if (widget->state == WIDGET_STATE_HOVER) {
        uint32_t glow_color = fb_color_from_rgba(0x00, 0xFF, 0xFF, 0x40); /* Semi-transparent cyan */
        fb_draw_rect(widget->bounds.x - 2, widget->bounds.y - 2, 
                    widget->bounds.width + 4, widget->bounds.height + 4, glow_color, 1);
    }
}

/* Render Panel Widget */
static void render_panel(gui_widget_t *widget) {
    if (!widget || !widget->data || !widget->visible) {
        return;
    }
    
    panel_data_t *data = (panel_data_t *)widget->data;
    framebuffer_device_t *fb = framebuffer_get_device();
    
    if (!fb) {
        return;
    }
    
    /* Draw panel background */
    uint32_t bg_color = fb_color_from_rgba(widget->bg_color.r, widget->bg_color.g, widget->bg_color.b, widget->bg_color.a);
    fb_fill_rect(widget->bounds.x, widget->bounds.y, widget->bounds.width, widget->bounds.height, bg_color);
    
    /* Draw border if enabled */
    if (data->has_border) {
        fb_draw_rect(widget->bounds.x, widget->bounds.y, widget->bounds.width, widget->bounds.height, 
                    data->border_color, (uint32_t)data->border_thickness);
    }
    
    /* Draw title */
    if (strlen(data->title) > 0) {
        uint32_t title_color = fb_color_from_rgba(widget->color.r, widget->color.g, widget->color.b, widget->color.a);
        fb_draw_string(widget->bounds.x + 8, widget->bounds.y + 4, data->title, title_color, 0);
    }
}

/* Render SCADA Gauge Widget */
static void render_scada_gauge(gui_widget_t *widget) {
    if (!widget || !widget->data || !widget->visible) {
        return;
    }
    
    scada_gauge_data_t *data = (scada_gauge_data_t *)widget->data;
    framebuffer_device_t *fb = framebuffer_get_device();
    
    if (!fb) {
        return;
    }
    
    int32_t center_x = widget->bounds.x + widget->bounds.width / 2;
    int32_t center_y = widget->bounds.y + widget->bounds.height / 2;
    uint32_t radius = (widget->bounds.width < widget->bounds.height ? widget->bounds.width : widget->bounds.height) / 2 - 10;
    
    /* Draw gauge background circle */
    uint32_t bg_color = fb_color_from_rgba(widget->bg_color.r, widget->bg_color.g, widget->bg_color.b, widget->bg_color.a);
    fb_fill_circle(center_x, center_y, radius, bg_color);
    
    /* Draw gauge border */
    uint32_t border_color = fb_color_from_rgba(widget->color.r, widget->color.g, widget->color.b, widget->color.a);
    fb_draw_circle(center_x, center_y, radius, border_color);
    
    /* Draw scale marks */
    for (int i = 0; i <= 10; i++) {
        float angle = -PI + (i * PI) / 10.0f; /* -180 to 0 degrees */
        int32_t mark_x1 = center_x + (int32_t)((radius - 15) * cosf(angle));
        int32_t mark_y1 = center_y + (int32_t)((radius - 15) * sinf(angle));
        int32_t mark_x2 = center_x + (int32_t)((radius - 5) * cosf(angle));
        int32_t mark_y2 = center_y + (int32_t)((radius - 5) * sinf(angle));
        
        fb_draw_line(mark_x1, mark_y1, mark_x2, mark_y2, border_color);
    }
    
    /* Draw needle */
    float value_range = data->max_value - data->min_value;
    float normalized_value = (data->current_value - data->min_value) / value_range;
    float needle_angle = -PI + normalized_value * PI; /* -180 to 0 degrees */
    
    int32_t needle_x = center_x + (int32_t)((radius - 20) * cosf(needle_angle));
    int32_t needle_y = center_y + (int32_t)((radius - 20) * sinf(needle_angle));
    
    uint32_t needle_color = data->critical_alarm ? 
        fb_color_from_rgba(data->needle_color.r, data->needle_color.g, data->needle_color.b, data->needle_color.a) :
        fb_color_from_rgba(0x00, 0xFF, 0x00, 0xFF); /* Green for normal */
    
    fb_draw_line(center_x, center_y, needle_x, needle_y, needle_color);
    
    /* Draw center dot */
    fb_fill_circle(center_x, center_y, 3, needle_color);
    
    /* Draw value text */
    char value_text[32];
    snprintf(value_text, sizeof(value_text), "%.1f %s", data->current_value, data->unit);
    fb_draw_string(center_x - 30, center_y + radius - 20, value_text, border_color, 0);
}

/* Render Neural Matrix Widget */
static void render_neural_matrix(gui_widget_t *widget) {
    if (!widget || !widget->data || !widget->visible) {
        return;
    }
    
    neural_matrix_data_t *data = (neural_matrix_data_t *)widget->data;
    framebuffer_device_t *fb = framebuffer_get_device();
    
    if (!fb || !data->data_matrix) {
        return;
    }
    
    /* Render matrix pattern */
    uint32_t cell_width = widget->bounds.width / data->matrix_width;
    uint32_t cell_height = widget->bounds.height / data->matrix_height;
    
    for (uint32_t y = 0; y < data->matrix_height; y++) {
        for (uint32_t x = 0; x < data->matrix_width; x++) {
            uint32_t index = y * data->matrix_width + x;
            float value = data->data_matrix[index];
            
            /* Add animation effect */
            float animated_value = value + sinf(data->animation_phase * 0.01f + x * 0.1f + y * 0.1f) * 0.2f;
            if (animated_value < 0.0f) animated_value = 0.0f;
            if (animated_value > 1.0f) animated_value = 1.0f;
            
            /* Calculate color based on value */
            uint8_t intensity = (uint8_t)(animated_value * 255);
            uint32_t color = fb_color_from_rgba(0, intensity, 0, 255); /* Green matrix effect */
            
            /* Draw cell */
            int32_t cell_x = widget->bounds.x + x * cell_width;
            int32_t cell_y = widget->bounds.y + y * cell_height;
            
            if (intensity > 50) { /* Only draw bright cells */
                fb_fill_rect(cell_x, cell_y, cell_width - 1, cell_height - 1, color);
            }
        }
    }
}

/* Render Progress Bar Widget */
static void render_progress_bar(gui_widget_t *widget) {
    if (!widget || !widget->data || !widget->visible) {
        return;
    }
    
    progress_bar_data_t *data = (progress_bar_data_t *)widget->data;
    framebuffer_device_t *fb = framebuffer_get_device();
    
    if (!fb) {
        return;
    }
    
    /* Draw background */
    fb_fill_rect(widget->bounds.x, widget->bounds.y, widget->bounds.width, widget->bounds.height, data->bg_color);
    
    /* Draw progress */
    uint32_t progress_width = (uint32_t)(widget->bounds.width * data->value);
    if (progress_width > 0) {
        fb_fill_rect(widget->bounds.x, widget->bounds.y, progress_width, widget->bounds.height, data->bar_color);
    }
    
    /* Draw border */
    uint32_t border_color = fb_color_from_rgba(widget->color.r, widget->color.g, widget->color.b, widget->color.a);
    fb_draw_rect(widget->bounds.x, widget->bounds.y, widget->bounds.width, widget->bounds.height, border_color, 1);
    
    /* Draw label and percentage */
    if (strlen(data->label) > 0) {
        fb_draw_string(widget->bounds.x + 4, widget->bounds.y + 2, data->label, border_color, 0);
    }
    
    if (data->show_percentage) {
        char percent_text[16];
        snprintf(percent_text, sizeof(percent_text), "%d%%", (int)(data->value * 100));
        fb_draw_string(widget->bounds.x + widget->bounds.width - 30, widget->bounds.y + 2, percent_text, border_color, 0);
    }
}

/* Update Functions */
static void update_button(gui_widget_t *widget, uint32_t delta_ms) {
    button_data_t *data = (button_data_t *)widget->data;
    
    if (data->pressed) {
        data->press_time += delta_ms;
        if (data->press_time > 100) { /* 100ms press effect */
            data->pressed = false;
            data->press_time = 0;
            widget->state = WIDGET_STATE_NORMAL;
        }
    }
}

static void update_scada_gauge(gui_widget_t *widget, uint32_t delta_ms) {
    scada_gauge_data_t *data = (scada_gauge_data_t *)widget->data;
    
    /* Smooth value interpolation */
    float diff = data->target_value - data->current_value;
    if (fabsf(diff) > 0.01f) {
        data->current_value += diff * (delta_ms / 1000.0f) * 2.0f; /* 2 units per second */
    }
}

static void update_neural_matrix(gui_widget_t *widget, uint32_t delta_ms) {
    neural_matrix_data_t *data = (neural_matrix_data_t *)widget->data;
    
    /* Update animation phase */
    data->animation_phase += delta_ms / 10; /* Slower animation */
    if (data->animation_phase > 360000) {
        data->animation_phase = 0;
    }
}

static void update_progress_bar(gui_widget_t *widget, uint32_t delta_ms) {
    progress_bar_data_t *data = (progress_bar_data_t *)widget->data;
    
    /* Smooth progress animation */
    float diff = data->target_value - data->value;
    if (fabsf(diff) > 0.001f) {
        data->value += diff * (delta_ms / 1000.0f) * 2.0f; /* 2 units per second */
    }
}

/* Widget Value Update Functions */
void gui_update_gauge_value(gui_widget_t *gauge, float value) {
    if (!gauge || gauge->type != WIDGET_SCADA_GAUGE || !gauge->data) {
        return;
    }
    
    scada_gauge_data_t *data = (scada_gauge_data_t *)gauge->data;
    
    /* Clamp value to range */
    if (value < data->min_value) value = data->min_value;
    if (value > data->max_value) value = data->max_value;
    
    data->target_value = value;
}

void gui_set_gauge_alarm(gui_widget_t *gauge, bool critical) {
    if (!gauge || gauge->type != WIDGET_SCADA_GAUGE || !gauge->data) {
        return;
    }
    
    scada_gauge_data_t *data = (scada_gauge_data_t *)gauge->data;
    data->critical_alarm = critical;
}

void gui_set_progress_value(gui_widget_t *progress_bar, float value) {
    if (!progress_bar || progress_bar->type != WIDGET_PROGRESS_BAR || !progress_bar->data) {
        return;
    }
    
    progress_bar_data_t *data = (progress_bar_data_t *)progress_bar->data;
    
    /* Clamp value to 0.0 - 1.0 */
    if (value < 0.0f) value = 0.0f;
    if (value > 1.0f) value = 1.0f;
    
    data->target_value = value;
}