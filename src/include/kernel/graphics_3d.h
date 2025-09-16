/* graphics_3d.h - Brandon Media OS 3D Graphics Engine
 * Neural Parallax Rendering System
 */

#ifndef KERNEL_GRAPHICS_3D_H
#define KERNEL_GRAPHICS_3D_H

#include <stdint.h>
#include <stdbool.h>
#include <math.h>

/* 3D Vector Operations */
typedef struct {
    float x, y, z;
} vec3_t;

typedef struct {
    float x, y, z, w;
} vec4_t;

/* 3D Matrix (4x4) */
typedef struct {
    float m[4][4];
} matrix4_t;

/* 3D Vertex */
typedef struct {
    vec3_t position;
    vec3_t normal;
    struct { float u, v; } texcoord;
    uint32_t color;
} vertex_3d_t;

/* 3D Triangle */
typedef struct {
    vertex_3d_t vertices[3];
    uint32_t material_id;
} triangle_3d_t;

/* 3D Mesh */
typedef struct {
    vertex_3d_t *vertices;
    uint32_t *indices;
    uint32_t vertex_count;
    uint32_t index_count;
    uint32_t material_id;
    char name[64];
} mesh_3d_t;

/* Material Properties */
typedef struct {
    uint32_t diffuse_color;
    uint32_t specular_color;
    uint32_t emissive_color;
    float shininess;
    float transparency;
    bool wireframe;
    char name[64];
} material_3d_t;

/* 3D Camera */
typedef struct {
    vec3_t position;
    vec3_t target;
    vec3_t up;
    float fov;           /* Field of view in degrees */
    float aspect_ratio;
    float near_plane;
    float far_plane;
    matrix4_t view_matrix;
    matrix4_t projection_matrix;
} camera_3d_t;

/* Light Types */
typedef enum {
    LIGHT_DIRECTIONAL,
    LIGHT_POINT,
    LIGHT_SPOT
} light_type_t;

/* 3D Light */
typedef struct {
    light_type_t type;
    vec3_t position;
    vec3_t direction;
    uint32_t color;
    float intensity;
    float range;         /* For point/spot lights */
    float spot_angle;    /* For spot lights */
    bool enabled;
} light_3d_t;

/* Render Modes */
typedef enum {
    RENDER_MODE_WIREFRAME,
    RENDER_MODE_SOLID,
    RENDER_MODE_TEXTURED,
    RENDER_MODE_NEURAL_MATRIX
} render_mode_t;

/* Z-Buffer */
typedef struct {
    float *buffer;
    uint32_t width;
    uint32_t height;
} zbuffer_t;

/* Parallax Layer 3D */
typedef struct {
    mesh_3d_t **meshes;
    uint32_t mesh_count;
    vec3_t layer_offset;
    float parallax_factor;
    float depth_scale;
    bool visible;
    uint8_t opacity;
} parallax_layer_3d_t;

/* 3D Scene */
typedef struct {
    mesh_3d_t **meshes;
    uint32_t mesh_count;
    material_3d_t *materials;
    uint32_t material_count;
    light_3d_t *lights;
    uint32_t light_count;
    camera_3d_t camera;
    parallax_layer_3d_t layers[8];
    uint32_t layer_count;
    vec3_t background_color;
} scene_3d_t;

/* Renderer State */
typedef struct {
    uint32_t *framebuffer;
    zbuffer_t zbuffer;
    uint32_t width;
    uint32_t height;
    render_mode_t mode;
    
    /* Matrices */
    matrix4_t world_matrix;
    matrix4_t view_matrix;
    matrix4_t projection_matrix;
    matrix4_t mvp_matrix;
    
    /* Culling and Clipping */
    bool backface_culling;
    bool depth_testing;
    bool alpha_blending;
    
    /* Neural Effects */
    bool neural_matrix_mode;
    float neural_time;
    uint32_t neural_seed;
    
    /* Performance Counters */
    uint32_t triangles_rendered;
    uint32_t vertices_processed;
    uint32_t pixels_drawn;
    uint32_t frame_time_ms;
    
    bool initialized;
} renderer_3d_t;

/* Core 3D Functions */
int graphics_3d_init(uint32_t width, uint32_t height, uint32_t *framebuffer);
void graphics_3d_shutdown(void);
void graphics_3d_clear(uint32_t color);
void graphics_3d_present(void);
void graphics_3d_set_viewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height);

/* Vector Math */
vec3_t vec3_add(vec3_t a, vec3_t b);
vec3_t vec3_sub(vec3_t a, vec3_t b);
vec3_t vec3_mul(vec3_t a, float scalar);
vec3_t vec3_cross(vec3_t a, vec3_t b);
float vec3_dot(vec3_t a, vec3_t b);
float vec3_length(vec3_t v);
vec3_t vec3_normalize(vec3_t v);
vec3_t vec3_lerp(vec3_t a, vec3_t b, float t);

/* Matrix Math */
matrix4_t matrix4_identity(void);
matrix4_t matrix4_multiply(matrix4_t a, matrix4_t b);
matrix4_t matrix4_translate(vec3_t translation);
matrix4_t matrix4_rotate_x(float angle);
matrix4_t matrix4_rotate_y(float angle);
matrix4_t matrix4_rotate_z(float angle);
matrix4_t matrix4_scale(vec3_t scale);
matrix4_t matrix4_perspective(float fov, float aspect, float near_plane, float far_plane);
matrix4_t matrix4_look_at(vec3_t eye, vec3_t target, vec3_t up);
vec3_t matrix4_transform_point(matrix4_t m, vec3_t p);

/* Camera Functions */
void camera_3d_init(camera_3d_t *camera, vec3_t position, vec3_t target, float fov);
void camera_3d_update(camera_3d_t *camera);
void camera_3d_move(camera_3d_t *camera, vec3_t delta);
void camera_3d_rotate(camera_3d_t *camera, float pitch, float yaw);
void camera_3d_set_projection(camera_3d_t *camera, float fov, float aspect, float near_plane, float far_plane);

/* Mesh Functions */
mesh_3d_t *mesh_create(uint32_t vertex_count, uint32_t index_count);
void mesh_destroy(mesh_3d_t *mesh);
void mesh_set_vertex(mesh_3d_t *mesh, uint32_t index, vertex_3d_t vertex);
void mesh_set_index(mesh_3d_t *mesh, uint32_t index, uint32_t vertex_index);
mesh_3d_t *mesh_create_cube(float size);
mesh_3d_t *mesh_create_plane(float width, float height);
mesh_3d_t *mesh_create_neural_grid(uint32_t width, uint32_t height);

/* Material Functions */
material_3d_t *material_create(const char *name, uint32_t diffuse_color);
void material_destroy(material_3d_t *material);
void material_set_properties(material_3d_t *material, uint32_t diffuse, uint32_t specular, float shininess);

/* Light Functions */
light_3d_t *light_create(light_type_t type, vec3_t position, uint32_t color);
void light_destroy(light_3d_t *light);
void light_set_direction(light_3d_t *light, vec3_t direction);
void light_set_intensity(light_3d_t *light, float intensity);

/* Scene Functions */
scene_3d_t *scene_create(void);
void scene_destroy(scene_3d_t *scene);
void scene_add_mesh(scene_3d_t *scene, mesh_3d_t *mesh);
void scene_add_material(scene_3d_t *scene, material_3d_t *material);
void scene_add_light(scene_3d_t *scene, light_3d_t *light);
void scene_set_camera(scene_3d_t *scene, camera_3d_t camera);

/* Parallax Layer Functions */
void parallax_layer_init(parallax_layer_3d_t *layer, float parallax_factor);
void parallax_layer_add_mesh(parallax_layer_3d_t *layer, mesh_3d_t *mesh);
void parallax_layer_update(parallax_layer_3d_t *layer, vec3_t camera_movement);
void parallax_layer_render(parallax_layer_3d_t *layer, renderer_3d_t *renderer);

/* Rendering Functions */
void render_scene(scene_3d_t *scene, renderer_3d_t *renderer);
void render_mesh(mesh_3d_t *mesh, material_3d_t *material, renderer_3d_t *renderer);
void render_triangle(triangle_3d_t *triangle, renderer_3d_t *renderer);
void render_line_3d(vec3_t start, vec3_t end, uint32_t color, renderer_3d_t *renderer);
void render_point_3d(vec3_t position, uint32_t color, renderer_3d_t *renderer);

/* Neural Effects */
void neural_matrix_effect(renderer_3d_t *renderer, float time);
void neural_parallax_layers(scene_3d_t *scene, vec3_t camera_movement);
uint32_t neural_color_pulse(uint32_t base_color, float time, float frequency);
void neural_grid_animation(mesh_3d_t *grid, float time);

/* Culling and Clipping */
bool frustum_cull_triangle(triangle_3d_t *triangle, camera_3d_t *camera);
bool backface_cull_triangle(triangle_3d_t *triangle, vec3_t view_direction);
triangle_3d_t *clip_triangle_to_plane(triangle_3d_t *triangle, vec4_t plane);

/* Rasterization */
void rasterize_triangle(triangle_3d_t *triangle, renderer_3d_t *renderer);
void rasterize_line(vec3_t start, vec3_t end, uint32_t color, renderer_3d_t *renderer);
void plot_pixel_3d(int32_t x, int32_t y, float z, uint32_t color, renderer_3d_t *renderer);

/* Z-Buffer Functions */
void zbuffer_init(zbuffer_t *zbuffer, uint32_t width, uint32_t height);
void zbuffer_clear(zbuffer_t *zbuffer, float value);
void zbuffer_destroy(zbuffer_t *zbuffer);
bool zbuffer_test(zbuffer_t *zbuffer, uint32_t x, uint32_t y, float z);
void zbuffer_write(zbuffer_t *zbuffer, uint32_t x, uint32_t y, float z);

/* Shader-like Functions (Software Implementation) */
vertex_3d_t vertex_shader(vertex_3d_t vertex, matrix4_t mvp_matrix);
uint32_t fragment_shader(vertex_3d_t fragment, material_3d_t *material, light_3d_t *lights, uint32_t light_count);
uint32_t neural_fragment_shader(vertex_3d_t fragment, float time, uint32_t seed);

/* Performance and Debug */
void graphics_3d_get_stats(uint32_t *triangles, uint32_t *vertices, uint32_t *pixels, uint32_t *frame_time);
void graphics_3d_reset_stats(void);
void graphics_3d_set_render_mode(render_mode_t mode);

/* Color Utilities */
uint32_t color_from_rgb(uint8_t r, uint8_t g, uint8_t b);
uint32_t color_from_rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a);
uint32_t color_lerp(uint32_t a, uint32_t b, float t);
uint32_t color_multiply(uint32_t color, float factor);
uint32_t color_add(uint32_t a, uint32_t b);

/* Math Constants */
#define PI 3.14159265359f
#define TWO_PI 6.28318530718f
#define HALF_PI 1.57079632679f
#define DEG_TO_RAD (PI / 180.0f)
#define RAD_TO_DEG (180.0f / PI)

/* Color Constants (Neural Theme) */
#define COLOR_NEURAL_BLUE    0xFF0080FF
#define COLOR_NEURAL_CYAN    0xFF00FFFF
#define COLOR_NEURAL_WHITE   0xFFFFFFFF
#define COLOR_NEURAL_BLACK   0xFF000000
#define COLOR_NEURAL_GREEN   0xFF00FF00
#define COLOR_NEURAL_RED     0xFFFF0000
#define COLOR_NEURAL_PURPLE  0xFF800080
#define COLOR_MATRIX_GREEN   0xFF00AA00
#define COLOR_DARK_BLUE      0xFF002040

#endif /* KERNEL_GRAPHICS_3D_H */