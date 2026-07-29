#pragma once
#include "defines.hpp"
#include "M_math.h"

#define SHADER_MAX_ATTRIBUTES 16
#define SHADER_MAX_UNIFORMS 128

typedef enum shader_attribute_type {
    SHADER_ATTRIB_TYPE_FLOAT32 = 0,
    SHADER_ATTRIB_TYPE_FLOAT32_2 = 1,
    SHADER_ATTRIB_TYPE_FLOAT32_3 = 2,
    SHADER_ATTRIB_TYPE_FLOAT32_4 = 3,
    SHADER_ATTRIB_TYPE_INT8 = 4,
    SHADER_ATTRIB_TYPE_UINT8 = 5,
    SHADER_ATTRIB_TYPE_INT16 = 6,
    SHADER_ATTRIB_TYPE_UINT16 = 7,
    SHADER_ATTRIB_TYPE_INT32 = 8,
    SHADER_ATTRIB_TYPE_UINT32 = 9
} shader_attribute_type;

typedef enum shader_uniform_type {
    SHADER_UNIFORM_TYPE_FLOAT32 = 0,
    SHADER_UNIFORM_TYPE_FLOAT32_2 = 1,
    SHADER_UNIFORM_TYPE_FLOAT32_3 = 2,
    SHADER_UNIFORM_TYPE_FLOAT32_4 = 3,
    SHADER_UNIFORM_TYPE_INT8 = 4,
    SHADER_UNIFORM_TYPE_UINT8 = 5,
    SHADER_UNIFORM_TYPE_INT16 = 6,
    SHADER_UNIFORM_TYPE_UINT16 = 7,
    SHADER_UNIFORM_TYPE_INT32 = 8,
    SHADER_UNIFORM_TYPE_UINT32 = 9,
    SHADER_UNIFORM_TYPE_MATRIX_4 = 10,
    SHADER_UNIFORM_TYPE_SAMPLER = 11,
    SHADER_UNIFORM_TYPE_VECTOR_2 = 12,
    SHADER_UNIFORM_TYPE_CUSTOM = 255
} shader_uniform_type;

typedef struct opengl_shader {
    u32 id;
    u32 global_ubo_id;
    u32 global_ubo_size;
    u32 uniform_locations[SHADER_MAX_UNIFORMS];
    u32 uniform_types[SHADER_MAX_UNIFORMS];
    u16 uniform_count;
} opengl_shader;

b8 opengl_shader_create(const char* name, opengl_shader* out_shader);
void opengl_shader_destroy(opengl_shader* shader);
b8 opengl_shader_add_attribute(opengl_shader* shader, const char* name, shader_attribute_type type);
b8 opengl_shader_add_uniform(opengl_shader* shader, const char* name, shader_uniform_type type);
b8 opengl_shader_initialize(opengl_shader* shader);
b8 opengl_shader_use(opengl_shader* shader);
b8 opengl_shader_set_uniform(opengl_shader* shader, u16 index, void* value);