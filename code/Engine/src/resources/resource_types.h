#pragma once
#include "defines.hpp"
#include "M_math.h"

typedef struct texture
{
    u32 id;
    u32 width;
    u32 height;
    u8 channel_count;
    b8 has_transparency;
    u32 generation;
    char name[512];
    void* internal_data;
} texture;

typedef enum texture_use 
{
    TEXTURE_USE_UNKNOWN = 0,
    TEXTURE_USE_MAP_DIFFUSE = 1
} texture_use;

typedef struct texture_map 
{
    struct texture* texture;
    texture_use use;
} texture_map;

typedef struct material 
{
    u32 id;
    u32 generation;
    u32 internal_id;
    char name[256];
    vect4 diffuse_color;
    texture_map diffuse_map;
} material;

typedef struct material_config 
{
    char name[256];
    b8 auto_release;
    vect4 diffuse_color;
    char diffuse_map_name[256];
} material_config;

typedef struct geometry
{
    u32 id;
    u32 internal_id;
    u32 generation;
    char name[256];
    struct material* material;
} geometry;

typedef enum resource_type 
{
    RESOURCE_TYPE_TEXT,
    RESOURCE_TYPE_BINARY,
    RESOURCE_TYPE_IMAGE,
    RESOURCE_TYPE_MATERIAL,
    RESOURCE_TYPE_CUSTOM
} resource_type;

typedef struct resource 
{
    u32 loader_id;
    const char* name;
    char full_path[512];
    u64 data_size;
    void* data; // holds the actual pixels, text, or structs!
} resource;

typedef struct image_resource_data 
{
    u8 channel_count;
    u32 width;
    u32 height;
    u8* pixels;
} image_resource_data;