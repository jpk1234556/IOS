/* gui.c - Brandon Media OS Neural GUI System Implementation
 * SCADA 3D Parallax Interface with Cyberpunk Aesthetics
 */

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include "kernel/gui.h"
#include "kernel/framebuffer.h"
#include "kernel/input.h"
#include "kernel/memory.h"

/* External functions */
extern void serial_puts(const char *s);
extern void print_dec(uint64_t num);
extern uint32_t get_time_ms(void);

/* Global GUI System State */
static gui_system_t gui_system;
static bool gui_initialized = false;

/* Initialize GUI System */
int gui_init(void) {
    if (gui_initialized) {
        return 0;
    }
    
    serial_puts("[NEURAL-GUI] Initializing Neural GUI System...\n");
    
    /* Clear system state */
    memset(&gui_system, 0, sizeof(gui_system_t));\n    \n    /* Initialize layers */
    for (int i = 0; i < MAX_GUI_LAYERS; i++) {
        gui_system.layers[i].type = (gui_layer_type_t)i;
        gui_system.layers[i].parallax_factor = 1.0f - (i * 0.1f); /* Decreasing parallax */
        gui_system.layers[i].depth_offset = i * 10.0f;
        gui_system.layers[i].visible = true;
        gui_system.layers[i].interactive = (i >= LAYER_FOREGROUND);
        gui_system.layers[i].opacity = 255;
        gui_system.widget_count[i] = 0;
    }
    
    /* Set parallax factors for specific layers */
    gui_system.layers[LAYER_BACKGROUND].parallax_factor = 0.1f;\n    gui_system.layers[LAYER_MIDGROUND_FAR].parallax_factor = 0.3f;
    gui_system.layers[LAYER_MIDGROUND_NEAR].parallax_factor = 0.6f;
    gui_system.layers[LAYER_FOREGROUND].parallax_factor = 1.0f;
    gui_system.layers[LAYER_HUD_OVERLAY].parallax_factor = 0.0f; /* Static */
    
    /* Initialize camera */
    gui_system.camera_position = (vec3_t){0.0f, 0.0f, 0.0f};
    gui_system.camera_velocity = (vec3_t){0.0f, 0.0f, 0.0f};
    gui_system.parallax_strength = 1.0f;
    
    /* Initialize input state */
    gui_system.mouse_pos = (point2d_t){0, 0};
    gui_system.mouse_buttons[0] = false;
    gui_system.mouse_buttons[1] = false;
    gui_system.mouse_buttons[2] = false;
    gui_system.focused_widget = NULL;
    gui_system.hovered_widget = NULL;
    
    /* Initialize timing */
    gui_system.frame_time_ms = 16; /* Target 60fps */
    gui_system.last_frame_time = get_time_ms();
    
    /* Initialize accessibility settings */
    gui_system.reduced_motion = false;
    gui_system.high_contrast = false;
    gui_system.ui_scale = 1.0f;
    
    /* Set cyberpunk theme colors */
    gui_system.theme_primary = (color_rgba_t)GUI_COLOR_NEURAL_BLUE;
    gui_system.theme_secondary = (color_rgba_t)GUI_COLOR_NEURAL_CYAN;
    gui_system.theme_accent = (color_rgba_t)GUI_COLOR_NEURAL_WHITE;
    gui_system.theme_background = (color_rgba_t)GUI_COLOR_DARK_BLUE;
    gui_system.theme_error = (color_rgba_t)GUI_COLOR_NEURAL_RED;
    gui_system.theme_warning = (color_rgba_t){0xFF, 0xA5, 0x00, 0xFF}; /* Orange */
    gui_system.theme_success = (color_rgba_t)GUI_COLOR_NEURAL_GREEN;
    
    gui_system.initialized = true;
    gui_initialized = true;
    
    serial_puts("[NEURAL-GUI] Neural GUI System initialized successfully\n");
    return 0;
}

/* Shutdown GUI System */
void gui_shutdown(void) {
    if (!gui_initialized) {
        return;
    }
    
    serial_puts("[NEURAL-GUI] Shutting down Neural GUI System...\n");
    
    /* Destroy all widgets */
    for (int layer = 0; layer < MAX_GUI_LAYERS; layer++) {
        for (int i = 0; i < gui_system.widget_count[layer]; i++) {
            if (gui_system.widgets[layer][i]) {
                gui_destroy_widget(gui_system.widgets[layer][i]);
            }
        }
        gui_system.widget_count[layer] = 0;
    }
    
    gui_system.initialized = false;
    gui_initialized = false;
    
    serial_puts("[NEURAL-GUI] Neural GUI System shutdown complete\n");
}

/* Update GUI System */
void gui_update(uint32_t delta_ms) {
    if (!gui_initialized) {
        return;
    }
    
    /* Update timing */
    gui_system.frame_time_ms = delta_ms;
    gui_system.last_frame_time = get_time_ms();
    
    /* Update camera velocity (smooth deceleration) */
    gui_system.camera_velocity.x *= 0.95f;
    gui_system.camera_velocity.y *= 0.95f;
    gui_system.camera_velocity.z *= 0.95f;
    
    /* Apply camera movement */
    gui_system.camera_position.x += gui_system.camera_velocity.x * (delta_ms / 1000.0f);
    gui_system.camera_position.y += gui_system.camera_velocity.y * (delta_ms / 1000.0f);
    gui_system.camera_position.z += gui_system.camera_velocity.z * (delta_ms / 1000.0f);
    
    /* Update layer offsets based on camera movement */
    for (int i = 0; i < MAX_GUI_LAYERS; i++) {
        gui_layer_t *layer = &gui_system.layers[i];
        layer->offset.x = gui_system.camera_position.x * layer->parallax_factor;
        layer->offset.y = gui_system.camera_position.y * layer->parallax_factor;
        layer->offset.z = gui_system.camera_position.z * layer->parallax_factor;
    }
    
    /* Update widgets */
    for (int layer = 0; layer < MAX_GUI_LAYERS; layer++) {
        for (int i = 0; i < gui_system.widget_count[layer]; i++) {
            gui_widget_t *widget = gui_system.widgets[layer][i];
            if (widget && widget->update) {
                widget->update(widget, delta_ms);
            }
        }
    }
    
    /* Update animations */
    gui_update_animations(delta_ms);
}

/* Render GUI System */
void gui_render(void) {
    if (!gui_initialized) {
        return;
    }
    
    framebuffer_device_t *fb = framebuffer_get_device();
    if (!fb) {
        return;
    }
    
    /* Clear background with cyberpunk theme */
    uint32_t bg_color = fb_color_from_rgba(gui_system.theme_background.r,
                                          gui_system.theme_background.g,
                                          gui_system.theme_background.b,
                                          gui_system.theme_background.a);
    fb_clear_screen(bg_color);
    
    /* Render neural background effect */
    fb_neural_matrix_effect(gui_system.last_frame_time);
    
    /* Render layers in depth order (back to front) */
    for (int layer = 0; layer < MAX_GUI_LAYERS; layer++) {
        gui_layer_t *current_layer = &gui_system.layers[layer];
        
        if (!current_layer->visible) {
            continue;
        }
        
        /* Render widgets in this layer */
        for (int i = 0; i < gui_system.widget_count[layer]; i++) {
            gui_widget_t *widget = gui_system.widgets[layer][i];
            if (widget && widget->visible && widget->render) {
                widget->render(widget);
            }
        }
    }
    
    /* Render debug information if enabled */
    #ifdef GUI_DEBUG
    gui_render_debug_info();
    #endif
}

/* Create Widget */
gui_widget_t *gui_create_widget(widget_type_t type, const char *name, rect_t bounds, gui_layer_type_t layer) {
    if (!gui_initialized) {
        return NULL;
    }
    
    /* Check layer capacity */
    if (gui_system.widget_count[layer] >= MAX_WIDGETS_PER_LAYER) {
        return NULL;
    }
    
    /* Allocate widget */
    gui_widget_t *widget = (gui_widget_t *)kmalloc(sizeof(gui_widget_t));
    if (!widget) {
        return NULL;
    }
    
    /* Initialize widget */
    memset(widget, 0, sizeof(gui_widget_t));
    widget->type = type;
    strncpy(widget->name, name, sizeof(widget->name) - 1);
    widget->bounds = bounds;
    widget->layer = layer;
    widget->state = WIDGET_STATE_NORMAL;
    widget->color = gui_system.theme_primary;
    widget->bg_color = gui_system.theme_background;
    widget->visible = true;
    widget->interactive = gui_system.layers[layer].interactive;
    widget->depth = gui_system.layers[layer].depth_offset;
    
    /* Initialize animation */
    widget->animation.type = ANIM_NONE;
    widget->animation.active = false;
    
    return widget;
}

/* Destroy Widget */
void gui_destroy_widget(gui_widget_t *widget) {
    if (!widget) {
        return;
    }
    
    /* Remove from layer */
    gui_remove_widget(widget);
    
    /* Free widget-specific data */
    if (widget->data) {
        kfree(widget->data);
    }
    
    /* Free widget */
    kfree(widget);
}

/* Add Widget to System */
void gui_add_widget(gui_widget_t *widget) {
    if (!widget || !gui_initialized) {
        return;
    }
    
    int layer = widget->layer;
    if (layer >= MAX_GUI_LAYERS || gui_system.widget_count[layer] >= MAX_WIDGETS_PER_LAYER) {
        return;
    }
    
    /* Add to layer */
    gui_system.widgets[layer][gui_system.widget_count[layer]] = widget;
    gui_system.widget_count[layer]++;
}

/* Remove Widget from System */
void gui_remove_widget(gui_widget_t *widget) {
    if (!widget || !gui_initialized) {
        return;
    }
    
    int layer = widget->layer;
    if (layer >= MAX_GUI_LAYERS) {
        return;
    }
    
    /* Find and remove widget */
    for (int i = 0; i < gui_system.widget_count[layer]; i++) {
        if (gui_system.widgets[layer][i] == widget) {
            /* Shift remaining widgets */
            for (int j = i; j < gui_system.widget_count[layer] - 1; j++) {
                gui_system.widgets[layer][j] = gui_system.widgets[layer][j + 1];
            }
            gui_system.widget_count[layer]--;
            break;
        }
    }
}

/* Find Widget by Name */
gui_widget_t *gui_find_widget(const char *name) {
    if (!name || !gui_initialized) {
        return NULL;
    }
    
    for (int layer = 0; layer < MAX_GUI_LAYERS; layer++) {
        for (int i = 0; i < gui_system.widget_count[layer]; i++) {
            gui_widget_t *widget = gui_system.widgets[layer][i];
            if (widget && strcmp(widget->name, name) == 0) {
                return widget;
            }
        }
    }
    
    return NULL;
}

/* Update Animations */
void gui_update_animations(uint32_t delta_ms) {
    for (int layer = 0; layer < MAX_GUI_LAYERS; layer++) {
        for (int i = 0; i < gui_system.widget_count[layer]; i++) {
            gui_widget_t *widget = gui_system.widgets[layer][i];
            if (!widget || !widget->animation.active) {
                continue;
            }
            
            animation_t *anim = &widget->animation;
            anim->elapsed_ms += delta_ms;
            
            /* Calculate progress */
            if (anim->duration_ms > 0) {
                anim->progress = (float)anim->elapsed_ms / (float)anim->duration_ms;
                if (anim->progress > 1.0f) {
                    anim->progress = 1.0f;
                }
            } else {
                anim->progress = 1.0f;
            }
            
            /* Check if animation is complete */
            if (anim->progress >= 1.0f) {
                if (anim->loop) {
                    anim->elapsed_ms = 0;
                    anim->progress = 0.0f;
                } else {
                    anim->active = false;
                    if (anim->on_complete) {
                        anim->on_complete(widget);
                    }
                }
            }
        }
    }
}

/* Color Interpolation */
color_rgba_t gui_color_lerp(color_rgba_t a, color_rgba_t b, float t) {
    if (t <= 0.0f) return a;
    if (t >= 1.0f) return b;
    
    color_rgba_t result;
    result.r = (uint8_t)(a.r + (b.r - a.r) * t);
    result.g = (uint8_t)(a.g + (b.g - a.g) * t);
    result.b = (uint8_t)(a.b + (b.b - a.b) * t);
    result.a = (uint8_t)(a.a + (b.a - a.a) * t);
    
    return result;
}

/* Easing Function */
float gui_ease_in_out(float t) {
    return t * t * (3.0f - 2.0f * t);
}

/* Scale Rectangle */
rect_t gui_rect_scale(rect_t rect, float scale) {
    rect_t result;
    result.x = (int32_t)(rect.x * scale);
    result.y = (int32_t)(rect.y * scale);
    result.width = (int32_t)(rect.width * scale);
    result.height = (int32_t)(rect.height * scale);
    return result;
}

/* Point in Rectangle Test */
bool gui_point_in_rect(point2d_t point, rect_t rect) {
    return (point.x >= rect.x && point.x < rect.x + rect.width &&
            point.y >= rect.y && point.y < rect.y + rect.height);
}

/* Set Camera Position */
void gui_set_camera_position(vec3_t position) {
    if (!gui_initialized) {
        return;
    }
    
    gui_system.camera_position = position;
}

/* Move Camera */
void gui_move_camera(vec3_t delta) {
    if (!gui_initialized) {
        return;
    }
    
    gui_system.camera_velocity.x += delta.x;
    gui_system.camera_velocity.y += delta.y;
    gui_system.camera_velocity.z += delta.z;
}

/* Enable Reduced Motion */
void gui_enable_reduced_motion(bool enable) {
    if (!gui_initialized) {
        return;
    }
    
    gui_system.reduced_motion = enable;
    
    if (enable) {
        /* Disable parallax effects */
        for (int i = 0; i < MAX_GUI_LAYERS; i++) {
            gui_system.layers[i].parallax_factor *= 0.1f;
        }
    }
}

/* Test Function */
void gui_test(void) {
    if (!gui_initialized) {
        serial_puts("[NEURAL-GUI] GUI system not initialized\n");
        return;
    }
    
    serial_puts("[NEURAL-GUI] Testing Neural GUI System...\n");
    
    /* Create test button */
    rect_t button_rect = {100, 100, 200, 50};
    gui_widget_t *test_button = gui_create_button("test_button", button_rect, "Neural Interface", NULL);
    if (test_button) {
        gui_add_widget(test_button);
        serial_puts("[SUCCESS] Test button created\n");
    }
    
    /* Create test panel */
    rect_t panel_rect = {50, 200, 300, 150};
    color_rgba_t panel_color = {0x00, 0x20, 0x40, 0x80}; /* Semi-transparent dark blue */
    gui_widget_t *test_panel = gui_create_panel("test_panel", panel_rect, panel_color);
    if (test_panel) {
        gui_add_widget(test_panel);
        serial_puts("[SUCCESS] Test panel created\n");
    }
    
    serial_puts("[NEURAL-GUI] GUI test completed\n");
}