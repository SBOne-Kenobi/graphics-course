#pragma once

#include "shader_program.hpp"
#include "blur_builder.hpp"
#include "scene_storage.hpp"
#include "direction_light_object.hpp"
#include "point_light_object.hpp"

class shadow_map_builder {
public:

    shadow_map_builder() = default;

    void init(int target_texture, int resolution);

    shadow_map_builder(int target_texture, int resolution);

    void draw(
        const std::vector<scene_storage*>& scenes,
        const std::pair<glm::vec3, glm::vec3>& bbox,
        const direction_light_object& light_obj
    );

private:

    GLuint _fbo = 0;
    GLuint _rbo = 0;
    int _resolution = 0;
    shader_program _program;
    blur_builder _blur;

public:

    GLuint shadow_map = 0;

};
