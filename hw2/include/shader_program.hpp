#pragma once

#include <GL/glew.h>

#include <unordered_map>
#include <string>

class shader_program {
public:

    shader_program(const char *vertex_source, const char *fragment_source);

    explicit operator GLuint() const;

    GLint operator[](const std::string& key) const;

    void bind() const;

private:

    GLuint vertex_shader = 0;
    GLuint fragment_shader = 0;
    GLuint program = 0;
    mutable std::unordered_map<std::string, GLint> locations;

};

