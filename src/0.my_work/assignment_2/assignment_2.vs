#version 330 core
layout (location = 0) in vec3 aPos;     // position
layout (location = 1) in vec3 aColor;   // color

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 FragPos;
out vec3 VertexColor;

const float STAR_BASE_SIZE = 800.0;

void main()
{
    vec4 worldPos = model * vec4(aPos, 1.0);
    vec4 clipPos = projection * view * worldPos;
    FragPos = vec3(worldPos);
    VertexColor = aColor;
    gl_Position = clipPos;
    gl_PointSize = STAR_BASE_SIZE / min(clipPos.w, 0.1);
}