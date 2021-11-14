#pragma once

#include "glm/vec3.hpp"
#include "glm/mat4x4.hpp"

class point_light_object {
public:

    point_light_object(const glm::vec3 &position, const glm::vec3 &light, const glm::vec3 &attenuation);

public:

    glm::vec3 position;
    glm::vec3 light;
    glm::vec3 attenuation;

};


