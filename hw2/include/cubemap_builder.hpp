#pragma once

#include <vector>

#include <GL/glew.h>

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

#include "scene_storage.hpp"
#include "shader_program.hpp"
#include "blur_builder.hpp"

class cubemap_builder {
public:

    cubemap_builder() = default;

    void init(int resolution, bool with_blur = false);

    explicit cubemap_builder(int resolution, bool with_blur = false);

    void draw(
        glm::vec3 position,
        const std::vector<scene_storage *> &scenes,
        shader_program &program,
        float near, float far
    );


private:

    GLuint _fbo = 0;
    GLuint _rbo = 0;
    int _resolution = 0;
    bool _with_blur = false;

    blur_builder _blur;
    GLuint _tmp_texture = 0;
    GLuint _tmp_fbo = 0;

public:

    GLuint cubemap = 0;

};


