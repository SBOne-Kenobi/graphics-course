#pragma once

#include <vector>

#include <GL/glew.h>

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

#include "scene_storage.hpp"
#include "shader_program.hpp"

class cubemap_builder {
public:

    cubemap_builder() = default;

    void init(int resolution);

    explicit cubemap_builder(int resolution);

    void draw(
        glm::vec3 position,
        const std::vector<scene_storage*>& scenes,
        shader_program& program,
        float near, float far
    ) const;


private:

    GLuint _fbo = 0;
    GLuint _rbo = 0;
    int _resolution = 0;

public:

    GLuint cubemap = 0;

};


