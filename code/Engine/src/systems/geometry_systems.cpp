#include "geometry_systems.h"
#include "M_memory.h"
#include "logger.h"
#include "material_systems.h"
#include "renderer_frontend.h"
#include "M_math.h"
#include <string.h>

typedef struct geometry_system_state 
{
    geometry_system_config config;
    geometry default_geometry;
    geometry hot_geometry;
} geometry_system_state;

static geometry_system_state* state_ptr = 0;

b8 geometry_system_initialize(u64* memory_requirement, void* state, geometry_system_config config)
{
    *memory_requirement = sizeof(geometry_system_state);
    if (!state) return TRUE;

    state_ptr = (geometry_system_state*)state;
    state_ptr->config = config;

    geometry_config def_config = geometry_system_generate_plane_config(1.0f, 1.0f, 1, 1, 1.0f, 1.0f, "default_geometry", "default");

    state_ptr->default_geometry.id = 0;
    strcpy_s(state_ptr->default_geometry.name, 256, def_config.name);
    state_ptr->default_geometry.material = material_system_get_default();
    renderer_create_geometry(&state_ptr->default_geometry, def_config.vertex_count, def_config.vertices, def_config.index_count, def_config.indices);
    Mfree(def_config.vertices, def_config.vertex_size, MEMORY_TAG_ARRAY);
    Mfree(def_config.indices, def_config.index_size, MEMORY_TAG_ARRAY);

    return TRUE;
}

void geometry_system_shutdown(void* state) 
{
    if (state_ptr)
    {
        renderer_destroy_geometry(&state_ptr->default_geometry);
        state_ptr = 0;
    }
}

geometry* geometry_system_acquire_from_config(geometry_config config, b8 auto_release) 
{
    strcpy_s(state_ptr->hot_geometry.name, 256, config.name);
    if (strlen(config.material_name) > 0) 
    {
        state_ptr->hot_geometry.material = material_system_acquire(config.material_name);
    }
    else 
    {
        state_ptr->hot_geometry.material = material_system_get_default();
    }

    renderer_create_geometry(&state_ptr->hot_geometry, config.vertex_count, config.vertices, config.index_count, config.indices);

    return &state_ptr->hot_geometry;
}

void geometry_system_release(geometry* geometry) 
{
    renderer_destroy_geometry(geometry);
}

geometry* geometry_system_get_default()
{
    return &state_ptr->default_geometry;
}

geometry_config geometry_system_generate_plane_config(f32 width, f32 height, u32 x_segment_count, u32 y_segment_count, f32 tile_x, f32 tile_y, const char* name, const char* material_name) 
{
    geometry_config config = {};
    if (width == 0.0f) width = 1.0f;
    if (height == 0.0f) height = 1.0f;
    if (x_segment_count == 0) x_segment_count = 1;
    if (y_segment_count == 0) y_segment_count = 1;

    config.vertex_count = x_segment_count * y_segment_count * 4;
    config.vertex_size = sizeof(vertex_3d) * config.vertex_count;
    config.vertices = Mallocate(config.vertex_size, MEMORY_TAG_ARRAY);

    config.index_count = x_segment_count * y_segment_count * 6;
    config.index_size = sizeof(u32) * config.index_count;
    config.indices = Mallocate(config.index_size, MEMORY_TAG_ARRAY);

    strcpy_s(config.name, 256, name);
    strcpy_s(config.material_name, 256, material_name);

    f32 seg_width = width / x_segment_count;
    f32 seg_height = height / y_segment_count;
    f32 half_width = width * 0.5f;
    f32 half_height = height * 0.5f;

    vertex_3d* v_ptr = (vertex_3d*)config.vertices;
    u32* i_ptr = (u32*)config.indices;
    u32 v_index = 0;

    for (u32 y = 0; y < y_segment_count; ++y) 
    {
        for (u32 x = 0; x < x_segment_count; ++x) 
        {
            f32 min_x = (x * seg_width) - half_width;
            f32 min_y = (y * seg_height) - half_height;
            f32 max_x = min_x + seg_width;
            f32 max_y = min_y + seg_height;

            f32 min_uv_x = (x / (f32)x_segment_count) * tile_x;
            f32 min_uv_y = (y / (f32)y_segment_count) * tile_y;
            f32 max_uv_x = ((x + 1) / (f32)x_segment_count) * tile_x;
            f32 max_uv_y = ((y + 1) / (f32)y_segment_count) * tile_y;

            v_ptr[0].position = vect3_create(min_x, min_y, 0.0f);
            v_ptr[0].texture = vect2_create(min_uv_x, min_uv_y);
            v_ptr[1].position = vect3_create(max_x, min_y, 0.0f);
            v_ptr[1].texture = vect2_create(max_uv_x, min_uv_y);
            v_ptr[2].position = vect3_create(max_x, max_y, 0.0f);
            v_ptr[2].texture = vect2_create(max_uv_x, max_uv_y);
            v_ptr[3].position = vect3_create(min_x, max_y, 0.0f);
            v_ptr[3].texture = vect2_create(min_uv_x, max_uv_y);

            i_ptr[0] = v_index + 0; i_ptr[1] = v_index + 1; i_ptr[2] = v_index + 2;
            i_ptr[3] = v_index + 0; i_ptr[4] = v_index + 2; i_ptr[5] = v_index + 3;

            v_ptr += 4;
            i_ptr += 6;
            v_index += 4;
        }
    }
    return config;
}