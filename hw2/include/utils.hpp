#pragma once

#include <GL/glew.h>
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>

struct vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texcoord;
};

void init_vao_vertex(GLuint vao);

GLuint create_shader(GLenum type, const char *source);

GLuint create_program(GLuint vertex_shader, GLuint fragment_shader);
