#version 330 core

uniform mat4 model;
uniform mat4 transform;

layout (location = 0) in vec3 in_position;

void main() {
    gl_Position = transform * model * vec4(in_position, 1.0);
}