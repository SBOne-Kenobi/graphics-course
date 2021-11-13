#pragma once

#include <GL/glew.h>

#include <optional>
#include <utility>
#include <vector>
#include <glm/mat4x4.hpp>

#include "utils.hpp"
#include "shader_program.hpp"

class object {
public:

    explicit object(std::vector<vertex> vertices);

    object(std::vector<vertex> vertices, std::vector<int> indices);

    void draw(const shader_program &program, bool use_textures = true, bool use_shadow_map = true);

    object &with_albedo_texture(GLuint albedo_texture);

private:

    std::vector<vertex> _vertices;
    std::optional<std::vector<int>> _indices = std::nullopt;

    GLuint _vao = 0;
    GLuint _vbo = 0;
    GLuint _ebo = 0;

    std::optional<GLuint> _albedo_texture = std::nullopt;

public:

    glm::mat4 model = glm::mat4(1.f);

};


