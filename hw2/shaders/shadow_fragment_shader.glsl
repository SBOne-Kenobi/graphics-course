#version 330 core

uniform int textures_mask;

uniform sampler2D mask; // 1 << 4

layout (location = 0) out vec4 out_color;

in vec2 texcoord;

void main() {
    bool use_mask = (textures_mask & (1 << 4)) != 0;

    float z = gl_FragCoord.z;
    float dx = dFdx(z);
    float dy = dFdy(z);
    float z2 = z * z + 0.25 * (dx * dx + dy * dy);

    float alpha = use_mask ? texture(mask, texcoord).x : 1.0;

    out_color = vec4(z, z2, 0.0, alpha);
}