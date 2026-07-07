#pragma once
#include "renderer_backend.h"

b8 opengl_backend_initialize(renderer_backend* backend, const char* application_name, struct platform_state* plat_state);
void opengl_backend_shutdown(renderer_backend* backend);
void opengl_backend_resized(renderer_backend* backend, u16 width, u16 height);
b8 opengl_backend_begin_frame(renderer_backend* backend, f32 delta_time);
b8 opengl_backend_end_frame(renderer_backend* backend, f32 delta_time);
void opengl_backend_update_global_state(struct renderer_backend* backend, mat4 projection, mat4 view);
void opengl_backend_update_object(struct renderer_backend* backend, geometry_render_data data);
void opengl_backend_set_projection_params(struct renderer_backend* backend, f32 fov, f32 near_clip, f32 far_clip);
void opengl_backend_set_view(struct renderer_backend* backend, mat4 view);
void opengl_backend_create_texture(struct renderer_backend* backend, const char* name, b8 auto_release, i32 width, i32 height, i32 channel_count, const u8* pixels, b8 has_transparency, struct texture* out_texture);
void opengl_backend_destroy_texture(struct renderer_backend* backend, struct texture* texture);
void opengl_backend_create_geometry(struct renderer_backend* backend, struct geometry* geometry, u32 vertex_count, const void* vertices, u32 index_count, const void* indices);
void opengl_backend_destroy_geometry(struct renderer_backend* backend, struct geometry* geometry);
