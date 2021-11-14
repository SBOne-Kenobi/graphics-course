#include "point_light_object.hpp"

point_light_object::point_light_object(
    const glm::vec3 &position,
    const glm::vec3 &light,
    const glm::vec3& attenuation
) : position(position), light(light), attenuation(attenuation) {}


