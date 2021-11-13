#pragma once

#include "shader_program.hpp"
#include "blur_builder.hpp"
#include "scene_storage.hpp"
#include "direction_light_object.hpp"
#include "point_light_object.hpp"

class shadow_map {
public:

    shadow_map() = default;

    void init(int target_texture, int resolution);

    shadow_map(int target_texture, int resolution);

    void draw(
        scene_storage& scene,
        const std::pair<glm::vec3, glm::vec3>& bbox,
        const direction_light_object& light_obj
    );

    void draw(
        scene_storage& scene,
        const std::pair<glm::vec3, glm::vec3>& bbox,
        const point_light_object& light_obj,
        int cube_index
    );

private:

    GLuint _shadow_map = 0;
    GLuint _fbo = 0;
    GLuint _rbo = 0;
    int _resolution = 0;
    shader_program _program;
    blur_builder _blur;

};
