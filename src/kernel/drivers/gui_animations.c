/* gui_animations.c - Brandon Media OS Neural Animation System
 * Cyberpunk Effects and Smooth Transitions
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

/* Animation Easing Functions */
static float ease_linear(float t) {
    return t;
}

static float ease_in_quad(float t) {
    return t * t;
}

static float ease_out_quad(float t) {
    return 1.0f - (1.0f - t) * (1.0f - t);
}

static float ease_in_out_quad(float t) {
    return t < 0.5f ? 2.0f * t * t : 1.0f - powf(-2.0f * t + 2.0f, 2.0f) / 2.0f;
}

static float ease_in_cubic(float t) {
    return t * t * t;
}

static float ease_out_cubic(float t) {
    return 1.0f - powf(1.0f - t, 3.0f);
}

static float ease_in_out_cubic(float t) {
    return t < 0.5f ? 4.0f * t * t * t : 1.0f - powf(-2.0f * t + 2.0f, 3.0f) / 2.0f;
}

static float ease_bounce_out(float t) {
    const float n1 = 7.5625f;
    const float d1 = 2.75f;
    
    if (t < 1.0f / d1) {
        return n1 * t * t;
    } else if (t < 2.0f / d1) {
        t -= 1.5f / d1;
        return n1 * t * t + 0.75f;
    } else if (t < 2.5f / d1) {
        t -= 2.25f / d1;
        return n1 * t * t + 0.9375f;
    } else {
        t -= 2.625f / d1;
        return n1 * t * t + 0.984375f;
    }
}

/* Get easing function by type */
static float (*get_easing_function(animation_type_t type))(float) {
    switch (type) {
        case ANIM_FADE:
            return ease_in_out_quad;
        case ANIM_SLIDE:
            return ease_out_cubic;
        case ANIM_SCALE:
            return ease_out_quad;
        case ANIM_ROTATE:
            return ease_linear;
        case ANIM_GLOW:
            return ease_in_out_quad;
        case ANIM_NEURAL_PULSE:
            return ease_in_out_cubic;
        case ANIM_MATRIX_FLOW:
            return ease_linear;
        default:
            return ease_linear;
    }
}

/* Start Animation */
void gui_start_animation(gui_widget_t *widget, animation_type_t type, uint32_t duration_ms, bool loop) {
    if (!widget) {
        return;
    }
    
    animation_t *anim = &widget->animation;
    anim->type = type;
    anim->duration_ms = duration_ms;
    anim->elapsed_ms = 0;
    anim->progress = 0.0f;
    anim->active = true;
    anim->loop = loop;
}

/* Stop Animation */
void gui_stop_animation(gui_widget_t *widget) {
    if (!widget) {
        return;
    }
    
    widget->animation.active = false;
    widget->animation.progress = 0.0f;
}

/* Apply Animation Effects */
void gui_apply_animation_effects(gui_widget_t *widget) {
    if (!widget || !widget->animation.active) {
        return;
    }
    
    animation_t *anim = &widget->animation;
    float (*easing_func)(float) = get_easing_function(anim->type);
    float eased_progress = easing_func(anim->progress);
    
    framebuffer_device_t *fb = framebuffer_get_device();
    if (!fb) {
        return;
    }
    
    switch (anim->type) {
        case ANIM_FADE: {
            /* Fade in/out effect */
            uint8_t alpha = (uint8_t)(255.0f * eased_progress);
            widget->color.a = alpha;
            widget->bg_color.a = alpha;
            break;
        }
        
        case ANIM_SLIDE: {
            /* Slide animation */
            int32_t slide_offset = (int32_t)(50.0f * (1.0f - eased_progress));
            widget->bounds.x += slide_offset;
            break;
        }
        
        case ANIM_SCALE: {
            /* Scale animation */
            float scale_factor = 0.8f + 0.2f * eased_progress;
            int32_t center_x = widget->bounds.x + widget->bounds.width / 2;
            int32_t center_y = widget->bounds.y + widget->bounds.height / 2;
            
            widget->bounds.width = (int32_t)(widget->bounds.width * scale_factor);
            widget->bounds.height = (int32_t)(widget->bounds.height * scale_factor);
            widget->bounds.x = center_x - widget->bounds.width / 2;
            widget->bounds.y = center_y - widget->bounds.height / 2;
            break;
        }
        
        case ANIM_GLOW: {
            /* Neural glow effect */
            if (widget->state == WIDGET_STATE_HOVER || widget->state == WIDGET_STATE_ACTIVE) {
                uint32_t glow_intensity = (uint32_t)(128.0f * eased_progress);
                uint32_t glow_color = fb_color_from_rgba(0x00, 0xFF, 0xFF, glow_intensity);
                
                /* Draw glow around widget */
                int32_t glow_size = (int32_t)(8.0f * eased_progress);
                fb_draw_rect(widget->bounds.x - glow_size, widget->bounds.y - glow_size,
                           widget->bounds.width + 2 * glow_size, widget->bounds.height + 2 * glow_size,
                           glow_color, 2);
            }
            break;
        }
        
        case ANIM_NEURAL_PULSE: {
            /* Neural pulse effect */
            float pulse_intensity = sinf(eased_progress * TWO_PI);
            uint8_t pulse_alpha = (uint8_t)(50.0f + 50.0f * pulse_intensity);
            
            uint32_t pulse_color = fb_color_from_rgba(widget->color.r, widget->color.g, widget->color.b, pulse_alpha);
            
            /* Draw pulsing border */
            fb_draw_rect(widget->bounds.x - 1, widget->bounds.y - 1,
                        widget->bounds.width + 2, widget->bounds.height + 2,
                        pulse_color, 1);
            break;
        }
        
        case ANIM_MATRIX_FLOW: {
            /* Matrix flow effect for neural matrix widgets */
            if (widget->type == WIDGET_NEURAL_MATRIX && widget->data) {
                neural_matrix_data_t *matrix_data = (neural_matrix_data_t *)widget->data;
                
                /* Create flowing effect */
                for (uint32_t y = 0; y < matrix_data->matrix_height; y++) {
                    for (uint32_t x = 0; x < matrix_data->matrix_width; x++) {
                        uint32_t index = y * matrix_data->matrix_width + x;
                        
                        /* Add flowing animation */
                        float flow_offset = sinf(eased_progress * TWO_PI + x * 0.2f + y * 0.1f);
                        matrix_data->data_matrix[index] += flow_offset * 0.1f;
                        
                        /* Clamp values */
                        if (matrix_data->data_matrix[index] < 0.0f) matrix_data->data_matrix[index] = 0.0f;
                        if (matrix_data->data_matrix[index] > 1.0f) matrix_data->data_matrix[index] = 1.0f;
                    }
                }
            }
            break;
        }
        
        default:
            break;
    }
}

/* Neural Particle System */
typedef struct {
    float x, y, z;
    float vx, vy, vz;
    float life;
    float max_life;
    uint32_t color;
    bool active;
} neural_particle_t;

#define MAX_PARTICLES 100
static neural_particle_t particles[MAX_PARTICLES];
static bool particle_system_initialized = false;

/* Initialize Particle System */
void neural_particle_system_init(void) {
    if (particle_system_initialized) {
        return;
    }
    
    memset(particles, 0, sizeof(particles));
    particle_system_initialized = true;
    
    serial_puts("[NEURAL-ANIM] Particle system initialized\n");
}

/* Spawn Neural Particle */
void neural_spawn_particle(float x, float y, float z, uint32_t color) {
    if (!particle_system_initialized) {
        neural_particle_system_init();
    }
    
    /* Find inactive particle */
    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (!particles[i].active) {
            neural_particle_t *p = &particles[i];
            
            p->x = x;
            p->y = y;
            p->z = z;
            p->vx = ((float)(rand() % 200) - 100.0f) / 100.0f;
            p->vy = ((float)(rand() % 200) - 100.0f) / 100.0f;
            p->vz = ((float)(rand() % 100)) / 100.0f;
            p->life = 2.0f + ((float)(rand() % 100)) / 100.0f;
            p->max_life = p->life;
            p->color = color;
            p->active = true;
            break;
        }
    }
}

/* Update Particle System */
void neural_particle_system_update(float delta_time) {
    if (!particle_system_initialized) {
        return;
    }
    
    for (int i = 0; i < MAX_PARTICLES; i++) {
        neural_particle_t *p = &particles[i];
        
        if (!p->active) {
            continue;
        }
        
        /* Update position */
        p->x += p->vx * delta_time;
        p->y += p->vy * delta_time;
        p->z += p->vz * delta_time;
        
        /* Update life */
        p->life -= delta_time;
        
        if (p->life <= 0.0f) {
            p->active = false;
        }
    }
}

/* Render Particle System */
void neural_particle_system_render(void) {
    if (!particle_system_initialized) {
        return;
    }
    
    framebuffer_device_t *fb = framebuffer_get_device();
    if (!fb) {
        return;
    }
    
    for (int i = 0; i < MAX_PARTICLES; i++) {
        neural_particle_t *p = &particles[i];
        
        if (!p->active) {
            continue;
        }
        
        /* Calculate alpha based on life */
        float life_ratio = p->life / p->max_life;
        uint8_t alpha = (uint8_t)(255.0f * life_ratio);
        
        /* Extract color components */
        uint8_t r = (p->color >> 16) & 0xFF;
        uint8_t g = (p->color >> 8) & 0xFF;
        uint8_t b = p->color & 0xFF;
        
        uint32_t particle_color = fb_color_from_rgba(r, g, b, alpha);
        
        /* Render particle */
        int32_t screen_x = (int32_t)p->x;
        int32_t screen_y = (int32_t)p->y;
        
        if (screen_x >= 0 && screen_x < (int32_t)fb->width &&
            screen_y >= 0 && screen_y < (int32_t)fb->height) {
            fb_put_pixel(screen_x, screen_y, particle_color);
        }
    }
}

/* Cyberpunk Scanline Effect */
void cyberpunk_scanlines_effect(uint32_t intensity) {
    framebuffer_device_t *fb = framebuffer_get_device();
    if (!fb) {
        return;
    }
    
    uint32_t scanline_color = fb_color_from_rgba(0x00, 0xFF, 0xFF, intensity);
    
    /* Draw horizontal scanlines */
    for (uint32_t y = 0; y < fb->height; y += 4) {
        for (uint32_t x = 0; x < fb->width; x++) {
            fb_put_pixel_alpha(x, y, scanline_color, intensity);
        }
    }
}

/* Neural Network Visualization */
void neural_network_effect(int32_t center_x, int32_t center_y, float time) {
    framebuffer_device_t *fb = framebuffer_get_device();
    if (!fb) {
        return;
    }
    
    /* Draw neural nodes */
    for (int i = 0; i < 8; i++) {
        float angle = (i * TWO_PI / 8.0f) + time * 0.001f;
        int32_t node_x = center_x + (int32_t)(80.0f * cosf(angle));
        int32_t node_y = center_y + (int32_t)(80.0f * sinf(angle));
        
        /* Pulsing node */
        float pulse = 0.5f + 0.5f * sinf(time * 0.002f + i);
        uint8_t intensity = (uint8_t)(128.0f + 127.0f * pulse);
        uint32_t node_color = fb_color_from_rgba(0x00, intensity, intensity, 0xFF);
        
        fb_fill_circle(node_x, node_y, 5, node_color);
        
        /* Draw connections */
        for (int j = i + 1; j < 8; j++) {
            float other_angle = (j * TWO_PI / 8.0f) + time * 0.001f;
            int32_t other_x = center_x + (int32_t)(80.0f * cosf(other_angle));
            int32_t other_y = center_y + (int32_t)(80.0f * sinf(other_angle));
            
            uint32_t connection_color = fb_color_from_rgba(0x00, 0x80, 0xFF, 0x60);
            fb_draw_line(node_x, node_y, other_x, other_y, connection_color);
        }
    }
    
    /* Central processing node */
    float central_pulse = 0.3f + 0.7f * sinf(time * 0.003f);
    uint8_t central_intensity = (uint8_t)(200.0f * central_pulse);
    uint32_t central_color = fb_color_from_rgba(0xFF, central_intensity, 0x00, 0xFF);
    fb_fill_circle(center_x, center_y, 8, central_color);
}

/* Holographic Border Effect */
void holographic_border_effect(rect_t bounds, float time) {
    framebuffer_device_t *fb = framebuffer_get_device();
    if (!fb) {
        return;
    }
    
    /* Animated border colors */
    float hue_shift = time * 0.001f;
    
    for (int32_t i = 0; i < bounds.width; i++) {
        float progress = (float)i / (float)bounds.width;
        float hue = hue_shift + progress * TWO_PI;
        
        uint8_t r = (uint8_t)(128.0f + 127.0f * sinf(hue));
        uint8_t g = (uint8_t)(128.0f + 127.0f * sinf(hue + TWO_PI / 3.0f));
        uint8_t b = (uint8_t)(128.0f + 127.0f * sinf(hue + 4.0f * TWO_PI / 3.0f));
        
        uint32_t color = fb_color_from_rgba(r, g, b, 0xFF);
        
        /* Top and bottom borders */
        fb_put_pixel(bounds.x + i, bounds.y, color);
        fb_put_pixel(bounds.x + i, bounds.y + bounds.height - 1, color);
    }
    
    for (int32_t i = 0; i < bounds.height; i++) {
        float progress = (float)i / (float)bounds.height;
        float hue = hue_shift + progress * TWO_PI;
        
        uint8_t r = (uint8_t)(128.0f + 127.0f * sinf(hue));
        uint8_t g = (uint8_t)(128.0f + 127.0f * sinf(hue + TWO_PI / 3.0f));
        uint8_t b = (uint8_t)(128.0f + 127.0f * sinf(hue + 4.0f * TWO_PI / 3.0f));
        
        uint32_t color = fb_color_from_rgba(r, g, b, 0xFF);
        
        /* Left and right borders */
        fb_put_pixel(bounds.x, bounds.y + i, color);
        fb_put_pixel(bounds.x + bounds.width - 1, bounds.y + i, color);
    }
}

/* Test Animation System */
void gui_animation_test(void) {
    serial_puts("[NEURAL-ANIM] Testing Neural Animation System...\n");
    
    /* Initialize particle system */
    neural_particle_system_init();
    
    /* Spawn test particles */
    for (int i = 0; i < 10; i++) {
        neural_spawn_particle(100.0f + i * 10.0f, 100.0f, 0.0f, COLOR_NEURAL_CYAN);
    }
    
    serial_puts("[SUCCESS] Neural Animation System test completed\n");
}