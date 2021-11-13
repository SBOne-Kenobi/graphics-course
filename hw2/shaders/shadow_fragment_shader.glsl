#version 330 core

layout (location = 0) out vec4 out_color;

void main() {
    float z = gl_FragCoord.z;
    float dx = dFdx(z);
    float dy = dFdy(z);
    float z2 = z * z + 0.25 * (dx * dx + dy * dy);
    out_color = vec4(z, z2, 0.0, 0.0);
}