#version 330 core

uniform mat4 view;
uniform mat4 transform;
uniform mat4 projection;

layout (location = 0) in vec2 in_position;
layout (location = 1) in float in_value;

out float value;

void main()
{
    gl_Position = projection * view * transform * vec4(in_position, in_value, 1.0);
    value = in_value;
}
