#include "opengl_shader.h"
#include "logger.h"
#include "M_memory.h"
#include "glad.h"
#include "M_shader_utils.h"
#include <stdio.h>
#include <string.h>

b8 opengl_shader_create(const char* name, opengl_shader* out_shader)
{
    if (!name || !out_shader) return FALSE;
    Mzero_memory(out_shader, sizeof(opengl_shader));

    char vert_name[512];
    char frag_name[512];
    sprintf_s(vert_name, 512, "%s.vert", name);
    sprintf_s(frag_name, 512, "%s.frag", name);
    out_shader->id = create_shader_program(vert_name, frag_name);
    if (out_shader->id == 0) 
    {
        MERROR("Failed to create OpenGL shader: %s", name);
        return FALSE;
    }

    out_shader->uniform_count = 0;
    MINFO("Successfully created OpenGL shader: %s", name);
    return TRUE;
}

void opengl_shader_destroy(opengl_shader* shader) 
{
    if (shader && shader->id != 0) 
    {
        glDeleteProgram(shader->id);
        Mzero_memory(shader, sizeof(opengl_shader));
    }
}

b8 opengl_shader_add_attribute(opengl_shader* shader, const char* name, shader_attribute_type type) 
{
    return TRUE;
}

b8 opengl_shader_add_uniform(opengl_shader* shader, const char* name, shader_uniform_type type) 
{
    if (!shader || shader->uniform_count >= SHADER_MAX_UNIFORMS) 
    {
        MERROR("Cannot add uniform '%s', shader is invalid or max uniforms reached.", name);
        return FALSE;
    }

    i32 location = glGetUniformLocation(shader->id, name);
    if (location == -1) 
    {
        MWARN("Uniform '%s' not found or optimized out in shader.", name);
    }

    u16 index = shader->uniform_count;
    shader->uniform_locations[index] = (u32)location;
    shader->uniform_types[index] = type;
    shader->uniform_count++;

    return TRUE;
}

b8 opengl_shader_initialize(opengl_shader* shader)
{
    if (!shader || shader->id == 0) return FALSE;
    return TRUE;
}

b8 opengl_shader_use(opengl_shader* shader)
{
    if (!shader || shader->id == 0) return FALSE;
    glUseProgram(shader->id);
    return TRUE;
}

b8 opengl_shader_set_uniform(opengl_shader* shader, u16 index, void* value)
{
    if (!shader || index >= shader->uniform_count) return FALSE;
    u32 loc = shader->uniform_locations[index];
    shader_uniform_type type = (shader_uniform_type)shader->uniform_types[index];
    switch (type) 
    {
    case SHADER_UNIFORM_TYPE_MATRIX_4:
        glUniformMatrix4fv(loc, 1, GL_FALSE, (f32*)value);
        break;
    case SHADER_UNIFORM_TYPE_SAMPLER:
        glUniform1i(loc, *(i32*)value);
        break;
    case SHADER_UNIFORM_TYPE_FLOAT32:
        glUniform1f(loc, *(f32*)value);
        break;
    default:
        MWARN("opengl_shader_set_uniform: Unsupported uniform type %d", type);
        return FALSE;
    }
    return TRUE;
}