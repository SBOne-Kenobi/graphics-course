#include "shader_program.hpp"

#include "utils.hpp"

shader_program::shader_program(const char *vertex_source, const char *fragment_source) {
    vertex_shader = create_shader(GL_VERTEX_SHADER, vertex_source);
    fragment_shader = create_shader(GL_FRAGMENT_SHADER, fragment_source);
    program = create_program(vertex_shader, fragment_shader);
}

shader_program::operator GLuint() const {
    return program;
}

GLint shader_program::operator[](const std::string& key) const {
    if (!locations.contains(key)) {
        locations[key] = glGetUniformLocation(program, key.c_str());
    }
    return locations[key];
}

void shader_program::bind() const {
    glUseProgram(program);
}
