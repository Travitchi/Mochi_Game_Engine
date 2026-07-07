#pragma once
#include "defines.hpp"
#include "resource_types.h"

typedef struct material_system_config 
{
    u32 max_material_count;
} material_system_config;

b8 material_system_initialize(u64* memory_requirement, void* state, material_system_config config);
void material_system_shutdown(void* state);

material* material_system_acquire(const char* name);
void material_system_release(const char* name);
material* material_system_get_default();