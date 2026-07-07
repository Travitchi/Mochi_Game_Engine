#pragma once
#include "defines.hpp"
#include "resource_types.h"

typedef b8(*PFN_resource_load)(const char* name, void* loader, resource* out_resource);
typedef void (*PFN_resource_unload)(void* loader, resource* res);

typedef struct resource_loader 
{
    u32 id;
    resource_type type;
    const char* type_path; // example "textures", "materials"
    PFN_resource_load load;
    PFN_resource_unload unload;
} resource_loader;

typedef struct resource_system_config
{
    u32 max_loader_count;
    const char* asset_base_path;
} resource_system_config;

b8 resource_system_initialize(u64* memory_requirement, void* state, resource_system_config config);
void resource_system_shutdown(void* state);

b8 resource_system_register_loader(resource_loader loader);
b8 resource_system_load(const char* name, resource_type type, resource* out_resource);
void resource_system_unload(resource* res);
const char* resource_system_base_path();