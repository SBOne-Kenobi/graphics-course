#include "scene_storage.hpp"

void scene_storage::draw_objects(shader_program &program, bool use_textures, bool use_shadow_map) {
    for (auto& obj: _objects) {
        obj.draw(program, use_textures, use_shadow_map);
    }
    for (auto& obj: _objects_with_mask) {
        obj.draw(program, use_textures, use_shadow_map);
    }
}

scene_storage& scene_storage::add_object(object obj) {
    if (obj.has_mask()) {
        _objects_with_mask.push_back(std::move(obj));
    } else {
        _objects.push_back(std::move(obj));
    }
    return *this;
}

scene_storage &scene_storage::apply(const std::function<void(object&)>& func) {
    for (auto& obj: _objects) {
        func(obj);
    }
    for (auto& obj: _objects_with_mask) {
        func(obj);
    }
    return *this;
}

