#pragma once

#include "shader_program.hpp"

class blur_builder {
public:

    blur_builder() = default;

    blur_builder(int target_texture, GLuint texture, int format, int width, int height, int fbo = -1);

    void init(int target_texture, GLuint texture, int format, int width, int height, int fbo = -1);

    void do_blur(int N = 7, float radius = 5.0);

private:
    GLuint _vao = 0;
    GLuint _fbo_x = 0;
    GLuint _fbo_y = 0;
    GLuint _texture = 0;
    GLuint _tmp_texture = 0;
    int _target_texture = 0;
    int _width = 0;
    int _height = 0;
    shader_program _program;

};


