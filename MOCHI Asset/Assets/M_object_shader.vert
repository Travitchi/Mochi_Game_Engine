#version 460 core

//The Matrices coming from your M_ObjectShader C++ class
uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

void main()
{
    // Multiply the matrices against the vertex position to place it on the screen
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}