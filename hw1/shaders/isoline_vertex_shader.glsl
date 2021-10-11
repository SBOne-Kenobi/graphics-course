#version 330 core

uniform mat4 view;
uniform mat4 transform;
uniform mat4 projection;

layout (location = 0) in vec3 in_position;

void main()
{
    gl_Position = projection * (view * transform * vec4(in_position, 1.0) + vec4(0, 0, 0.008, 0));
}
