#include "renderer_frontend.h"
#include "renderer_backend.h"
#include "opengl_backend.h"
#include "logger.h"
#include "M_memory.h"
#include "Platform.h"
#include "M_math.h"
#include "event.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "texture_systems.h"
#include "material_systems.h"
#include "geometry_systems.h"
#include "shader_system.h"
#include "camera_system.h"
#include "M_transform.h"

static renderer_backend* backend = 0;
static mat4 world_projection = mat4_id();
static mat4 world_view = mat4_id();
static render_target window_target = {};
static render_pass registered_passes[10];
static u32 registered_pass_count = 0;
static render_target world_offscreen_target = {};
static texture offscreen_color_tex = {};
static texture offscreen_depth_tex = {};
static render_target shadow_target = {};
static texture shadow_depth_tex = {};


static b8 create_render_pass(const render_pass_config* config, render_pass* out_pass)
{
    out_pass->id = registered_pass_count++;
    strcpy_s(out_pass->name, 256, config->name);
    out_pass->render_area_x = config->render_area_x;
    out_pass->render_area_y = config->render_area_y;
    out_pass->render_area_w = config->render_area_w;
    out_pass->render_area_h = config->render_area_h;
    out_pass->clear_color = config->clear_color;
    out_pass->clear_flags = config->clear_flags;
    out_pass->depth_test_enabled = config->depth_test_enabled;
    out_pass->blend_enabled = config->blend_enabled;
    return TRUE;
}



b8 renderer_initialize(const char* application_name, struct platform_state* plat_state)
{
    backend = (renderer_backend*)Mallocate(sizeof(renderer_backend), MEMORY_TAG_RENDERER);
    if (!backend) 
    {
        MFATAL("Failed to allocate memory for renderer backend!");
        return FALSE;
    }
    backend->plat_state = plat_state;
    backend->frame_number = 0;

    backend->initialize = opengl_backend_initialize;
    backend->shutdown = opengl_backend_shutdown;
    backend->resized = opengl_backend_resized;
    backend->begin_frame = opengl_backend_begin_frame;
    backend->end_frame = opengl_backend_end_frame;
    backend->begin_render_pass = opengl_backend_begin_render_pass;
    backend->end_render_pass = opengl_backend_end_render_pass;
    backend->update_object = opengl_backend_update_object;
    backend->set_projection_params = opengl_backend_set_projection_params;
    backend->set_view = opengl_backend_set_view;
    backend->create_texture = opengl_backend_create_texture;
    backend->destroy_texture = opengl_backend_destroy_texture;
    backend->create_geometry = opengl_backend_create_geometry;
    backend->destroy_geometry = opengl_backend_destroy_geometry;
    backend->shader_create = opengl_backend_shader_create;
    backend->shader_destroy = opengl_backend_shader_destroy;
    backend->shader_use = opengl_backend_shader_use;
    backend->shader_set_uniform = opengl_backend_shader_set_uniform;
    backend->update_global_matrices = opengl_backend_update_global_matrices;
    backend->render_target_create = opengl_backend_render_target_create;
    backend->render_target_destroy = opengl_backend_render_target_destroy;

    if (!backend->initialize(backend, application_name, plat_state))
    {
        MFATAL("Renderer backend failed to initialize!");
        return FALSE;
    }


    window_target.id = 0;
    window_target.internal_fbo_id = 0;
    window_target.color_attachment = 0;
    window_target.depth_attachment = 0;

    //Configure the WORLD Render Pass
    render_pass_config world_config = {};
    strcpy_s(world_config.name, 256, "Builtin.RenderPass.World");
    world_config.render_area_x = 0;
    world_config.render_area_y = 0;
    world_config.render_area_w = 1280;
    world_config.render_area_h = 720;
    world_config.clear_color = vect4_create(0.1f, 0.1f, 0.12f, 1.0f);
    world_config.clear_flags = RENDER_PASS_CLEAR_COLOR_BUFFER_FLAG | RENDER_PASS_CLEAR_DEPTH_BUFFER_FLAG;
    world_config.depth_test_enabled = TRUE;
    world_config.blend_enabled = FALSE;
    create_render_pass(&world_config, &registered_passes[0]);

    //Configure Shadow Pass
    render_pass_config shadow_config = {};
    strcpy_s(shadow_config.name, 256, "Builtin.RenderPass.Shadow");
    shadow_config.render_area_x = 0;
    shadow_config.render_area_y = 0;
    shadow_config.render_area_w = 2048; // High resolution for crisp shadows
    shadow_config.render_area_h = 2048;
    shadow_config.clear_color = vect4_create(0.0f, 0.0f, 0.0f, 0.0f);
    shadow_config.clear_flags = RENDER_PASS_CLEAR_DEPTH_BUFFER_FLAG; // Depth ONLY!
    shadow_config.depth_test_enabled = TRUE;
    shadow_config.blend_enabled = FALSE;
    create_render_pass(&shadow_config, &registered_passes[2]);
    renderer_create_texture("shadow_map_texture", FALSE, 2048, 2048, 1, nullptr, FALSE, &shadow_depth_tex);
    texture* shadow_attachments[1] = { &shadow_depth_tex };
    if (backend && backend->render_target_create)
    {
        backend->render_target_create(backend, 1, shadow_attachments, &shadow_target);
    }


    //Configure the UI Render Pass
    render_pass_config ui_config = {};
    strcpy_s(ui_config.name, 256, "Builtin.RenderPass.UI");
    ui_config.render_area_x = 0;
    ui_config.render_area_y = 0;
    ui_config.render_area_w = 1280;
    ui_config.render_area_h = 720;
    ui_config.clear_color = vect4_create(0.0f, 0.0f, 0.0f, 0.0f);
    ui_config.clear_flags = RENDER_PASS_CLEAR_NONE_FLAG;
    ui_config.depth_test_enabled = FALSE;
    ui_config.blend_enabled = TRUE;
    create_render_pass(&ui_config, &registered_passes[1]);

    //blank textures for offscreen rendering
    renderer_create_texture("world_color_buffer", FALSE, 1280, 720, 4, nullptr, FALSE, &offscreen_color_tex);
    renderer_create_texture("world_depth_buffer", FALSE, 1280, 720, 1, nullptr, FALSE, &offscreen_depth_tex);
    //Combine them into an offscreen FBO render target!
    texture* attachments[2] = { &offscreen_color_tex, &offscreen_depth_tex };
    if (backend && backend->render_target_create)
    {
        backend->render_target_create(backend, 2, attachments, &world_offscreen_target);
    }

    renderer_on_resize(1280, 720);
    return TRUE;
}

void renderer_shutdown()
{
    if (backend) 
    {
        backend->shutdown(backend);

        Mfree(backend, sizeof(renderer_backend), MEMORY_TAG_RENDERER);
        backend = 0;
    }
}

void renderer_on_resize(u16 width, u16 height)
{
    if (height != 0)
    {
        world_projection = mat4_perspective(deg_to_rad(30.0f), (f32)width / (f32)height, 0.1f, 1000.0f);
    }

    for (u32 i = 0; i < registered_pass_count; ++i)
    {
        if (strcmp(registered_passes[i].name, "Builtin.RenderPass.Shadow") != 0)
        {
        registered_passes[i].render_area_w = (f32)width;
        registered_passes[i].render_area_h = (f32)height;
        }
    }

    if (backend) 
    {
        backend->resized(backend, width, height);
    }
}

void renderer_set_view(mat4 view)
{
    world_view = view;
    if (backend && backend->set_view)
    {
        backend->set_view(backend, view);
    }
}

void renderer_create_texture(const char* name, b8 auto_release, i32 width, i32 height, i32 channel_count, const u8* pixels, b8 has_transparency, texture* out_texture)
{
    if (backend && backend->create_texture) {
        backend->create_texture(backend, name, auto_release, width, height, channel_count, pixels, has_transparency, out_texture);
    }
}

void renderer_destroy_texture(texture* texture)
{
    if (backend && backend->destroy_texture) {
        backend->destroy_texture(backend, texture);
    }
}

void renderer_create_geometry(geometry* geometry, u32 vertex_size, u32 vertex_count, const void* vertices, u32 index_size, u32 index_count, const void* indices)
{
    if (backend && backend->create_geometry)
    {
        backend->create_geometry(backend, geometry, vertex_size, vertex_count, vertices, index_size, index_count, indices);
    }
}

void renderer_destroy_geometry(geometry* geometry) 
{
    if (backend && backend->destroy_geometry) 
    {
        backend->destroy_geometry(backend, geometry);
    }
}

b8 renderer_begin_render_pass(struct render_pass* pass, struct render_target* target)
{
    return backend->begin_render_pass(backend, pass, target);
}

void renderer_end_render_pass(struct render_pass* pass)
{
    backend->end_render_pass(backend, pass);
}

struct render_pass* renderer_render_pass_get(const char* name)
{
    for (u32 i = 0; i < registered_pass_count; ++i) 
    {
        if (strcmp(registered_passes[i].name, name) == 0)
        {
            return &registered_passes[i];
        }
    }
    return 0;
}

struct render_target* renderer_window_target_get()
{
    return &window_target;
}

struct render_target* renderer_world_target_get()
{
    return &world_offscreen_target;
}

b8 renderer_begin_frame(render_packet* packet)
{
    if (!backend) return FALSE;
    return backend->begin_frame(backend, packet->delta_time);
}

b8 renderer_end_frame(render_packet* packet)
{
    if (!backend) return FALSE;
    b8 result = backend->end_frame(backend, packet->delta_time);
    backend->frame_number++;
    return result;
}

void renderer_draw_geometry(geometry_render_data data)
{
    if (backend && backend->update_object)
    {
        backend->update_object(backend, data);
    }
}


b8 renderer_shader_create(struct shader* shader, const struct shader_config* config)
{
    return backend->shader_create(backend, shader, config);
}

void renderer_shader_destroy(struct shader* shader)
{
    if (backend && backend->shader_destroy)
    {
        backend->shader_destroy(backend, shader);
    }
}

b8 renderer_shader_use(struct shader* shader) 
{
    return backend->shader_use(backend, shader);
}

b8 renderer_shader_set_uniform(struct shader* shader, u16 index, void* value) 
{
    if (!backend || !backend->shader_set_uniform) return FALSE;
    return backend->shader_set_uniform(backend, shader, index, value);
}

void renderer_update_global_matrices(mat4 projection, mat4 view) 
{
    if (backend && backend->update_global_matrices) 
    {
        backend->update_global_matrices(backend, projection, view);
    }
}

void renderer_push_world_matrices() 
{
    if (backend && backend->update_global_matrices)
    {
        camera* active_camera = camera_system_get_default();
        mat4 view = camera_view_get(active_camera);
        backend->update_global_matrices(backend, world_projection, view);
    }
}

static MINLINE f32 vect3_distance_squared(vect3 a, vect3 b)
{
    f32 dx = b.x - a.x;
    f32 dy = b.y - a.y;
    f32 dz = b.z - a.z;
    return (dx * dx) + (dy * dy) + (dz * dz);
}

static int compare_distance_back_to_front(const void* a, const void* b)
{
    const geometry_render_data* geom_a = (const geometry_render_data*)a;
    const geometry_render_data* geom_b = (const geometry_render_data*)b;
    vect3 pos_a = vect3_create(geom_a->model.data[12], geom_a->model.data[13], geom_a->model.data[14]);
    vect3 pos_b = vect3_create(geom_b->model.data[12], geom_b->model.data[13], geom_b->model.data[14]);
    camera* active_cam = camera_system_get_default();
    vect3 cam_pos = camera_position_get(active_cam);
    f32 dist_a = vect3_distance_squared(pos_a, cam_pos);
    f32 dist_b = vect3_distance_squared(pos_b, cam_pos);

    if (dist_a < dist_b) return 1;
    if (dist_a > dist_b) return -1;
    return 0;
}

static int compare_y_sort(const void* a, const void* b)
{
    const geometry_render_data* geom_a = (const geometry_render_data*)a;
    const geometry_render_data* geom_b = (const geometry_render_data*)b;
    f32 y_a = geom_a->model.data[13];
    f32 y_b = geom_b->model.data[13];

    if (y_a < y_b) return 1;
    if (y_a > y_b) return -1;
    return 0;
}

void renderer_sort_geometries(geometry_render_data* geometries, u32 count, b8 sort_by_camera_distance)
{
    if (!geometries || count <= 1) return;

    if (sort_by_camera_distance)
    {
        qsort(geometries, count, sizeof(geometry_render_data), compare_distance_back_to_front);
    }
    else
    {
        qsort(geometries, count, sizeof(geometry_render_data), compare_y_sort);
    }
}

struct render_target* renderer_shadow_target_get()
{
    return &shadow_target;
}

texture* renderer_shadow_map_texture_get()
{
    return &shadow_depth_tex;
}

mat4 renderer_calculate_directional_light_space_matrix(vect3 light_dir, vect3 target_center)
{
    //orthographic projection box for the sun (Left, Right, Bottom, Top, Near, Far)
    mat4 light_projection = mat4_orthographic(-30.0f, 30.0f, -30.0f, 30.0f, -100.0f, 100.0f);

    //Position the light camera backward
    vect3 light_pos = vect3_sub(target_center, vect3_mult_scale(light_dir, 30.0f));

    //Look at the center of the scene
    mat4 light_view = mat4_look_at(light_pos, target_center, vect3_create(0.0f, -1.0f, 0.0f));

    // Combine into a single matrix that transforms World Space -> Light Space
    return mat4_mult(light_view, light_projection);
}