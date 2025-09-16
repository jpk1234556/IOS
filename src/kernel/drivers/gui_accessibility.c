/* gui_accessibility.c - Brandon Media OS Neural GUI Accessibility System
 * WCAG 2.1 Compliant Accessibility Features with Reduced Motion Support
 */

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <math.h>
#include "kernel/gui.h"
#include "kernel/framebuffer.h"
#include "kernel/input.h"
#include "kernel/memory.h"

/* External functions */
extern void serial_puts(const char *s);
extern uint32_t get_time_ms(void);

/* Accessibility Settings */
typedef struct {
    /* Motion */
    bool reduced_motion_enabled;
    bool animation_disabled;
    float motion_reduction_factor;
    
    /* Visual */
    bool high_contrast_enabled;
    bool large_text_enabled;
    bool focus_indicators_enhanced;
    float ui_scale_factor;
    uint32_t focus_color;
    
    /* Audio */
    bool screen_reader_mode;
    bool sound_feedback_enabled;
    
    /* Input */
    bool sticky_keys_enabled;
    bool slow_keys_enabled;
    bool mouse_keys_enabled;
    uint32_t key_repeat_delay;
    uint32_t key_repeat_rate;
    
    /* Navigation */
    bool keyboard_navigation_enabled;
    bool tab_order_enforced;
    gui_widget_t *focused_widget;
    gui_widget_t **tab_order;
    uint32_t tab_order_count;
    uint32_t current_tab_index;
    
    /* Color and Contrast */
    float contrast_ratio;
    color_rgba_t high_contrast_bg;
    color_rgba_t high_contrast_fg;
    color_rgba_t high_contrast_focus;
    
    bool initialized;
} accessibility_system_t;

static accessibility_system_t accessibility;

/* WCAG 2.1 Color Contrast Standards */
#define WCAG_AA_NORMAL_CONTRAST   4.5f
#define WCAG_AA_LARGE_CONTRAST    3.0f
#define WCAG_AAA_NORMAL_CONTRAST  7.0f
#define WCAG_AAA_LARGE_CONTRAST   4.5f

/* Initialize Accessibility System */
int accessibility_init(void) {
    if (accessibility.initialized) {
        return 0;
    }
    
    serial_puts("[NEURAL-A11Y] Initializing Neural Accessibility System...\n");
    
    /* Clear accessibility state */
    memset(&accessibility, 0, sizeof(accessibility_system_t));
    
    /* Set default accessibility settings */
    accessibility.reduced_motion_enabled = false;
    accessibility.animation_disabled = false;
    accessibility.motion_reduction_factor = 1.0f;
    
    accessibility.high_contrast_enabled = false;
    accessibility.large_text_enabled = false;
    accessibility.focus_indicators_enhanced = true;
    accessibility.ui_scale_factor = 1.0f;
    accessibility.focus_color = fb_color_from_rgba(0xFF, 0xFF, 0x00, 0xFF); /* Yellow focus */
    
    accessibility.screen_reader_mode = false;
    accessibility.sound_feedback_enabled = false;
    
    accessibility.sticky_keys_enabled = false;
    accessibility.slow_keys_enabled = false;
    accessibility.mouse_keys_enabled = false;
    accessibility.key_repeat_delay = 500; /* 500ms */
    accessibility.key_repeat_rate = 30;   /* 30 per second */
    
    accessibility.keyboard_navigation_enabled = true;
    accessibility.tab_order_enforced = true;
    accessibility.focused_widget = NULL;
    accessibility.tab_order = NULL;
    accessibility.tab_order_count = 0;
    accessibility.current_tab_index = 0;
    
    /* Set high contrast colors */
    accessibility.contrast_ratio = WCAG_AA_NORMAL_CONTRAST;
    accessibility.high_contrast_bg = (color_rgba_t){0x00, 0x00, 0x00, 0xFF}; /* Black */
    accessibility.high_contrast_fg = (color_rgba_t){0xFF, 0xFF, 0xFF, 0xFF}; /* White */
    accessibility.high_contrast_focus = (color_rgba_t){0xFF, 0xFF, 0x00, 0xFF}; /* Yellow */
    
    accessibility.initialized = true;
    
    serial_puts("[NEURAL-A11Y] Neural Accessibility System initialized\n");
    return 0;
}

/* Shutdown Accessibility System */
void accessibility_shutdown(void) {
    if (!accessibility.initialized) {
        return;
    }
    
    serial_puts("[NEURAL-A11Y] Shutting down Neural Accessibility System...\n");
    
    /* Free tab order if allocated */
    if (accessibility.tab_order) {
        kfree(accessibility.tab_order);
        accessibility.tab_order = NULL;
    }
    
    accessibility.initialized = false;
    
    serial_puts("[NEURAL-A11Y] Neural Accessibility System shutdown complete\n");
}

/* Enable/Disable Reduced Motion */
void accessibility_set_reduced_motion(bool enabled) {
    if (!accessibility.initialized) {
        return;
    }
    
    accessibility.reduced_motion_enabled = enabled;
    
    if (enabled) {
        accessibility.motion_reduction_factor = 0.1f; /* Reduce motion to 10% */
        accessibility.animation_disabled = true;
        serial_puts("[A11Y] Reduced motion enabled\n");
    } else {
        accessibility.motion_reduction_factor = 1.0f;
        accessibility.animation_disabled = false;
        serial_puts("[A11Y] Reduced motion disabled\n");
    }
    
    /* Apply to existing GUI system */
    extern void gui_enable_reduced_motion(bool enable);
    gui_enable_reduced_motion(enabled);
}

/* Enable/Disable High Contrast Mode */
void accessibility_set_high_contrast(bool enabled) {
    if (!accessibility.initialized) {
        return;
    }
    
    accessibility.high_contrast_enabled = enabled;
    
    if (enabled) {
        serial_puts("[A11Y] High contrast mode enabled\n");
        /* Apply high contrast colors to GUI system */
        extern void gui_set_theme_colors(color_rgba_t primary, color_rgba_t secondary, color_rgba_t accent, color_rgba_t background);
        gui_set_theme_colors(accessibility.high_contrast_fg, 
                           accessibility.high_contrast_fg,
                           accessibility.high_contrast_focus,
                           accessibility.high_contrast_bg);
    } else {
        serial_puts("[A11Y] High contrast mode disabled\n");
        /* Restore default cyberpunk theme */
        color_rgba_t neural_blue = GUI_COLOR_NEURAL_BLUE;
        color_rgba_t neural_cyan = GUI_COLOR_NEURAL_CYAN;
        color_rgba_t neural_white = GUI_COLOR_NEURAL_WHITE;
        color_rgba_t dark_blue = GUI_COLOR_DARK_BLUE;
        gui_set_theme_colors(neural_blue, neural_cyan, neural_white, dark_blue);
    }
}

/* Set UI Scale Factor */
void accessibility_set_ui_scale(float scale) {
    if (!accessibility.initialized) {
        return;
    }
    
    /* Clamp scale between 0.5 and 3.0 */
    if (scale < 0.5f) scale = 0.5f;
    if (scale > 3.0f) scale = 3.0f;
    
    accessibility.ui_scale_factor = scale;
    accessibility.large_text_enabled = (scale > 1.2f);
    
    serial_puts("[A11Y] UI scale factor set to: ");
    /* Note: We would print the scale value here if we had float printing */
    serial_puts("\n");
    
    /* Apply to GUI system */
    extern void gui_set_ui_scale(float scale);
    gui_set_ui_scale(scale);
}

/* Calculate Color Luminance (WCAG 2.1) */
static float calculate_luminance(color_rgba_t color) {
    /* Convert to linear RGB */
    float r = color.r / 255.0f;
    float g = color.g / 255.0f;
    float b = color.b / 255.0f;
    
    /* Apply gamma correction */
    r = (r <= 0.03928f) ? r / 12.92f : powf((r + 0.055f) / 1.055f, 2.4f);
    g = (g <= 0.03928f) ? g / 12.92f : powf((g + 0.055f) / 1.055f, 2.4f);
    b = (b <= 0.03928f) ? b / 12.92f : powf((b + 0.055f) / 1.055f, 2.4f);
    
    /* Calculate luminance */
    return 0.2126f * r + 0.7152f * g + 0.0722f * b;
}

/* Calculate Contrast Ratio (WCAG 2.1) */
float accessibility_calculate_contrast_ratio(color_rgba_t color1, color_rgba_t color2) {
    float lum1 = calculate_luminance(color1);
    float lum2 = calculate_luminance(color2);
    
    /* Ensure lighter color is in numerator */
    if (lum1 < lum2) {
        float temp = lum1;
        lum1 = lum2;
        lum2 = temp;
    }
    
    return (lum1 + 0.05f) / (lum2 + 0.05f);
}

/* Check if Colors Meet WCAG Standards */
bool accessibility_check_contrast_compliance(color_rgba_t fg, color_rgba_t bg, bool large_text) {
    float ratio = accessibility_calculate_contrast_ratio(fg, bg);
    float required_ratio = large_text ? WCAG_AA_LARGE_CONTRAST : WCAG_AA_NORMAL_CONTRAST;
    
    return ratio >= required_ratio;
}

/* Adjust Colors for Better Contrast */
color_rgba_t accessibility_improve_contrast(color_rgba_t fg, color_rgba_t bg, bool large_text) {
    float current_ratio = accessibility_calculate_contrast_ratio(fg, bg);
    float required_ratio = large_text ? WCAG_AA_LARGE_CONTRAST : WCAG_AA_NORMAL_CONTRAST;
    
    if (current_ratio >= required_ratio) {
        return fg; /* Already compliant */
    }
    
    /* Simple approach: make foreground more extreme (lighter or darker) */
    color_rgba_t improved_fg = fg;
    float bg_lum = calculate_luminance(bg);
    
    if (bg_lum > 0.5f) {
        /* Light background, make foreground darker */
        improved_fg.r = (uint8_t)(improved_fg.r * 0.5f);
        improved_fg.g = (uint8_t)(improved_fg.g * 0.5f);
        improved_fg.b = (uint8_t)(improved_fg.b * 0.5f);
    } else {
        /* Dark background, make foreground lighter */
        improved_fg.r = 255 - (uint8_t)((255 - improved_fg.r) * 0.5f);
        improved_fg.g = 255 - (uint8_t)((255 - improved_fg.g) * 0.5f);
        improved_fg.b = 255 - (uint8_t)((255 - improved_fg.b) * 0.5f);
    }
    
    return improved_fg;
}

/* Keyboard Navigation Functions */
void accessibility_build_tab_order(gui_widget_t **widgets, uint32_t widget_count) {
    if (!accessibility.initialized) {
        return;
    }
    
    /* Free existing tab order */
    if (accessibility.tab_order) {
        kfree(accessibility.tab_order);
    }
    
    /* Count interactive widgets */
    uint32_t interactive_count = 0;
    for (uint32_t i = 0; i < widget_count; i++) {
        if (widgets[i] && widgets[i]->interactive && widgets[i]->visible) {
            interactive_count++;
        }
    }
    
    if (interactive_count == 0) {
        accessibility.tab_order = NULL;
        accessibility.tab_order_count = 0;
        return;
    }
    
    /* Allocate tab order array */
    accessibility.tab_order = (gui_widget_t **)kmalloc(interactive_count * sizeof(gui_widget_t *));
    if (!accessibility.tab_order) {
        return;
    }
    
    /* Build tab order (left-to-right, top-to-bottom) */
    uint32_t tab_index = 0;
    for (uint32_t i = 0; i < widget_count; i++) {
        gui_widget_t *widget = widgets[i];
        if (widget && widget->interactive && widget->visible) {
            accessibility.tab_order[tab_index++] = widget;
        }
    }
    
    accessibility.tab_order_count = interactive_count;
    accessibility.current_tab_index = 0;
    
    serial_puts("[A11Y] Tab order built with ");
    /* Print widget count here */
    serial_puts(" interactive widgets\n");
}

/* Navigate to Next Widget */
void accessibility_tab_next(void) {
    if (!accessibility.initialized || !accessibility.keyboard_navigation_enabled ||
        !accessibility.tab_order || accessibility.tab_order_count == 0) {
        return;
    }
    
    /* Remove focus from current widget */
    if (accessibility.focused_widget) {
        accessibility.focused_widget->state = WIDGET_STATE_NORMAL;
    }
    
    /* Move to next widget */
    accessibility.current_tab_index = (accessibility.current_tab_index + 1) % accessibility.tab_order_count;
    accessibility.focused_widget = accessibility.tab_order[accessibility.current_tab_index];
    
    /* Set focus state */
    if (accessibility.focused_widget) {
        accessibility.focused_widget->state = WIDGET_STATE_HOVER; /* Use hover as focus indicator */
    }
}

/* Navigate to Previous Widget */
void accessibility_tab_previous(void) {
    if (!accessibility.initialized || !accessibility.keyboard_navigation_enabled ||
        !accessibility.tab_order || accessibility.tab_order_count == 0) {
        return;
    }
    
    /* Remove focus from current widget */
    if (accessibility.focused_widget) {
        accessibility.focused_widget->state = WIDGET_STATE_NORMAL;
    }
    
    /* Move to previous widget */
    if (accessibility.current_tab_index == 0) {
        accessibility.current_tab_index = accessibility.tab_order_count - 1;
    } else {
        accessibility.current_tab_index--;
    }
    accessibility.focused_widget = accessibility.tab_order[accessibility.current_tab_index];
    
    /* Set focus state */
    if (accessibility.focused_widget) {
        accessibility.focused_widget->state = WIDGET_STATE_HOVER;
    }
}

/* Activate Focused Widget */
void accessibility_activate_focused_widget(void) {
    if (!accessibility.initialized || !accessibility.focused_widget) {
        return;
    }
    
    gui_widget_t *widget = accessibility.focused_widget;
    
    /* Trigger click handler */
    if (widget->on_click) {
        widget->state = WIDGET_STATE_ACTIVE;
        widget->on_click(widget);
        
        /* Provide audio feedback if enabled */
        if (accessibility.sound_feedback_enabled) {
            /* Beep sound would go here */
            serial_puts("[A11Y] Widget activated\n");
        }
    }
}

/* Enhanced Focus Indicators */
void accessibility_draw_focus_indicator(gui_widget_t *widget) {
    if (!accessibility.initialized || !widget || !accessibility.focus_indicators_enhanced) {
        return;
    }
    
    if (widget == accessibility.focused_widget) {
        framebuffer_device_t *fb = framebuffer_get_device();
        if (!fb) {
            return;
        }
        
        /* Draw enhanced focus border */
        uint32_t focus_color = accessibility.focus_color;
        
        /* Double border for better visibility */
        fb_draw_rect(widget->bounds.x - 3, widget->bounds.y - 3,
                    widget->bounds.width + 6, widget->bounds.height + 6,
                    focus_color, 2);
        
        /* Inner dotted line (simulated) */
        for (int32_t i = 0; i < widget->bounds.width; i += 4) {
            fb_put_pixel(widget->bounds.x + i, widget->bounds.y - 1, focus_color);
            fb_put_pixel(widget->bounds.x + i, widget->bounds.y + widget->bounds.height, focus_color);
        }
        
        for (int32_t i = 0; i < widget->bounds.height; i += 4) {
            fb_put_pixel(widget->bounds.x - 1, widget->bounds.y + i, focus_color);
            fb_put_pixel(widget->bounds.x + widget->bounds.width, widget->bounds.y + i, focus_color);
        }
    }
}

/* Input Event Handler for Accessibility */
void accessibility_handle_input_event(input_event_t *event) {
    if (!accessibility.initialized) {
        return;
    }
    
    switch (event->type) {
        case INPUT_EVENT_KEY_PRESS:
            if (accessibility.keyboard_navigation_enabled) {
                switch (event->data.key.key) {
                    case KEY_TAB:
                        if (event->data.key.shift) {
                            accessibility_tab_previous();
                        } else {
                            accessibility_tab_next();
                        }
                        break;
                        
                    case KEY_ENTER:
                    case KEY_SPACE:
                        accessibility_activate_focused_widget();
                        break;
                        
                    case KEY_ESCAPE:
                        /* Clear focus */
                        if (accessibility.focused_widget) {
                            accessibility.focused_widget->state = WIDGET_STATE_NORMAL;
                            accessibility.focused_widget = NULL;
                        }
                        break;
                        
                    default:
                        break;
                }
            }
            break;
            
        default:
            break;
    }
}

/* Get Accessibility Settings */
void accessibility_get_settings(bool *reduced_motion, bool *high_contrast, float *ui_scale) {
    if (!accessibility.initialized) {
        return;
    }
    
    if (reduced_motion) *reduced_motion = accessibility.reduced_motion_enabled;
    if (high_contrast) *high_contrast = accessibility.high_contrast_enabled;
    if (ui_scale) *ui_scale = accessibility.ui_scale_factor;
}

/* Check if Motion Should Be Reduced */
bool accessibility_should_reduce_motion(void) {
    return accessibility.initialized && accessibility.reduced_motion_enabled;
}

/* Get Motion Reduction Factor */
float accessibility_get_motion_factor(void) {
    if (!accessibility.initialized) {
        return 1.0f;
    }
    
    return accessibility.motion_reduction_factor;
}

/* Test Accessibility System */
void accessibility_test(void) {
    if (!accessibility.initialized) {
        serial_puts("[NEURAL-A11Y] Accessibility system not initialized\n");
        return;
    }
    
    serial_puts("[NEURAL-A11Y] Testing Neural Accessibility System...\n");
    
    /* Test contrast ratio calculation */
    color_rgba_t white = {0xFF, 0xFF, 0xFF, 0xFF};
    color_rgba_t black = {0x00, 0x00, 0x00, 0xFF};
    float ratio = accessibility_calculate_contrast_ratio(white, black);
    
    serial_puts("[TEST] White/Black contrast ratio: ");
    /* Print ratio value here */
    serial_puts(" (should be 21:1)\n");
    
    /* Test WCAG compliance */
    bool compliant = accessibility_check_contrast_compliance(white, black, false);
    serial_puts("[TEST] WCAG AA compliance: ");
    serial_puts(compliant ? "PASS" : "FAIL");
    serial_puts("\n");
    
    /* Test accessibility settings */
    accessibility_set_reduced_motion(true);
    accessibility_set_high_contrast(true);
    accessibility_set_ui_scale(1.5f);
    
    serial_puts("[NEURAL-A11Y] Accessibility test completed\n");
}