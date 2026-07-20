#version 460 core

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec2 in_texcoord;
layout(location = 2) in vec3 in_normal;

layout (std140, binding = 0) uniform GlobalUniforms {
    mat4 projection;
    mat4 view;
    vec4 ambient_color;
    vec4 view_position;
} global_ubo;                  

layout (location = 0) uniform struct PushConstants {
    mat4 model;
    mat4 normal_matrix;
} u_push;

layout (location = 0) out vec2 out_texcoord;
layout (location = 1) out vec3 out_normal;
layout (location = 2) out vec4 out_ambient;
layout (location = 3) out vec3 out_view_position;
layout (location = 4) out vec3 out_frag_position;


void main() {
    vec4 world_position = u_push.model * vec4(in_position, 1.0);
    gl_Position = global_ubo.projection * global_ubo.view * world_position;
    
    out_texcoord = in_texcoord;
    out_ambient = global_ubo.ambient_color;
    out_normal = mat3(u_push.normal_matrix) * in_normal;
    
    out_view_position = global_ubo.view_position.xyz;
    out_frag_position = world_position.xyz;
}