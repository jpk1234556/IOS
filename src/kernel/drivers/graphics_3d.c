/* graphics_3d.c - Brandon Media OS 3D Graphics Engine Implementation
 * Neural Parallax Rendering System with Software 3D Pipeline
 */

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <math.h>
#include "kernel/graphics_3d.h"
#include "kernel/framebuffer.h"
#include "kernel/memory.h"

/* External functions */
extern void serial_puts(const char *s);
extern void print_dec(uint64_t num);
extern uint32_t get_time_ms(void);

/* Global renderer state */
static renderer_3d_t renderer;
static bool graphics_3d_initialized = false;

/* Initialize 3D Graphics System */
int graphics_3d_init(uint32_t width, uint32_t height, uint32_t *framebuffer) {
    if (graphics_3d_initialized) {
        return 0;
    }
    
    serial_puts("[NEURAL-3D] Initializing Neural 3D Graphics Engine...\n");
    
    /* Clear renderer state */
    memset(&renderer, 0, sizeof(renderer_3d_t));
    
    /* Set basic properties */
    renderer.framebuffer = framebuffer;
    renderer.width = width;
    renderer.height = height;
    renderer.mode = RENDER_MODE_SOLID;
    
    /* Initialize Z-buffer */
    zbuffer_init(&renderer.zbuffer, width, height);
    
    /* Initialize matrices */
    renderer.world_matrix = matrix4_identity();
    renderer.view_matrix = matrix4_identity();
    renderer.projection_matrix = matrix4_perspective(60.0f, (float)width / (float)height, 0.1f, 1000.0f);
    
    /* Set rendering options */
    renderer.backface_culling = true;
    renderer.depth_testing = true;
    renderer.alpha_blending = true;
    
    /* Initialize neural effects */
    renderer.neural_matrix_mode = false;
    renderer.neural_time = 0.0f;
    renderer.neural_seed = 0x12345678;
    
    /* Reset performance counters */
    graphics_3d_reset_stats();
    
    renderer.initialized = true;
    graphics_3d_initialized = true;
    
    serial_puts("[NEURAL-3D] Neural 3D Graphics Engine initialized\n");
    return 0;
}

/* Shutdown 3D Graphics */
void graphics_3d_shutdown(void) {
    if (!graphics_3d_initialized) {
        return;
    }
    
    serial_puts("[NEURAL-3D] Shutting down Neural 3D Graphics Engine...\n");
    
    /* Cleanup Z-buffer */
    zbuffer_destroy(&renderer.zbuffer);
    
    renderer.initialized = false;
    graphics_3d_initialized = false;
    
    serial_puts("[NEURAL-3D] Neural 3D Graphics Engine shutdown complete\n");
}

/* Clear Screen and Z-buffer */
void graphics_3d_clear(uint32_t color) {
    if (!graphics_3d_initialized || !renderer.framebuffer) {
        return;
    }
    
    /* Clear framebuffer */
    for (uint32_t i = 0; i < renderer.width * renderer.height; i++) {
        renderer.framebuffer[i] = color;
    }
    
    /* Clear Z-buffer */
    zbuffer_clear(&renderer.zbuffer, 1.0f);
    
    /* Reset frame counters */
    renderer.triangles_rendered = 0;
    renderer.vertices_processed = 0;
    renderer.pixels_drawn = 0;
}

/* Vector Math Operations */
vec3_t vec3_add(vec3_t a, vec3_t b) {
    return (vec3_t){a.x + b.x, a.y + b.y, a.z + b.z};
}

vec3_t vec3_sub(vec3_t a, vec3_t b) {
    return (vec3_t){a.x - b.x, a.y - b.y, a.z - b.z};
}

vec3_t vec3_mul(vec3_t a, float scalar) {
    return (vec3_t){a.x * scalar, a.y * scalar, a.z * scalar};
}

vec3_t vec3_cross(vec3_t a, vec3_t b) {
    return (vec3_t){
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    };
}

float vec3_dot(vec3_t a, vec3_t b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

float vec3_length(vec3_t v) {
    return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
}

vec3_t vec3_normalize(vec3_t v) {
    float len = vec3_length(v);
    if (len > 0.0f) {
        return vec3_mul(v, 1.0f / len);
    }
    return (vec3_t){0.0f, 0.0f, 0.0f};
}

vec3_t vec3_lerp(vec3_t a, vec3_t b, float t) {
    return (vec3_t){
        a.x + (b.x - a.x) * t,
        a.y + (b.y - a.y) * t,
        a.z + (b.z - a.z) * t
    };
}

/* Matrix Math Operations */
matrix4_t matrix4_identity(void) {
    matrix4_t m = {0};
    m.m[0][0] = 1.0f;
    m.m[1][1] = 1.0f;
    m.m[2][2] = 1.0f;
    m.m[3][3] = 1.0f;
    return m;
}

matrix4_t matrix4_multiply(matrix4_t a, matrix4_t b) {
    matrix4_t result = {0};
    
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            for (int k = 0; k < 4; k++) {
                result.m[i][j] += a.m[i][k] * b.m[k][j];
            }
        }
    }
    
    return result;
}

matrix4_t matrix4_translate(vec3_t translation) {
    matrix4_t m = matrix4_identity();
    m.m[0][3] = translation.x;
    m.m[1][3] = translation.y;
    m.m[2][3] = translation.z;
    return m;
}

matrix4_t matrix4_rotate_y(float angle) {
    matrix4_t m = matrix4_identity();
    float c = cosf(angle);
    float s = sinf(angle);
    
    m.m[0][0] = c;
    m.m[0][2] = s;
    m.m[2][0] = -s;
    m.m[2][2] = c;
    
    return m;
}

matrix4_t matrix4_perspective(float fov, float aspect, float near_plane, float far_plane) {
    matrix4_t m = {0};
    
    float f = 1.0f / tanf(fov * DEG_TO_RAD * 0.5f);
    
    m.m[0][0] = f / aspect;
    m.m[1][1] = f;
    m.m[2][2] = (far_plane + near_plane) / (near_plane - far_plane);
    m.m[2][3] = (2.0f * far_plane * near_plane) / (near_plane - far_plane);
    m.m[3][2] = -1.0f;
    
    return m;
}

vec3_t matrix4_transform_point(matrix4_t m, vec3_t p) {
    vec4_t result;
    result.x = m.m[0][0] * p.x + m.m[0][1] * p.y + m.m[0][2] * p.z + m.m[0][3];
    result.y = m.m[1][0] * p.x + m.m[1][1] * p.y + m.m[1][2] * p.z + m.m[1][3];
    result.z = m.m[2][0] * p.x + m.m[2][1] * p.y + m.m[2][2] * p.z + m.m[2][3];
    result.w = m.m[3][0] * p.x + m.m[3][1] * p.y + m.m[3][2] * p.z + m.m[3][3];
    
    if (result.w != 0.0f) {
        return (vec3_t){result.x / result.w, result.y / result.w, result.z / result.w};
    }
    
    return (vec3_t){result.x, result.y, result.z};
}

/* Z-Buffer Operations */
void zbuffer_init(zbuffer_t *zbuffer, uint32_t width, uint32_t height) {
    zbuffer->width = width;
    zbuffer->height = height;
    zbuffer->buffer = (float *)kmalloc(width * height * sizeof(float));
    
    if (zbuffer->buffer) {
        zbuffer_clear(zbuffer, 1.0f);
    }
}

void zbuffer_clear(zbuffer_t *zbuffer, float value) {
    if (!zbuffer->buffer) {
        return;
    }
    
    uint32_t size = zbuffer->width * zbuffer->height;
    for (uint32_t i = 0; i < size; i++) {
        zbuffer->buffer[i] = value;
    }
}

void zbuffer_destroy(zbuffer_t *zbuffer) {
    if (zbuffer->buffer) {
        kfree(zbuffer->buffer);
        zbuffer->buffer = NULL;
    }
}

bool zbuffer_test(zbuffer_t *zbuffer, uint32_t x, uint32_t y, float z) {
    if (!zbuffer->buffer || x >= zbuffer->width || y >= zbuffer->height) {
        return false;
    }
    
    uint32_t index = y * zbuffer->width + x;
    return z < zbuffer->buffer[index];
}

void zbuffer_write(zbuffer_t *zbuffer, uint32_t x, uint32_t y, float z) {
    if (!zbuffer->buffer || x >= zbuffer->width || y >= zbuffer->height) {
        return;
    }
    
    uint32_t index = y * zbuffer->width + x;
    zbuffer->buffer[index] = z;
}

/* Plot 3D Pixel with Depth Testing */
void plot_pixel_3d(int32_t x, int32_t y, float z, uint32_t color, renderer_3d_t *r) {
    if (x < 0 || x >= (int32_t)r->width || y < 0 || y >= (int32_t)r->height) {
        return;
    }
    
    /* Depth test */
    if (r->depth_testing && !zbuffer_test(&r->zbuffer, x, y, z)) {
        return;
    }
    
    /* Write pixel */
    uint32_t index = y * r->width + x;
    r->framebuffer[index] = color;
    
    /* Write depth */
    if (r->depth_testing) {
        zbuffer_write(&r->zbuffer, x, y, z);
    }
    
    r->pixels_drawn++;
}

/* Rasterize Line using Bresenham's Algorithm */
void rasterize_line(vec3_t start, vec3_t end, uint32_t color, renderer_3d_t *r) {
    int32_t x0 = (int32_t)start.x;
    int32_t y0 = (int32_t)start.y;
    int32_t x1 = (int32_t)end.x;
    int32_t y1 = (int32_t)end.y;
    
    int32_t dx = abs(x1 - x0);
    int32_t dy = abs(y1 - y0);
    int32_t sx = x0 < x1 ? 1 : -1;
    int32_t sy = y0 < y1 ? 1 : -1;
    int32_t err = dx - dy;
    
    float z_step = (end.z - start.z) / sqrtf(dx * dx + dy * dy);
    float current_z = start.z;
    
    while (true) {
        plot_pixel_3d(x0, y0, current_z, color, r);
        
        if (x0 == x1 && y0 == y1) break;
        
        int32_t e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x0 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y0 += sy;
        }
        
        current_z += z_step;
    }
}

/* Create Neural Grid Mesh */
mesh_3d_t *mesh_create_neural_grid(uint32_t width, uint32_t height) {
    uint32_t vertex_count = (width + 1) * (height + 1);
    uint32_t index_count = width * height * 6; /* Two triangles per quad */
    
    mesh_3d_t *mesh = mesh_create(vertex_count, index_count);
    if (!mesh) {
        return NULL;
    }
    
    /* Generate vertices */
    uint32_t vertex_index = 0;
    for (uint32_t y = 0; y <= height; y++) {
        for (uint32_t x = 0; x <= width; x++) {
            vertex_3d_t vertex = {0};
            vertex.position.x = (float)x - (float)width * 0.5f;
            vertex.position.y = 0.0f;
            vertex.position.z = (float)y - (float)height * 0.5f;
            vertex.normal = (vec3_t){0.0f, 1.0f, 0.0f};
            vertex.texcoord.u = (float)x / (float)width;
            vertex.texcoord.v = (float)y / (float)height;
            vertex.color = COLOR_NEURAL_BLUE;
            
            mesh_set_vertex(mesh, vertex_index++, vertex);
        }
    }
    
    /* Generate indices */
    uint32_t index = 0;
    for (uint32_t y = 0; y < height; y++) {
        for (uint32_t x = 0; x < width; x++) {
            uint32_t i0 = y * (width + 1) + x;
            uint32_t i1 = i0 + 1;
            uint32_t i2 = (y + 1) * (width + 1) + x;
            uint32_t i3 = i2 + 1;
            
            /* First triangle */
            mesh_set_index(mesh, index++, i0);
            mesh_set_index(mesh, index++, i2);
            mesh_set_index(mesh, index++, i1);
            
            /* Second triangle */
            mesh_set_index(mesh, index++, i1);
            mesh_set_index(mesh, index++, i2);
            mesh_set_index(mesh, index++, i3);
        }
    }
    
    strncpy(mesh->name, "neural_grid", sizeof(mesh->name) - 1);
    return mesh;
}

/* Neural Matrix Effect */
void neural_matrix_effect(renderer_3d_t *r, float time) {
    if (!r->neural_matrix_mode) {
        return;
    }
    
    /* Create flowing matrix pattern */
    for (uint32_t y = 0; y < r->height; y += 2) {
        for (uint32_t x = 0; x < r->width; x += 8) {
            float wave = sinf(time * 0.001f + x * 0.1f + y * 0.05f);
            if (wave > 0.7f) {
                uint32_t intensity = (uint32_t)(wave * 255.0f);
                uint32_t color = (intensity << 8); /* Green channel */
                plot_pixel_3d(x, y, 0.5f, color, r);
            }
        }
    }
}

/* Color Utilities */
uint32_t color_from_rgb(uint8_t r, uint8_t g, uint8_t b) {
    return (0xFF << 24) | (r << 16) | (g << 8) | b;
}

uint32_t color_from_rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    return (a << 24) | (r << 16) | (g << 8) | b;
}

uint32_t color_lerp(uint32_t a, uint32_t b, float t) {
    if (t <= 0.0f) return a;
    if (t >= 1.0f) return b;
    
    uint8_t ar = (a >> 16) & 0xFF;
    uint8_t ag = (a >> 8) & 0xFF;
    uint8_t ab = a & 0xFF;
    uint8_t aa = (a >> 24) & 0xFF;
    
    uint8_t br = (b >> 16) & 0xFF;
    uint8_t bg = (b >> 8) & 0xFF;
    uint8_t bb = b & 0xFF;
    uint8_t ba = (b >> 24) & 0xFF;
    
    uint8_t rr = ar + (uint8_t)((br - ar) * t);
    uint8_t rg = ag + (uint8_t)((bg - ag) * t);
    uint8_t rb = ab + (uint8_t)((bb - ab) * t);
    uint8_t ra = aa + (uint8_t)((ba - aa) * t);
    
    return (ra << 24) | (rr << 16) | (rg << 8) | rb;
}

/* Get Performance Statistics */
void graphics_3d_get_stats(uint32_t *triangles, uint32_t *vertices, uint32_t *pixels, uint32_t *frame_time) {
    if (!graphics_3d_initialized) {
        return;
    }
    
    if (triangles) *triangles = renderer.triangles_rendered;
    if (vertices) *vertices = renderer.vertices_processed;
    if (pixels) *pixels = renderer.pixels_drawn;
    if (frame_time) *frame_time = renderer.frame_time_ms;
}

/* Reset Performance Statistics */
void graphics_3d_reset_stats(void) {
    if (!graphics_3d_initialized) {
        return;
    }
    
    renderer.triangles_rendered = 0;
    renderer.vertices_processed = 0;
    renderer.pixels_drawn = 0;
    renderer.frame_time_ms = 0;
}

/* Set Render Mode */
void graphics_3d_set_render_mode(render_mode_t mode) {
    if (!graphics_3d_initialized) {
        return;
    }
    
    renderer.mode = mode;
    
    if (mode == RENDER_MODE_NEURAL_MATRIX) {
        renderer.neural_matrix_mode = true;
    } else {
        renderer.neural_matrix_mode = false;
    }
}

/* Test 3D Graphics */
void graphics_3d_test(void) {
    if (!graphics_3d_initialized) {
        serial_puts("[NEURAL-3D] 3D Graphics not initialized\n");
        return;
    }
    
    serial_puts("[NEURAL-3D] Testing Neural 3D Graphics Engine...\n");
    
    /* Clear screen with dark blue */
    graphics_3d_clear(COLOR_DARK_BLUE);
    
    /* Draw test lines */
    vec3_t line_start = {100.0f, 100.0f, 0.5f};
    vec3_t line_end = {300.0f, 200.0f, 0.5f};
    rasterize_line(line_start, line_end, COLOR_NEURAL_CYAN, &renderer);
    
    /* Enable neural matrix mode */
    graphics_3d_set_render_mode(RENDER_MODE_NEURAL_MATRIX);
    neural_matrix_effect(&renderer, get_time_ms());
    
    /* Print statistics */
    uint32_t triangles, vertices, pixels, frame_time;
    graphics_3d_get_stats(&triangles, &vertices, &pixels, &frame_time);
    
    serial_puts("[STATS] Pixels drawn: ");
    print_dec(pixels);
    serial_puts("\n");
    
    serial_puts("[NEURAL-3D] 3D Graphics test completed\n");
}