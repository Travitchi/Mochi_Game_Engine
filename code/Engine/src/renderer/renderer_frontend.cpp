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
#include "texture_systems.h"
#include "material_systems.h"
#include "geometry_systems.h"
#include "shader_system.h"
#include "camera_system.h"
#include "M_transform.h"

static renderer_backend* backend = 0;
static geometry* test_geometry = 0;
static mat4 world_projection = mat4_id();
static mat4 world_view = mat4_id();
static render_target window_target = {};
static render_pass registered_passes[10];
static u32 registered_pass_count = 0;
static render_target world_offscreen_target = {};
static texture offscreen_color_tex = {};
static texture offscreen_depth_tex = {};


b8 event_on_debug_event(u16 code, void* sender, void* listener_inst, event_context context)
{

    static f64 last_swap_time = 0;
    f64 current_time = platform_get_absolute_time();

    if (current_time - last_swap_time < 0.2)
    {
        return TRUE;
    }
    last_swap_time = current_time;
    const char* names[2] = { "test_mat_1", "test_mat_2" };
    static i32 choice = 1;

    if (test_geometry)
    {
        test_geometry->material = material_system_acquire(names[choice]);
    }

    choice++;
    choice %= 2;

    return TRUE;
}

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

    event_register(0x10, 0, event_on_debug_event);
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
        registered_passes[i].render_area_w = (f32)width;
        registered_passes[i].render_area_h = (f32)height;
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

void renderer_draw_test_geometry()
{
    if (backend && backend->update_object)
    {
        if (!test_geometry)
        {
            geometry_config cube_config = geometry_system_generate_cube_config(10.0f, 10.0f, 10.0f, 1.0f, 1.0f, "test_cube", "test_mat_1");
            test_geometry = geometry_system_acquire_from_config(cube_config, TRUE);
            Mfree(cube_config.vertices, cube_config.vertex_size * cube_config.vertex_count, MEMORY_TAG_ARRAY);
            Mfree(cube_config.indices, cube_config.index_size * cube_config.index_count, MEMORY_TAG_ARRAY);
        }

        geometry_render_data data = {};
        data.geometry = test_geometry;
        static transform test_transform = transform_from_position(vect3_create(0.0f, 0.0f, -20.0f));
       // static f32 angle = 0.0f;
        //angle += 0.0005f;
        //transform_rotation_set(&test_transform, vect3_create(angle * 0.5f, angle, 0.0f));
        data.model = transform_get_world(&test_transform);
        
        shader* obj_shader = shader_system_get("M_object_shader");
        shader_system_use("M_object_shader");
        shader_system_set_uniform(obj_shader, "u_push.model", &data.model);
        mat4 normal_matrix = data.model;
        shader_system_set_uniform(obj_shader, "u_push.normal_matrix", &normal_matrix);
        i32 texture_unit = 0;
        shader_system_set_uniform(obj_shader, "diffuse_sampler", &texture_unit);


        i32 spec_texture_unit = 1;
        shader_system_set_uniform(obj_shader, "specular_sampler", &spec_texture_unit);
        shader_system_set_uniform(obj_shader, "shininess", &data.geometry->material->shininess);

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