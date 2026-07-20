#pragma once
#include "defines.hpp"
#include "opengl_shader.h"

typedef struct shader_uniform_config
{
    char name[256];
    shader_uniform_type type;
} shader_uniform_config;

typedef struct shader_attribute_config 
{
    char name[256];
    shader_attribute_type type;
} shader_attribute_config;

typedef struct shader_config 
{
    char name[256];
    u8 attribute_count;
    shader_attribute_config attributes[SHADER_MAX_ATTRIBUTES];
    u8 uniform_count;
    shader_uniform_config uniforms[SHADER_MAX_UNIFORMS];
} shader_config;

typedef struct shader 
{
    u32 id;
    char name[256];
    u8 uniform_count;
    shader_uniform_config uniforms[SHADER_MAX_UNIFORMS];
    void* internal_data;
} shader;

typedef struct shader_system_config
{
    u32 max_shader_count;
} shader_system_config;

b8 shader_system_initialize(u64* memory_requirement, void* state, shader_system_config config);
void shader_system_shutdown(void* state);
b8 shader_system_create_shader(const shader_config* config, shader* out_shader);
shader* shader_system_get(const char* name);
b8 shader_system_use(const char* name);
u16 shader_system_get_uniform_index(shader* s, const char* uniform_name);
b8 shader_system_set_uniform(shader* s, const char* uniform_name, void* value);