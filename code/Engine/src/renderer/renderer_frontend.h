#pragma once
#include "defines.hpp"
#include "M_math.h"
#include "resource_types.h"

struct platform_state;

typedef struct geometry_render_data
{
    mat4 model;
    struct geometry* geometry;
    u32 object_id;
} geometry_render_data;

typedef struct render_packet 
{
    f32 delta_time;
    u32 geometry_count;
    geometry_render_data* geometries;
    u32 ui_geometry_count;
    geometry_render_data* ui_geometries;
}render_packet;

b8 renderer_initialize(const char* application_name, struct platform_state* plat_state);
void renderer_shutdown();

void renderer_on_resize(u16 width, u16 height);
KAPI void renderer_set_view(mat4 view);
KAPI b8 renderer_begin_frame(render_packet* packet);
KAPI b8 renderer_end_frame(render_packet* packet);
KAPI void renderer_draw_geometry(struct geometry_render_data data);
KAPI void renderer_draw_test_geometry();

KAPI void renderer_create_texture(const char* name, b8 auto_release, i32 width, i32 height, i32 channel_count, const u8* pixels, b8 has_transparency, texture* out_texture);
KAPI void renderer_destroy_texture(texture* texture);
KAPI void renderer_create_geometry(geometry* geometry, u32 vertex_size, u32 vertex_count, const void* vertices, u32 index_size, u32 index_count, const void* indices);
KAPI void renderer_destroy_geometry(geometry* geometry);
KAPI b8 renderer_begin_render_pass(u8 pass_id);
KAPI void renderer_end_render_pass(u8 pass_id);
KAPI void renderer_update_global_ui_state(mat4 projection, mat4 view);