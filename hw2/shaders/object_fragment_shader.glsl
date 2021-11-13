#version 330 core

uniform int textures_mask;

uniform sampler2D albedo_texture; // 1 << 1

uniform vec3 ambient;

uniform vec3 light_direction;
uniform vec3 light_color;

uniform mat4 transform;

uniform sampler2D shadow_map; // 1 << 0

in vec3 position;
in vec3 normal;
in vec2 texcoord;

layout (location = 0) out vec4 out_color;

void main()
{

    bool use_shadow = (textures_mask & (1 << 0)) != 0;
    bool use_albedo = (textures_mask & (1 << 1)) != 0;


    float shadow_factor = 1.0;
    if (use_shadow) {
        vec4 shadow_pos = transform * vec4(position, 1.0);
        shadow_pos /= shadow_pos.w;
        shadow_pos = shadow_pos * 0.5 + vec4(0.5);

        bool in_shadow_texture =
            (shadow_pos.x > 0.0) && (shadow_pos.x < 1.0) &&
            (shadow_pos.y > 0.0) && (shadow_pos.y < 1.0) &&
            (shadow_pos.z > 0.0) && (shadow_pos.z < 1.0);
        if (in_shadow_texture) {
            vec2 data = texture(shadow_map, shadow_pos.xy).rg;
            float mu = data.x;
            float sigma = data.y - mu * mu;
            float z = shadow_pos.z;
            z -= 0.001;
            if (z < mu) {
                shadow_factor = 1.0;
            } else {
                shadow_factor = sigma / (sigma + (z - mu) * (z - mu));
                float delta = 0.125;
                if (shadow_factor < delta) {
                    shadow_factor = 0.0;
                } else {
                    shadow_factor = (shadow_factor - delta) / (1.0 - delta);
                }
            }
        }
    }

    vec3 albedo = use_albedo ? texture(albedo_texture, texcoord).rgb : vec3(1.0, 1.0, 1.0);

    vec3 light = ambient;
    light += light_color * max(0.0, dot(normal, light_direction)) * shadow_factor;
    vec3 color = albedo * light;

    out_color = vec4(color, 1.0);
}
