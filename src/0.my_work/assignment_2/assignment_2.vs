#version 330 core
layout (location = 0) in vec3 aPos;     // sphere vertex
layout (location = 1) in vec3 instancePos; // star position
layout (location = 2) in vec3 instanceColor; // star color

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 VertexColor;

void main()
{
    vec4 worldPos = model * vec4(aPos + instancePos, 1.0);
    gl_Position = projection * view * worldPos;
    VertexColor = instanceColor;
}