#version 460 core
in vec2 out_texcoord;
out vec4 out_color;

uniform sampler2D diffuse_sampler;

void main() {
    out_color = texture(diffuse_sampler, out_texcoord);
}