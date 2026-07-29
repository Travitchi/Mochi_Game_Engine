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
} u_push;

uniform mat4 u_light_space_matrix;
uniform vec2 u_uv_offset;
uniform vec2 u_uv_scale;

layout (location = 0) out vec2 out_texcoord;
layout (location = 1) out vec3 out_normal;
layout (location = 2) out vec3 out_tangent;
layout (location = 3) out vec3 out_bitangent;
layout (location = 4) out vec4 out_ambient;
layout (location = 5) out vec3 out_view_position;
layout (location = 6) out vec3 out_frag_position;
layout (location = 7) out vec4 out_light_space_pos;

void main() {
    //Slice the texture atlas!
    out_texcoord = (in_texcoord * u_uv_scale) + u_uv_offset;

    //Extract world position center and scale from the Model matrix
    vec3 world_center = u_push.model[3].xyz;
    vec2 quad_scale = vec2(length(u_push.model[0].xyz), length(u_push.model[1].xyz));

    //CYLINDRICAL BILLBOARDING
    // Extract camera Right axis from View matrix (Row 0) and lock Y to 0.0
    vec3 cam_right = normalize(vec3(global_ubo.view[0][0], 0.0, global_ubo.view[2][0]));
    vec3 cam_up = vec3(0.0, 1.0, 0.0);
    vec3 cam_forward = normalize(cross(cam_up, cam_right)); // Tangent space normal facing camera

    // Calculate final world position of the upright quad vertex
    vec3 world_position = world_center + (cam_right * in_position.x * quad_scale.x) + (cam_up * in_position.y * quad_scale.y);
    gl_Position = global_ubo.projection * global_ubo.view * vec4(world_position, 1.0);
    
    // 4. Pass Tangent-Space vectors to Fragment Shader for Normal Mapping
    out_normal = cam_forward;
    out_tangent = cam_right;
    out_bitangent = cam_up;
    
    out_ambient = global_ubo.ambient_color;
    out_view_position = global_ubo.view_position.xyz;
    out_frag_position = world_position;
    out_light_space_pos = u_light_space_matrix * vec4(world_position, 1.0);
}