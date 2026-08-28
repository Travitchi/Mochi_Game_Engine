#version 460 core

layout (location = 0) in vec2 in_texcoord;
layout (location = 6) in vec3 in_frag_position;
layout (location = 7) in vec4 in_light_space_pos;

layout (location = 0) out vec4 out_color;

uniform sampler2D diffuse_sampler;
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

float calculate_shadow(vec4 light_space_pos) 
{
    vec3 proj_coords = light_space_pos.xyz / light_space_pos.w;
    proj_coords = proj_coords * 0.5 + 0.5;

    if (proj_coords.z > 1.0 || proj_coords.x < 0.0 || proj_coords.x > 1.0 || proj_coords.y < 0.0 || proj_coords.y > 1.0) {
        return 0.0;
    }

    float current_depth = proj_coords.z;
    float bias = 0.005;
    float shadow = 0.0;
    vec2 texel_size = 1.0 / textureSize(u_shadow_sampler, 0); 

    for (int x = -1; x <= 1; ++x) 
    {
        for (int y = -1; y <= 1; ++y) 
        {
            float pcf_depth = texture(u_shadow_sampler, proj_coords.xy + vec2(x, y) * texel_size).r; 
            shadow += current_depth - bias > pcf_depth ? 1.0 : 0.0;        
        }    
    }
    return shadow / 9.0;
}

void main() {
    vec4 diffuse_sample = texture(diffuse_sampler, in_texcoord);
    
    if (diffuse_sample.a < 0.1) {
        discard;
    }

    float shadow = calculate_shadow(in_light_space_pos);

    // 1. Calculate Base Light (Pure white 1.0 in the sun, tinted ambient room color in the shade)
    vec3 base_light = mix(global_ubo.ambient_color.rgb, vec3(1.0), 1.0 - shadow);

    // 2. Accumulate Point Lights
    vec3 point_light_total = vec3(0.0);
    for (uint i = 0; i < global_ubo.num_p_lights; ++i) {
        float dist = length(global_ubo.p_lights[i].position.xyz - in_frag_position);
        float attenuation = 1.0 / (global_ubo.p_lights[i].attenuation.x + 
                                  global_ubo.p_lights[i].attenuation.y * dist + 
                                  global_ubo.p_lights[i].attenuation.z * (dist * dist));
        point_light_total += global_ubo.p_lights[i].color.rgb * attenuation;
    }

    // 3. Combine and Clamp! (Total light can NEVER exceed 100% white)
    vec3 total_light = clamp(base_light + point_light_total, 0.0, 1.0);

    // 4. Safely multiply the sprite's true colors by the calculated light
    vec3 final_rgb = diffuse_sample.rgb * total_light;
    
    out_color = vec4(final_rgb, diffuse_sample.a);
}