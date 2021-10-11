#version 330 core

uniform mat4 view;
uniform mat4 transform;
uniform mat4 projection;

uniform float z0;
uniform float z1;
uniform int x_size;
uniform int y_size;
uniform int z_size;

void main()
{
    mat4 tr = projection * view * transform;

    float x_best = -1;
    float y_best = 1;
    float best = -1;

    for (int i = -1; i <= 1; i += 2) {
        float x = float(i);
        for (int j = -1; j <= 1; j += 2) {
            float y = float(j);
            vec4 cur = tr * vec4(x, y, 0, 1);
            float cur_l = -10;
            if (cur.w != 0) {
                cur_l = cur.z / cur.w;
            }
            if (cur_l < 0 || cur_l > best) {
                best = cur_l;
                x_best = x;
                y_best = y;
            }
            if (best < -5) {
                break;
            }
        }
        if (best < -5) {
            break;
        }
    }

    vec3 position;

    int end = gl_VertexID % 2;
    int id = (gl_VertexID - end) / 2;

    if (id < x_size) {
        float t = float(id) / float(x_size - 1);
        position.x = 2 * t - 1;
        position.y = y_best;
        position.z = (1 - end) * z0 + end * z1;
    } else if (id < x_size + y_size) {
        float t = float(id - x_size) / float(y_size - 1);
        position.x = x_best;
        position.y = 2 * t - 1;
        position.z = (1 - end) * z0 + end * z1;
    } else if (id < x_size + y_size + z_size) {
        float t = float(id - x_size - y_size) / float(z_size - 1);
        position.x = 2 * end - 1;
        position.y = y_best;
        position.z = (1 - t) * z0 + t * z1;
    } else {
        float t = float(id - x_size - y_size - z_size) / float(z_size - 1);
        position.x = x_best;
        position.y = 2 * end - 1;
        position.z = (1 - t) * z0 + t * z1;
    }

    position.z *= 1.3;

    gl_Position = tr * vec4(position, 1.0);
}
