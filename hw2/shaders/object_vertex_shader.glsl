#version 330 core

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

layout (location = 0) in vec3 in_position;
layout (location = 1) in vec3 in_normal;
layout (location = 2) in vec2 in_texcoord;

out vec3 position;
out vec2 texcoord;
out vec3 cam_position;
out mat3 tbn;

void main()
{
    gl_Position = projection * view * model * vec4(in_position, 1.0);
    position = (model * vec4(in_position, 1.0)).xyz;
    texcoord = in_texcoord;
    cam_position = (inverse(view) * vec4(0, 0, 0, 1)).xyz;

    vec3 normal = normalize((model * vec4(in_normal, 0.0)).xyz);
    vec3 t;
    float eps = 0.0001;
    if (abs(normal.x) > eps || abs(normal.y) > eps) {
        t = normalize(cross(vec3(0.0, 0.0, 1.0), normal));
    } else {
        t = normalize(cross(vec3(1.0, 1.0, 0.0), normal));
    }
    vec3 b = cross(t, normal);
    tbn = mat3(t, b, normal);
}
