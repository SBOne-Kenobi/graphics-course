#pragma once

#include <GL/glew.h>
#include <glm/vec3.hpp>


struct vertex {
    glm::vec3 position;
    glm::vec3 normal;
};

void init_vao_vertex(GLuint vao);

GLuint create_shader(GLenum type, const char *source);

GLuint create_program(GLuint vertex_shader, GLuint fragment_shader);

