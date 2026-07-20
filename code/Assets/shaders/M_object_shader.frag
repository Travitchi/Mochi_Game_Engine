#version 460 core

layout (location = 0) in vec2 in_texcoord;
layout (location = 1) in vec3 in_normal;
layout (location = 2) in vec4 in_ambient;
layout (location = 3) in vec3 in_view_position;
layout (location = 4) in vec3 in_frag_position;

layout (location = 0) out vec4 out_color;

uniform sampler2D diffuse_sampler;
uniform sampler2D specular_sampler;
uniform float shininess;

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

vec4 calculate_directional_light(DirectionalLight light, vec3 normal, vec3 view_dir, vec4 diffuse_sample, vec4 specular_sample, vec4 ambient_color);
vec4 calculate_point_light(PointLight light, vec3 normal, vec3 frag_position, vec3 view_dir, vec4 diffuse_sample, vec4 specular_sample, vec4 ambient_color);

void main() {
    vec4 diffuse_sample = texture(diffuse_sampler, in_texcoord);
    vec4 specular_sample = texture(specular_sampler, in_texcoord);
    
    vec3 view_dir = normalize(in_view_position - in_frag_position);
    vec3 normal = normalize(in_normal);
    vec4 final_color = calculate_directional_light(global_ubo.dir_light, normal, view_dir, diffuse_sample, specular_sample, in_ambient);

    for (uint i = 0; i < global_ubo.num_p_lights; ++i) {
        final_color += calculate_point_light(global_ubo.p_lights[i], normal, in_frag_position, view_dir, diffuse_sample, specular_sample, in_ambient);
    }

    out_color = vec4(final_color.rgb, diffuse_sample.a);
}
vec4 calculate_directional_light(DirectionalLight light, vec3 normal, vec3 view_dir, vec4 diffuse_sample, vec4 specular_sample, vec4 ambient_color) {
    // Note: Using light.direction.xyz because it is a vec4
    float diffuse_factor = max(dot(normal, -normalize(light.direction.xyz)), 0.0);

    vec3 half_direction = normalize(view_dir - normalize(light.direction.xyz));
    float specular_factor = pow(max(dot(normal, half_direction), 0.0), shininess);

    vec4 ambient = ambient_color * diffuse_sample;
    vec4 diffuse = light.color * diffuse_factor * diffuse_sample;
    vec4 specular = light.color * specular_factor * specular_sample;

    return ambient + diffuse + specular;
}

vec4 calculate_point_light(PointLight light, vec3 normal, vec3 frag_position, vec3 view_dir, vec4 diffuse_sample, vec4 specular_sample, vec4 ambient_color) {
    vec3 light_dir = normalize(light.position.xyz - frag_position);
    
    // Diffuse
    float diffuse_factor = max(dot(normal, light_dir), 0.0);

    // Specular
    vec3 half_direction = normalize(view_dir + light_dir); 
    float specular_factor = pow(max(dot(normal, half_direction), 0.0), shininess);

    // Attenuation
    float distance = length(light.position.xyz - frag_position);
    float attenuation = 1.0 / (light.attenuation.x + light.attenuation.y * distance + light.attenuation.z * (distance * distance));

    // Combine
    vec4 ambient = ambient_color * diffuse_sample;
    vec4 diffuse = light.color * diffuse_factor * diffuse_sample;
    vec4 specular = light.color * specular_factor * specular_sample;

    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;

    return ambient + diffuse + specular;
}