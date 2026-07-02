#version 460 core

out vec4 FragColor;

void main()
{
    // RGBA format (Red, Green, Blue, Alpha)
    // 1.0 Red + 0.5 Green + 0.0 Blue = Bright Orange!
    FragColor = vec4(1.0, 0.5, 0.0, 1.0); 
}