#include "direction_light_object.hpp"

#include <vector>

direction_light_object::direction_light_object(const glm::vec3 &direction, const glm::vec3 &light) :
    _direction(direction), _light(light) {}

const glm::vec3 &direction_light_object::get_direction() const {
    return _direction;
}

const glm::vec3 &direction_light_object::get_light() const {
    return _light;
}

glm::mat4 direction_light_object::get_transform(const std::pair<glm::vec3, glm::vec3>& bbox) const {
    glm::vec3 bbox_center = glm::vec3(0.5) * (bbox.first + bbox.second);
    glm::vec3 light_z = _direction;
    glm::vec3 light_x = glm::normalize(glm::cross(light_z, {bbox.second.x - bbox.first.x, 0.f, 0.f}));
    glm::vec3 light_y = glm::cross(light_x, light_z);
    float shadow_scale_x = 0.f;
    float shadow_scale_y = 0.f;
    float shadow_scale_z = 0.f;
    for (auto x_v: {bbox.first.x, bbox.second.x}) {
        for (auto y_v: {bbox.first.y, bbox.second.y}) {
            for (auto z_v: {bbox.first.z, bbox.second.z}) {
                glm::vec3 v(x_v, y_v, z_v);
                v -= bbox_center;
                shadow_scale_x = fmaxf(shadow_scale_x, abs(glm::dot(v, light_x)));
                shadow_scale_y = fmaxf(shadow_scale_y, abs(glm::dot(v, light_y)));
                shadow_scale_z = fmaxf(shadow_scale_z, abs(glm::dot(v, light_z)));
            }
        }
    }
    light_x /= glm::vec3(shadow_scale_x);
    light_y /= glm::vec3(shadow_scale_y);
    light_z /= glm::vec3(shadow_scale_z);

    glm::mat4 transform = glm::transpose(glm::mat4(
        glm::vec4(light_x, 0.f),
        glm::vec4(light_y, 0.f),
        glm::vec4(light_z, 0.f),
        glm::vec4(0.f, 0.f, 0.f, 1.f)
    ));

    return transform;
}
