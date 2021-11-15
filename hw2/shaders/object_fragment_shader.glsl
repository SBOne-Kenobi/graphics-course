#version 330 core

uniform int textures_mask;

uniform sampler2D shadow_map; // 1 << 0
uniform sampler2D albedo_texture; // 1 << 1
uniform sampler2D specular_map; // 1 << 2
uniform sampler2D norm_map; // 1 << 3
uniform sampler2D mask; // 1 << 4

uniform vec3 ambient;

uniform float specular_power;
uniform vec3 specular_color;

uniform vec3 light_direction;
uniform vec3 light_color;

uniform mat4 transform;

uniform int point_light_number;
uniform vec3 point_light_color[32];
uniform vec3 point_light_attenuation[32];
uniform vec3 point_light_position[32];

in vec3 position;
in vec2 texcoord;
in vec3 cam_position;
in mat3 tbn;

layout (location = 0) out vec4 out_color;

void main()
{
    bool use_shadow = (textures_mask & (1 << 0)) != 0;
    bool use_albedo = (textures_mask & (1 << 1)) != 0;
    bool use_specular = (textures_mask & (1 << 2)) != 0;
    bool use_norm = (textures_mask & (1 << 3)) != 0;
    bool use_mask = (textures_mask & (1 << 4)) != 0;

    vec3 normal_vec = use_norm ? normalize(2 * texture(norm_map, texcoord).xyz - 1) : vec3(0.0, 0.0, 1.0);
    normal_vec = normalize(tbn * normal_vec);

    vec3 cam_direction = normalize(cam_position - position);

    float specular_factor = use_specular ? texture(specular_map, texcoord).x : 1.0;

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
                float delta = 0.6;
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

    light += light_color * max(0.0, dot(normal_vec, light_direction)) * shadow_factor;
    vec3 color = albedo * light;

    float light_cosine = dot(normal_vec, light_direction);
    vec3 reflected = 2.0 * normal_vec * light_cosine - light_direction;
    float specular = pow(max(0.0, dot(reflected, cam_direction)), specular_power) * specular_factor;
    color += shadow_factor * specular_color * specular;

    for (int i = 0; i < point_light_number; i++) {
        vec3 point_light_vector = point_light_position[i] - position;
        vec3 point_light_direction = normalize(point_light_vector);
        float point_light_cosine = dot(normal_vec, point_light_direction);
        float point_light_factor = max(0.0, point_light_cosine);

        float point_light_distance = length(point_light_vector);
        float point_light_intensity = 1.0 / dot(point_light_attenuation[i],
            vec3(1.0, point_light_distance, point_light_distance * point_light_distance));

        color += albedo * point_light_color[i] * point_light_factor * point_light_intensity;

        vec3 point_reflected = 2.0 * normal_vec * point_light_cosine - point_light_direction;
        float point_specular = pow(max(0.0, dot(point_reflected, cam_direction)), specular_power) * specular_factor;
        float specular_intensity = 1.0 / dot(vec2(1.0, 0.1),
            vec2(1.0, point_light_distance));
        color += specular_intensity * point_light_color[i] * specular_color * point_specular;
    }

    color = color / (1.0 + color);

    float alpha = use_mask ? texture(mask, texcoord).x : 1.0;

    out_color = vec4(color, alpha);
}
