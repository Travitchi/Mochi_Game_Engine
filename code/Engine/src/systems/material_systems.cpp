#include "material_systems.h"
#include "texture_systems.h"
#include "M_memory.h"
#include "logger.h"
#include "resource_systems.h"
#include <stdio.h>
#include <string.h>
#include <string>

typedef struct material_system_state {
    material_system_config config;
    material default_material;
	// todo: integrate hash table for materials later
    material hot_material;
} material_system_state;

static material_system_state* state_ptr = 0;



b8 material_system_initialize(u64* memory_requirement, void* state, material_system_config config) 
{
    *memory_requirement = sizeof(material_system_state);
    if (!state) return TRUE;

    state_ptr = (material_system_state*)state;
    state_ptr->config = config;

    // Build Default Material
    strcpy_s(state_ptr->default_material.name, 256, "default");
    state_ptr->default_material.diffuse_color = vect4_create(1.0f, 1.0f, 1.0f, 1.0f);
    state_ptr->default_material.diffuse_map.use = TEXTURE_USE_MAP_DIFFUSE;
    state_ptr->default_material.diffuse_map.texture = texture_system_get_default_texture();

    return TRUE;
}

void material_system_shutdown(void* state) 
{
    state_ptr = 0;
}

material* material_system_acquire(const char* name)
{
    resource mat_resource;
    if (!resource_system_load(name, RESOURCE_TYPE_MATERIAL, &mat_resource))
    {
        MWARN("Failed to load material '%s', using default.", name);
        return &state_ptr->default_material;
    }

    material_config* config = (material_config*)mat_resource.data;

    strcpy_s(state_ptr->hot_material.name, 256, config->name);
    state_ptr->hot_material.diffuse_color = config->diffuse_color;
    state_ptr->hot_material.diffuse_map.use = TEXTURE_USE_MAP_DIFFUSE;

    if (strlen(config->diffuse_map_name) > 0)
    {
        state_ptr->hot_material.diffuse_map.texture = texture_system_acquire(config->diffuse_map_name, config->auto_release);
    }
    else
    {
        state_ptr->hot_material.diffuse_map.texture = texture_system_get_default_texture();
    }

    resource_system_unload(&mat_resource);

    return &state_ptr->hot_material;
}

void material_system_release(const char* name)
{

}

material* material_system_get_default() {
    return &state_ptr->default_material;
}