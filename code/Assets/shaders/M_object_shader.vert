#version 460 core

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec2 in_texcoord;
layout(location = 2) in vec3 in_normal;

layout(std140, binding = 0) uniform GlobalState
{
    mat4 projection; 
    mat4 view;
}global_ubo;                   

uniform struct PushConstants {
    mat4 model;
    mat4 normal_matrix;
} u_push;

out vec2 frag_texcoord;
out vec3 frag_normal;

void main()
{
    frag_texcoord = in_texcoord;
    frag_normal = mat3(u_push.normal_matrix) * in_normal;
    
   gl_Position = global_ubo.projection * global_ubo.view * u_push.model * vec4(in_position, 1.0);
}