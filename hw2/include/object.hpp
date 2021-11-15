#pragma once

#include <GL/glew.h>

#include <optional>
#include <functional>
#include <utility>
#include <vector>

#include <glm/mat4x4.hpp>

#include "utils.hpp"
#include "shader_program.hpp"

class object {
public:

    object(std::vector<vertex> vertices, const glm::vec3& specular_color, float specular_power);

    void draw(const shader_program &program, bool use_textures = true, bool use_shadow_map = true);

    object& with_albedo_texture(GLuint albedo_texture);

    object& with_indices(std::vector<int> indices);

    object& with_specular_map(GLuint specular_map);

    object& with_norm_map(GLuint norm_map);

    object& with_mask(GLuint mask);

    bool has_mask() const;

private:

    std::optional<std::vector<int>> _indices = std::nullopt;

    GLuint _vao = 0;
    GLuint _vbo = 0;
    GLuint _ebo = 0;

    std::optional<GLuint> _albedo_texture = std::nullopt;
    std::optional<GLuint> _specular_map = std::nullopt;
    std::optional<GLuint> _norm_map = std::nullopt;
    std::optional<GLuint> _mask = std::nullopt;

    glm::vec3 _specular_color;
    float _specular_power;

public:

    std::vector<vertex> vertices;
    glm::mat4 model = glm::mat4(1.f);

};


