#pragma once

#include <GL/glew.h>

#include <unordered_map>
#include <string>

class shader_program {
public:

    shader_program() = default;

    shader_program(const char *vertex_source, const char *fragment_source);

    void init(const char *vertex_source, const char *fragment_source);

    explicit operator GLuint() const;

    GLint operator[](const std::string& key) const;

    void bind() const;

private:

    GLuint _vertex_shader = 0;
    GLuint _fragment_shader = 0;
    GLuint _program = 0;
    mutable std::unordered_map<std::string, GLint> _locations;

};

