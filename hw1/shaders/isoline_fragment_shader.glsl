#version 330 core

layout (location = 0) out vec4 out_color;

void main()
{
    vec3 color = vec3(116, 66, 200) / 255;
//    vec3 color = vec3(148, 0, 211) / 255;
//    vec3 color = vec3(215, 24, 104) / 255;
    out_color = vec4(color, 1.0);
}
