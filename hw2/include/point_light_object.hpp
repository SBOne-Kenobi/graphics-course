#pragma once

#include "glm/vec3.hpp"
#include "glm/mat4x4.hpp"

class point_light_object {
public:

    point_light_object(const glm::vec3 &position, const glm::vec3 &light);

    const glm::vec3& get_position() const;

    const glm::vec3 &get_light() const;

    glm::mat4 get_transform(const std::pair<glm::vec3, glm::vec3>& bbox, int cube_index) const;

private:

    glm::vec3 _position;
    glm::vec3 _light;

};


