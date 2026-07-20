#include "texture_systems.h"
#include "M_memory.h"
#include "logger.h"
#include "hashtable.h"
#include "renderer_frontend.h"
#include <stdio.h> 
#include "resource_systems.h"

typedef struct texture_reference 
{
    u32 reference_count;
    u32 handle;
    b8 auto_release;
} texture_reference;

typedef struct texture_system_state 
{
    texture_system_config config;
    texture default_texture;
    texture default_specular_texture;
    texture* registered_textures;
    hashtable registered_texture_table;
} texture_system_state;

static texture_system_state* state_ptr = 0;

b8 load_texture(const char* texture_name, texture* out_texture)
{
    resource img_resource;
    if (!resource_system_load(texture_name, RESOURCE_TYPE_IMAGE, &img_resource)) 
    {
        return FALSE;
    }

    image_resource_data* resource_data = (image_resource_data*)img_resource.data;
    texture temp_texture = {};
    renderer_create_texture(texture_name, TRUE, resource_data->width, resource_data->height, resource_data->channel_count, resource_data->pixels, FALSE, &temp_texture);
    texture old = *out_texture;
    *out_texture = temp_texture;
    renderer_destroy_texture(&old);
    out_texture->generation = old.generation + 1;

    resource_system_unload(&img_resource);

    return TRUE;
}

b8 texture_system_initialize(u64* memory_requirement, void* state, texture_system_config config)
{
    if (config.max_texture_count == 0) return FALSE;

    u64 struct_requirement = sizeof(texture_system_state);
    u64 array_requirement = sizeof(texture) * config.max_texture_count;
    u64 hashtable_requirement = sizeof(texture_reference) * config.max_texture_count;

    *memory_requirement = struct_requirement + array_requirement + hashtable_requirement;

    if (!state) return TRUE;

    state_ptr = (texture_system_state*)state;
    state_ptr->config = config;

    void* array_block = (u8*)state + struct_requirement;
    state_ptr->registered_textures = (texture*)array_block;

    void* hashtable_block = (u8*)array_block + array_requirement;
    hashtable_create(sizeof(texture_reference), config.max_texture_count, hashtable_block, FALSE, &state_ptr->registered_texture_table);

    const u32 tex_dim = 256;
    const u32 channels = 4;
    u8* pixels = (u8*)Mallocate(tex_dim * tex_dim * channels, MEMORY_TAG_ARRAY);
    Mset_memory(pixels, 255, tex_dim * tex_dim * channels);

    renderer_create_texture("default", FALSE, tex_dim, tex_dim, channels, pixels, FALSE, &state_ptr->default_texture);
    Mfree(pixels, tex_dim * tex_dim * channels, MEMORY_TAG_ARRAY);

    u8* spec_pixels = (u8*)Mallocate(tex_dim * tex_dim * channels, MEMORY_TAG_ARRAY);
    Mset_memory(spec_pixels, 0, tex_dim * tex_dim * channels);
    renderer_create_texture("default_specular", FALSE, tex_dim, tex_dim, channels, spec_pixels, FALSE, &state_ptr->default_specular_texture);
    Mfree(spec_pixels, tex_dim * tex_dim * channels, MEMORY_TAG_ARRAY);

    return TRUE;
}

void texture_system_shutdown(void* state) 
{
    if (state_ptr) {
        renderer_destroy_texture(&state_ptr->default_texture);
        state_ptr = 0;
    }
}

texture* texture_system_acquire(const char* name, b8 auto_release) 
{
    static u32 texture_count = 0;
    texture* new_texture = &state_ptr->registered_textures[texture_count];
    if (load_texture(name, new_texture))
    {
        texture_count++;
        return new_texture;
    }

    return &state_ptr->default_texture;
}

void texture_system_release(const char* name) 
{
    // Stubbed for now
}

texture* texture_system_get_default_texture() 
{
    return &state_ptr->default_texture;
}

texture* texture_system_get_default_specular_texture() 
{
    return &state_ptr->default_specular_texture;
}