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
    vec3 direction;
    vec4 color;
};

vec4 calculate_directional_light(DirectionalLight light, vec3 normal, vec3 view_dir, vec4 diffuse_sample, vec4 specular_sample, vec4 ambient_color);

void main() {
    DirectionalLight sun_light;
    sun_light.direction = vec3(0.57735, -0.57735, -0.57735); 
    sun_light.color = vec4(0.8, 0.8, 0.8, 1.0); 

    vec4 diffuse_sample = texture(diffuse_sampler, in_texcoord);
    vec4 specular_sample = texture(specular_sampler, in_texcoord);
    
    vec3 view_dir = normalize(in_view_position - in_frag_position);

    out_color = calculate_directional_light(sun_light, normalize(in_normal), view_dir, diffuse_sample, specular_sample, in_ambient);
}

vec4 calculate_directional_light(DirectionalLight light, vec3 normal, vec3 view_dir, vec4 diffuse_sample, vec4 specular_sample, vec4 ambient_color) {
    float diffuse_factor = max(dot(normal, -normalize(light.direction)), 0.0);

    vec3 half_direction = normalize(view_dir - normalize(light.direction));
    float specular_factor = pow(max(dot(normal, half_direction), 0.0), shininess);

    vec4 ambient = ambient_color * diffuse_sample;
    vec4 diffuse = light.color * diffuse_factor * diffuse_sample;
    vec4 specular = light.color * specular_factor * specular_sample;

    return vec4((ambient + diffuse + specular).rgb, diffuse_sample.a);
}