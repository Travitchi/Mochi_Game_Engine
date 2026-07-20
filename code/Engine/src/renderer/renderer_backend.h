#pragma once
#include "defines.hpp"
#include "renderer_frontend.h"
#include "M_math.h"
#include "resource_types.h"


typedef struct renderer_backend 
{
    struct platform_state* plat_state;
    u64 frame_number;

    // Function pointers that the frontend will use to command the backend
    b8(*initialize)(struct renderer_backend* backend, const char* application_name, struct platform_state* plat_state);
    void (*shutdown)(struct renderer_backend* backend);
    void (*resized)(struct renderer_backend* backend, u16 width, u16 height);
    b8(*begin_frame)(struct renderer_backend* backend, f32 delta_time);
    b8(*end_frame)(struct renderer_backend* backend, f32 delta_time);
    b8(*begin_render_pass)(struct renderer_backend* backend, u8 pass_id);
    void (*end_render_pass)(struct renderer_backend* backend, u8 pass_id);
    void (*update_object)(struct renderer_backend* backend, geometry_render_data data);
    void (*set_projection_params)(struct renderer_backend* backend, f32 fov_radians, f32 near_clip, f32 far_clip);
    void (*set_view)(struct renderer_backend* backend, mat4 view);
    void (*create_texture)(struct renderer_backend* backend, const char* name, b8 auto_release, i32 width, i32 height, i32 channel_count, const u8* pixels, b8 has_transparency, struct texture* out_texture);
    void (*destroy_texture)(struct renderer_backend* backend, struct texture* texture);
    void (*create_geometry)(struct renderer_backend* backend, struct geometry* geometry, u32 vertex_size, u32 vertex_count, const void* vertices, u32 index_size, u32 index_count, const void* indices);
    void (*destroy_geometry)(struct renderer_backend* backend, struct geometry* geometry);
    b8(*shader_create)(struct renderer_backend* backend, struct shader* shader, const struct shader_config* config);
    void (*shader_destroy)(struct renderer_backend* backend, struct shader* shader);
    b8(*shader_use)(struct renderer_backend* backend, struct shader* shader);
    b8(*shader_set_uniform)(struct renderer_backend* backend, struct shader* shader, u16 index, void* value);
    void (*update_global_matrices)(struct renderer_backend* backend, mat4 projection, mat4 view);
} renderer_backend;