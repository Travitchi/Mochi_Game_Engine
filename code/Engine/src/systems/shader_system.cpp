#include "shader_system.h"
#include "logger.h"
#include "M_memory.h"
#include "hashtable.h"
#include "renderer_frontend.h"
#include <string.h>

typedef struct shader_system_state
{
    shader_system_config config;
    shader* registered_shaders;
    hashtable lookup_table;
    u32 shader_count;
} shader_system_state;

static shader_system_state* state_ptr = 0;

b8 shader_system_initialize(u64* memory_requirement, void* state, shader_system_config config) 
{
    u64 struct_requirement = sizeof(shader_system_state);
    u64 array_requirement = sizeof(shader) * config.max_shader_count;
    u64 hashtable_requirement = sizeof(u32) * config.max_shader_count;
    *memory_requirement = struct_requirement + array_requirement + hashtable_requirement;
    if (!state) return TRUE;
    state_ptr = (shader_system_state*)state;
    state_ptr->config = config;
    state_ptr->shader_count = 0;
    void* array_block = (u8*)state + struct_requirement;
    state_ptr->registered_shaders = (shader*)array_block;
    void* hashtable_block = (u8*)array_block + array_requirement;
    hashtable_create(sizeof(u32), config.max_shader_count, hashtable_block, FALSE, &state_ptr->lookup_table);

    return TRUE;
}

void shader_system_shutdown(void* state)
{
    if (state_ptr) 
    {
        for (u32 i = 0; i < state_ptr->shader_count; ++i)
        {
            renderer_shader_destroy(&state_ptr->registered_shaders[i]);
        }
        state_ptr = 0;
    }
}

b8 shader_system_create_shader(const shader_config* config, shader* out_shader)
{
    if (!state_ptr || !config || !out_shader) return FALSE;
    strcpy_s(out_shader->name, 256, config->name);
    out_shader->uniform_count = config->uniform_count;
    for (u32 i = 0; i < config->uniform_count; ++i)
    {
        out_shader->uniforms[i] = config->uniforms[i];
    }

    if (!renderer_shader_create(out_shader, config)) 
    {
        MERROR("Failed to create shader: %s", config->name);
        return FALSE;
    }

    u32 id = state_ptr->shader_count++;
    out_shader->id = id;
    state_ptr->registered_shaders[id] = *out_shader;
    hashtable_set(&state_ptr->lookup_table, config->name, &id);

    return TRUE;
}

shader* shader_system_get(const char* name) 
{
    u32 index;
    if (hashtable_get(&state_ptr->lookup_table, name, &index)) 
    {
        return &state_ptr->registered_shaders[index];
    }
    return 0;
}

b8 shader_system_use(const char* name) 
{
    shader* s = shader_system_get(name);
    if (!s) return FALSE;
    return renderer_shader_use(s);
}

u16 shader_system_get_uniform_index(shader* s, const char* uniform_name) 
{
    if (!s) return 0xFFFF;
    for (u16 i = 0; i < s->uniform_count; ++i)
    {
        if (strcmp(s->uniforms[i].name, uniform_name) == 0) 
        {
            return i;
        }
    }
    MWARN("Uniform '%s' not found in shader '%s'.", uniform_name, s->name);
    return 0xFFFF;
}

b8 shader_system_set_uniform(shader* s, const char* uniform_name, void* value)
{
    u16 index = shader_system_get_uniform_index(s, uniform_name);
    if (index == 0xFFFF) return FALSE;
    return renderer_shader_set_uniform(s, index, value);
}