#pragma once

#include "glm/vec3.hpp"
#include "glm/mat4x4.hpp"


class direction_light_object {
public:

    direction_light_object(const glm::vec3 &direction, const glm::vec3 &light);

    const glm::vec3 &get_direction() const;

    const glm::vec3 &get_light() const;

    glm::mat4 get_transform(const std::pair<glm::vec3, glm::vec3>& bbox) const;

private:

    glm::vec3 _direction;
    glm::vec3 _light;

};


