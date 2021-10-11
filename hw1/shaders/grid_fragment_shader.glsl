#version 330 core

layout (location = 0) out vec4 out_color;

void main()
{
    vec3 color = vec3(1, 1, 1) * 0.5;
    out_color = vec4(color, 1.0);
}
