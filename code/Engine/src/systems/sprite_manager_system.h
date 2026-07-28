#pragma once
#include "defines.hpp"
#include "M_math.h"
#include "resource_types.h"

typedef struct sprite_sheet 
{
    u32 id;
    char name[256];
    struct texture* diffuse_texture;
    struct texture* normal_texture;
    u32 width;
    u32 height;
    u32 tile_width;
    u32 tile_height;
    u32 columns;
    u32 rows;
} sprite_sheet;

typedef struct sprite 
{
    u32 id;
    char name[256];
    struct sprite_sheet* sheet;
    u32 current_frame;
    vect2 uv_min;                   // Bottom-left (or top-left depending on UV layout)
    vect2 uv_max;                   // Top-right (or bottom-right)
    vect2 uv_offset;                // Direct uniform input for GLSL: Tile offset
    vect2 uv_scale;                 // Direct uniform input for GLSL: Tile scale
} sprite;

typedef struct sprite_system_config 
{
    u32 max_sprite_sheet_count;
    u32 max_sprite_count;
} sprite_system_config;

KAPI b8 sprite_system_initialize(u64* memory_requirement, void* state, sprite_system_config config);
KAPI void sprite_system_shutdown(void* state);

// Sprite Sheet Management
KAPI sprite_sheet* sprite_system_create_sheet(const char* name, const char* diffuse_texture_name, const char* normal_texture_name, u32 tile_width, u32 tile_height);
KAPI void sprite_system_destroy_sheet(sprite_sheet* sheet);
KAPI sprite_sheet* sprite_system_get_sheet(const char* name);

// Sprite Instance & UV Management
KAPI sprite* sprite_system_create_sprite(const char* name, sprite_sheet* sheet);
KAPI void sprite_system_destroy_sprite(sprite* s);
KAPI sprite* sprite_system_get_sprite(const char* name);

// Frame Slicing Math
KAPI void sprite_set_frame(sprite* s, u32 frame_index);