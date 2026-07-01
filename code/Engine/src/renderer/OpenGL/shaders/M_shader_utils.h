#pragma once
#include "defines.hpp"
#include "logger.h"
#include "glad.h"
#include <fstream>
#include <sstream>
#include <string>

MINLINE u32 create_shader_program(const char* vert_path, const char* frag_path)
{
    std::string vertex_code;
    std::string fragment_code;
    std::ifstream v_shader_file;
    std::ifstream f_shader_file;

    //file missing or locked it will throw an exception
    v_shader_file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    f_shader_file.exceptions(std::ifstream::failbit | std::ifstream::badbit);

    try 
    {
        v_shader_file.open(vert_path);
        f_shader_file.open(frag_path);

        std::stringstream v_shader_stream, f_shader_stream;

        // Dump the file contents into the string streams
        v_shader_stream << v_shader_file.rdbuf();
        f_shader_stream << f_shader_file.rdbuf();

        v_shader_file.close();
        f_shader_file.close();

        // Convert the streams into auto-sizing C++ strings
        vertex_code = v_shader_stream.str();
        fragment_code = f_shader_stream.str();
    }

    catch (std::ifstream::failure& e) 
    {
        MERROR("ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ");
        return 0;
    }

	// Convert the C++ strings into C-style strings for OpenGL
    const char* v_shader_code = vertex_code.c_str();
    const char* f_shader_code = fragment_code.c_str();

    u32 vertex, fragment;
    i32 success;
    char info_log[512];
     
	//Build and compile the vertex shader
    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &v_shader_code, 0);
    glCompileShader(vertex);

    glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
    if (!success) 
    {
        glGetShaderInfoLog(vertex, 512, 0, info_log);
        MERROR("ERROR::SHADER::VERTEX::COMPILATION_FAILED\n%s", info_log);
    }

	//build and compile the fragment shader
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

	// Clean up the shaders as they are no longer needed after linking, saves VRAM!!
    glDeleteShader(vertex);
    glDeleteShader(fragment);

    return shader_program_id;
}