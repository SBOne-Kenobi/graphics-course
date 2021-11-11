#version 330 core

uniform sampler2D target;
uniform int mode;
uniform int N;
uniform float radius;

in vec2 texcoord;

layout (location = 0) out vec4 out_color;

void main()
{
    vec2 dir = vec2(0.0, 1.0);
    if (mode == 1) {
        dir = vec2(1.0, 0.0);
    }

    vec4 sum = vec4(0.0);
    float sum_w = 0.0;
    for (int i = -N; i <= N; i++) {
        float w = exp(-i*i / (radius*radius));
        vec2 vector = i * dir / vec2(textureSize(target, 0));
        sum += w * texture(target, texcoord + vector);
        sum_w += w;
    }

    out_color = sum / sum_w;
}
