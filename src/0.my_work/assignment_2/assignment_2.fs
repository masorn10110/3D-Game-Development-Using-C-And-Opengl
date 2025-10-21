#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 VertexColor;

void main()
{
    vec2 coord = gl_PointCoord * 2.0 - 1.0;
    float dist = dot(coord, coord);
    if (dist > 1.0) discard;

    float brightness = smoothstep(1.0, 0.0, dist);
    float radiusFactor = clamp(length(FragPos) / 10.0, 0.0, 1.0);
    vec3 centerColor = vec3(1.0, 0.9, 0.6);
    vec3 edgeColor = vec3(0.2, 0.6, 1.0);
    vec3 color = mix(centerColor, edgeColor, radiusFactor);

    FragColor = vec4(color * brightness, 1.0);
}