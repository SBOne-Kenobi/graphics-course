#include "scene_storage.hpp"

void scene_storage::draw_objects(shader_program &program) {
    for (auto& obj: _objects) {
        obj.draw(program);
    }
}
