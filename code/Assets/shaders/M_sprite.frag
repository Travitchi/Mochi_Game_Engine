#version 460 core

layout (location = 0) in vec2 in_texcoord;
layout (location = 1) in vec3 in_normal;
layout (location = 2) in vec3 in_tangent;
layout (location = 3) in vec3 in_bitangent;
layout (location = 4) in vec4 in_ambient;
layout (location = 5) in vec3 in_view_position;
layout (location = 6) in vec3 in_frag_position;
layout (location = 7) in vec4 in_light_space_pos;

layout (location = 0) out vec4 out_color;

uniform sampler2D diffuse_sampler;
uniform sampler2D normal_sampler;  // Tangent-space normal map!
uniform float shininess;
uniform sampler2D u_shadow_sampler;

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

float calculate_shadow(vec4 light_space_pos, vec3 normal, vec3 light_dir) 
{
    vec3 proj_coords = light_space_pos.xyz / light_space_pos.w;
    proj_coords = proj_coords * 0.5 + 0.5;

    if (proj_coords.z > 1.0 || proj_coords.x < 0.0 || proj_coords.x > 1.0 || proj_coords.y < 0.0 || proj_coords.y > 1.0) {
        return 0.0;
    }

    float current_depth = proj_coords.z;
    float bias = max(0.005 * (1.0 - dot(normal, light_dir)), 0.001);

    float shadow = 0.0;
    vec2 texel_size = 1.0 / textureSize(u_shadow_sampler, 0); 

    for(int x = -1; x <= 1; ++x) 
    {
        for(int y = -1; y <= 1; ++y) 
        {
            float pcf_depth = texture(u_shadow_sampler, proj_coords.xy + vec2(x, y) * texel_size).r; 
            shadow += current_depth - bias > pcf_depth ? 1.0 : 0.0;        
        }    
    }
    return shadow / 9.0;
}

vec4 calculate_directional_light(DirectionalLight light, vec3 normal, vec3 view_dir, vec4 diffuse_sample, vec4 ambient_color, float shadow);
vec4 calculate_point_light(PointLight light, vec3 normal, vec3 frag_position, vec3 view_dir, vec4 diffuse_sample, vec4 ambient_color);

void main() {
    vec4 diffuse_sample = texture(diffuse_sampler, in_texcoord);
    
    // CRITICAL HD-2D STEP: Alpha Discard!
    // Prevents transparent quad boundaries from rendering black or casting solid box shadows
    if (diffuse_sample.a < 0.1) {
        discard;
    }

    // Convert 2D Tangent-Space normal map into 3D World Space
    vec3 normal_map = texture(normal_sampler, in_texcoord).rgb;
    normal_map = normalize(normal_map * 2.0 - 1.0);
    
    mat3 TBN = mat3(normalize(in_tangent), normalize(in_bitangent), normalize(in_normal));
    vec3 world_normal = normalize(TBN * normal_map);
    
    vec3 view_dir = normalize(in_view_position - in_frag_position);
    float shadow = calculate_shadow(in_light_space_pos, world_normal, -normalize(global_ubo.dir_light.direction.xyz));

    vec4 final_color = calculate_directional_light(global_ubo.dir_light, world_normal, view_dir, diffuse_sample, in_ambient, shadow);

    for (uint i = 0; i < global_ubo.num_p_lights; ++i) {
        final_color += calculate_point_light(global_ubo.p_lights[i], world_normal, in_frag_position, view_dir, diffuse_sample, in_ambient);
    }
    
    out_color = vec4(final_color.rgb, diffuse_sample.a);
}

vec4 calculate_directional_light(DirectionalLight light, vec3 normal, vec3 view_dir, vec4 diffuse_sample, vec4 ambient_color, float shadow)
{
    float diffuse_factor = max(dot(normal, -normalize(light.direction.xyz)), 0.0);
    vec3 half_direction = normalize(view_dir - normalize(light.direction.xyz));
    float specular_factor = pow(max(dot(normal, half_direction), 0.0), shininess);
    
    vec4 ambient = ambient_color * diffuse_sample;
    vec4 diffuse = light.color * diffuse_factor * diffuse_sample;
    vec4 specular = light.color * specular_factor * 0.2; // Subtler specular for pixel art
    
    return ambient + ((1.0 - shadow) * (diffuse + specular));
}

vec4 calculate_point_light(PointLight light, vec3 normal, vec3 frag_position, vec3 view_dir, vec4 diffuse_sample, vec4 ambient_color) {
    vec3 light_dir = normalize(light.position.xyz - frag_position);
    
    float diffuse_factor = max(dot(normal, light_dir), 0.0);
    vec3 half_direction = normalize(view_dir + light_dir); 
    float specular_factor = pow(max(dot(normal, half_direction), 0.0), shininess);

    float distance = length(light.position.xyz - frag_position);
    float attenuation = 1.0 / (light.attenuation.x + light.attenuation.y * distance + light.attenuation.z * (distance * distance));

    vec4 ambient = ambient_color * diffuse_sample;
    vec4 diffuse = light.color * diffuse_factor * diffuse_sample;
    vec4 specular = light.color * specular_factor * 0.2;

    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;

    return ambient + diffuse + specular;
}