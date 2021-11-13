#include "object.hpp"

object::object(std::vector<vertex> vertices) : _vertices(std::move(vertices)) {
    glGenVertexArrays(1, &_vao);
    glGenBuffers(1, &_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glBufferData(GL_ARRAY_BUFFER, _vertices.size() * sizeof(_vertices[0]), _vertices.data(), GL_DYNAMIC_COPY);
    init_vao_vertex(_vao);
}

object::object(std::vector<vertex> vertices, std::vector<int> indices) : object(std::move(vertices)) {
    _indices = std::move(indices);

    glGenBuffers(1, &_ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, _indices.value().size() * sizeof(_indices.value()[0]), _indices.value().data(),
                 GL_DYNAMIC_COPY);
}

#include "iostream"

void object::draw(const shader_program &program, bool use_textures, bool use_shadow_map) {
    glBindVertexArray(_vao);
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glUniformMatrix4fv(program["model"], 1, GL_FALSE, reinterpret_cast<float *>(&model));

    int textures_mask = 0;

    if (use_textures) {
        if (_albedo_texture.has_value()) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, _albedo_texture.value());
            glUniform1i(program["albedo_texture"], 0);
            textures_mask |= (1 << 1);
        }
    }

    if (use_shadow_map) {
        textures_mask |= (1 << 0);
    }

    glUniform1i(program["textures_mask"], textures_mask);

    if (_indices.has_value()) {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ebo);
        glDrawElements(GL_TRIANGLES, _indices.value().size(), GL_UNSIGNED_INT, nullptr);
    } else {
        glDrawArrays(GL_TRIANGLES, 0, _vertices.size());
    }

}

object &object::with_albedo_texture(GLuint albedo_texture) {
    _albedo_texture = albedo_texture;
    return *this;
}
