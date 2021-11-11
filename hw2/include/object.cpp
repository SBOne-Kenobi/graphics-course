#include "object.hpp"

object::object(std::vector<vertex> vertices) : _vertices(std::move(vertices)) {
    glGenVertexArrays(1, &_vao);
    glGenBuffers(1, &_vbo);
    init_vao_vertex(_vao);

    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glBufferData(GL_ARRAY_BUFFER, _vertices.size() * sizeof(_vertices[0]), _vertices.data(), GL_DYNAMIC_COPY);
}

object::object(std::vector<vertex> vertices, std::vector<int> indices) : object(std::move(vertices)) {
    _indices = std::move(indices);

    glGenBuffers(1, &_ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, _indices->size() * sizeof(_indices.value()[0]), _indices->data(),
                 GL_DYNAMIC_COPY);
}

void object::draw(const shader_program &program) {
    glBindVertexArray(_vao);
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glUniformMatrix4fv(program["model"], 1, GL_FALSE, reinterpret_cast<float *>(&model));

    if (_indices.has_value()) {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ebo);
        glDrawElements(GL_TRIANGLES, _indices->size(), GL_UNSIGNED_INT, nullptr);
    } else {
        glDrawArrays(GL_TRIANGLES, 0, _vertices.size());
    }

}
