#version 460 core

in vec2 frag_texcoord;
in vec3 frag_normal;

layout(binding = 0) uniform sampler2D diffuse_sampler;
out vec4 out_color;

void main()
{
    out_color = texture(diffuse_sampler, frag_texcoord);
}