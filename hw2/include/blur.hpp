#pragma once

#include "shader_program.hpp"

class blur {
public:

    blur(int target_texture, GLuint texture, int format, int width, int height, int fbo = -1);

    void do_blur(int N = 7, float radius = 5.0);

private:
    GLuint vao = 0;
    GLuint fbo_x = 0;
    GLuint fbo_y = 0;
    GLuint texture = 0;
    GLuint tmp_texture = 0;
    int target_texture;
    int width;
    int height;
    shader_program program;

};


