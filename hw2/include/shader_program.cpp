#include "shader_program.hpp"

#include "utils.hpp"

shader_program::shader_program(const char *vertex_source, const char *fragment_source) {
    init(vertex_source, fragment_source);
}

void shader_program::init(const char *vertex_source, const char *fragment_source) {
    _vertex_shader = create_shader(GL_VERTEX_SHADER, vertex_source);
    _fragment_shader = create_shader(GL_FRAGMENT_SHADER, fragment_source);
    _program = create_program(_vertex_shader, _fragment_shader);
}

shader_program::operator GLuint() const {
    return _program;
}

GLint shader_program::operator[](const std::string& key) const {
    if (!_locations.contains(key)) {
        _locations[key] = glGetUniformLocation(_program, key.c_str());
    }
    return _locations[key];
}

void shader_program::bind() const {
    glUseProgram(_program);
}
