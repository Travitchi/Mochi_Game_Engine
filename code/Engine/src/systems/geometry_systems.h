#pragma once
#include "defines.hpp"
#include "resources/resource_types.h"

typedef struct geometry_system_config
{
    u32 max_geometry_count;
} geometry_system_config;

typedef struct geometry_config 
{
    u32 vertex_size;
    u32 vertex_count;
    void* vertices;
    u32 index_size;
    u32 index_count;
    void* indices;
    char name[256];
    char material_name[256];
} geometry_config;

b8 geometry_system_initialize(u64* memory_requirement, void* state, geometry_system_config config);
void geometry_system_shutdown(void* state);
geometry* geometry_system_acquire_from_config(geometry_config config, b8 auto_release);
void geometry_system_release(geometry* geometry);
geometry* geometry_system_get_default();
geometry_config geometry_system_generate_plane_config(f32 width, f32 height, u32 x_segment_count, u32 y_segment_count, f32 tile_x, f32 tile_y, const char* name, const char* material_name);
geometry_config geometry_system_generate_cube_config(f32 width, f32 height, f32 depth, f32 tile_x, f32 tile_y, const char* name, const char* material_name);