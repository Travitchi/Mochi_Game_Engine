#version 460 core
layout (location = 0) in vec2 in_position;
layout (location = 1) in vec2 in_texcoord;

layout (std140, binding = 0) uniform global_uniform_object
{
    mat4 projection;
    mat4 view;
} global_ubo;

uniform struct push_constants
{
    mat4 model;
} u_push;

out vec2 out_texcoord;

void main() 
{

    out_texcoord = in_texcoord;
    gl_Position = global_ubo.projection * u_push.model * vec4(in_position, 0.0, 1.0);
}