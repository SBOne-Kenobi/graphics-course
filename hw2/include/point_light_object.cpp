#include "point_light_object.hpp"

const glm::vec3 &point_light_object::get_position() const {
    return _position;
}

const glm::vec3 &point_light_object::get_light() const {
    return _light;
}

point_light_object::point_light_object(const glm::vec3 &position, const glm::vec3 &light) :
    _position(position), _light(light) {}

glm::mat4 point_light_object::get_transform(const std::pair<glm::vec3, glm::vec3> &bbox, int cube_index) const {
    // TODO: realise
    //
    // 0 - right
    // 1 - left
    // 2 - top
    // 3 - bottom
    // 4 - front
    // 5 - back
    //
    return glm::mat4();
}

