#version 330 core
out vec4 FragColor;

// in vec3 ourColor;

uniform vec3 myColor;

void main()
{
    FragColor = vec4(myColor, 1.0f);
}