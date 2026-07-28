#version 460 core

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec2 in_texcoord;
layout(location = 2) in vec3 in_normal;

struct DirectionalLight {
    vec4 color;
    vec4 direction;
};

struct PointLight {
    vec4 color;
    vec4 position;
    vec4 attenuation;
};

layout (std140, binding = 0) uniform GlobalUniforms {
    mat4 projection;
    mat4 view;
    vec4 ambient_color;
    vec4 view_position;
    DirectionalLight dir_light;
    PointLight p_lights[10];
    uint num_p_lights;
} global_ubo;

layout (location = 0) uniform struct PushConstants {
    mat4 model;
    mat4 normal_matrix;
} u_push;

uniform mat4 u_light_space_matrix;

layout (location = 0) out vec2 out_texcoord;
layout (location = 1) out vec3 out_normal;
layout (location = 2) out vec4 out_ambient;
layout (location = 3) out vec3 out_view_position;
layout (location = 4) out vec3 out_frag_position;
layout (location = 5) out vec4 out_light_space_pos;

void main() {
    vec4 world_position = u_push.model * vec4(in_position, 1.0);
    gl_Position = global_ubo.projection * global_ubo.view * world_position;
    
    out_texcoord = in_texcoord;
    out_ambient = global_ubo.ambient_color;
    out_normal = mat3(u_push.normal_matrix) * in_normal;
    
    out_view_position = global_ubo.view_position.xyz;
    out_frag_position = world_position.xyz;
    out_light_space_pos = u_light_space_matrix * world_position;
}