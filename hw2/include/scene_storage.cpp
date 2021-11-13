#include "scene_storage.hpp"

void scene_storage::draw_objects(shader_program &program, bool use_textures, bool use_shadow_map) {
    for (auto& obj: _objects) {
        obj.draw(program, use_textures, use_shadow_map);
    }
}

void scene_storage::add_object(object obj) {
    _objects.push_back(std::move(obj));
}

scene_storage &scene_storage::apply(const std::function<void(object&)>& func) {
    for (auto& obj: _objects) {
        func(obj);
    }
    return *this;
}

