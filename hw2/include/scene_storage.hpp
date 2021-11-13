#pragma once

#include <vector>
#include <functional>

#include "object.hpp"
#include "shader_program.hpp"

class scene_storage {
public:

    void draw_objects(shader_program& program, bool use_textures = true, bool use_shadow_map = true);

    void add_object(object obj);

    scene_storage& apply(const std::function<void(object&)>& func);

    virtual ~scene_storage() = default;

private:

    std::vector<object> _objects;

};
