#pragma once

#include <vector>

#include "object.hpp"
#include "shader_program.hpp"

class scene_storage {
public:

    void draw_objects(shader_program& program);

    virtual ~scene_storage() = default;

private:

    std::vector<object> _objects;

};
