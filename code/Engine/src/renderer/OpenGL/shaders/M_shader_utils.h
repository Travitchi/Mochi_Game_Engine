#pragma once
#include "defines.hpp"
#include "logger.h"
#include "glad.h"
#include "resource_types.h"
#include "resource_systems.h"


MINLINE u32 create_shader_program(const char* vert_name, const char* frag_name)
{
    resource vert_res = {};
    resource frag_res = {};

    if (!resource_system_load(vert_name, RESOURCE_TYPE_TEXT, &vert_res))
    {
        MERROR("Failed to load vertex shader resource: %s", vert_name);
        return 0;
    }
    if (!resource_system_load(frag_name, RESOURCE_TYPE_TEXT, &frag_res))
    {
        MERROR("Failed to load fragment shader resource: %s", frag_name);
        return 0;
    }
    
    const char* v_shader_code = (const char*)vert_res.data;
    const char* f_shader_code = (const char*)frag_res.data;
    u32 vertex, fragment;
    i32 success;
    char info_log[512];
    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &v_shader_code, 0);
    glCompileShader(vertex);
    glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertex, 512, 0, info_log);
        MERROR("ERROR::SHADER::VERTEX::COMPILATION_FAILED\n%s", info_log);
    }

    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &f_shader_code, 0);
    glCompileShader(fragment);
    glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragment, 512, 0, info_log);
        MERROR("ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n%s", info_log);
    }

    u32 shader_program_id = glCreateProgram();
    glAttachShader(shader_program_id, vertex);
    glAttachShader(shader_program_id, fragment);
    glLinkProgram(shader_program_id);
    glGetProgramiv(shader_program_id, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(shader_program_id, 512, 0, info_log);
        MERROR("ERROR::SHADER::PROGRAM::LINKING_FAILED\n%s", info_log);
    }

    resource_system_unload(&vert_res);
    resource_system_unload(&frag_res);

    glDeleteShader(vertex);
    glDeleteShader(fragment);

    return shader_program_id;
}