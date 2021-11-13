#pragma once

#include "glm/vec3.hpp"
#include "glm/mat4x4.hpp"


class direction_light_object {
public:

    direction_light_object(const glm::vec3 &direction, const glm::vec3 &light);

    glm::mat4 get_transform(const std::pair<glm::vec3, glm::vec3>& bbox) const;

public:

    glm::vec3 direction;
    glm::vec3 light;

};


