#include "object.hpp"

object::object(std::vector<vertex> vertices, const glm::vec3& specular_color, float specular_power) :
    vertices(std::move(vertices)), _specular_color(specular_color), _specular_power(specular_power) {
    glGenVertexArrays(1, &_vao);
    glGenBuffers(1, &_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glBufferData(GL_ARRAY_BUFFER, this->vertices.size() * sizeof(this->vertices[0]), this->vertices.data(), GL_DYNAMIC_COPY);
    init_vao_vertex(_vao);
}

void object::draw(const shader_program &program, bool use_textures, bool use_shadow_map) {
    glBindVertexArray(_vao);
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glUniformMatrix4fv(program["model"], 1, GL_FALSE, reinterpret_cast<float *>(&model));

    int textures_mask = 0;

    if (use_textures) {
        if (_albedo_texture.has_value()) {
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, _albedo_texture.value());
            glUniform1i(program["albedo_texture"], 1);
            textures_mask |= (1 << 1);
        }

        if (_specular_map.has_value()) {
            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, _specular_map.value());
            glUniform1i(program["specular_map"], 2);
            textures_mask |= (1 << 2);
        }

        if (_norm_map.has_value()) {
            glActiveTexture(GL_TEXTURE3);
            glBindTexture(GL_TEXTURE_2D, _norm_map.value());
            glUniform1i(program["norm_map"], 3);
            textures_mask |= (1 << 3);
        }
    }

    if (_mask.has_value()) {
        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, _mask.value());
        glUniform1i(program["mask"], 4);
        textures_mask |= (1 << 4);
    }

    if (use_shadow_map) {
        textures_mask |= (1 << 0);
    }

    glUniform1i(program["textures_mask"], textures_mask);
    glUniform1f(program["specular_power"], _specular_power);
    glUniform3fv(program["specular_color"], 1, reinterpret_cast<float *>(&_specular_color));

    if (_indices.has_value()) {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ebo);
        glDrawElements(GL_TRIANGLES, _indices.value().size(), GL_UNSIGNED_INT, nullptr);
    } else {
        glDrawArrays(GL_TRIANGLES, 0, vertices.size());
    }
}

object &object::with_albedo_texture(GLuint albedo_texture) {
    _albedo_texture = albedo_texture;
    return *this;
}

object &object::with_indices(std::vector<int> indices) {
    _indices = std::move(indices);
    glGenBuffers(1, &_ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, _indices.value().size() * sizeof(_indices.value()[0]),
                 _indices.value().data(), GL_DYNAMIC_COPY);
    return *this;
}

object &object::with_specular_map(GLuint specular_map) {
    _specular_map = specular_map;
    return *this;
}

object &object::with_norm_map(GLuint norm_map) {
    _norm_map = norm_map;
    return *this;
}

object &object::with_mask(GLuint mask) {
    _mask = mask;
    return *this;
}

bool object::has_mask() const {
    return _mask.has_value();
}
