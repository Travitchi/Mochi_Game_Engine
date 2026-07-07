#include "resource_systems.h"
#include "M_memory.h"
#include "logger.h"
#include <string.h>

typedef struct resource_system_state
{
    resource_system_config config;
    resource_loader registered_loaders[32]; //Hardcoded max 32 loaders for simplicity
} resource_system_state;

static resource_system_state* state_ptr = 0;

b8 resource_system_initialize(u64* memory_requirement, void* state, resource_system_config config)
{
    *memory_requirement = sizeof(resource_system_state);
    if (!state) return TRUE;

    state_ptr = (resource_system_state*)state;
    state_ptr->config = config;

    for (u32 i = 0; i < 32; ++i) 
    {
        state_ptr->registered_loaders[i].id = 0;
    }

    MINFO("Resource System Initialized with base path: %s", config.asset_base_path);
    return TRUE;
}

void resource_system_shutdown(void* state) 
{
    state_ptr = 0;
}

b8 resource_system_register_loader(resource_loader loader)
{
    if (!state_ptr) return FALSE;
    for (u32 i = 0; i < 32; ++i)
    {
        if (state_ptr->registered_loaders[i].id == 0) 
        {
            loader.id = i + 1;
            state_ptr->registered_loaders[i] = loader;
            return TRUE;
        }
    }
    return FALSE;
}

b8 resource_system_load(const char* name, resource_type type, resource* out_resource)
{
    if (!state_ptr) return FALSE;
    for (u32 i = 0; i < 32; ++i) 
    {
        if (state_ptr->registered_loaders[i].id != 0 && state_ptr->registered_loaders[i].type == type)
        {
            out_resource->loader_id = state_ptr->registered_loaders[i].id;
            return state_ptr->registered_loaders[i].load(name, &state_ptr->registered_loaders[i], out_resource);
        }
    }
    MERROR("No loader found for resource type!");
    return FALSE;
}

void resource_system_unload(resource* res)
{
    if (!state_ptr || !res || res->loader_id == 0) return;
    for (u32 i = 0; i < 32; ++i) 
    {
        if (state_ptr->registered_loaders[i].id == res->loader_id) 
        {
            state_ptr->registered_loaders[i].unload(&state_ptr->registered_loaders[i], res);
            return;
        }
    }
}

const char* resource_system_base_path() 
{
    return state_ptr ? state_ptr->config.asset_base_path : "";
}