#include <stdexcept>
#include "blur_builder.hpp"
#include "blur_vertex_shader.h"
#include "blur_fragment_shader.h"

void blur_builder::init(int target_texture, GLuint texture, int format, int width, int height, int fbo) {
    _width = width;
    _height = height;
    _texture = texture;
    _target_texture = target_texture;
    _program = shader_program(blur_vertex_shader_source, blur_fragment_shader_source);

    glGenVertexArrays(1, &_vao);

    glActiveTexture(GL_TEXTURE0 + target_texture);

    glGenTextures(1, &_tmp_texture);
    glBindTexture(GL_TEXTURE_2D, _tmp_texture);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, GL_RGBA, GL_FLOAT, nullptr);

    glGenFramebuffers(1, &_fbo_x);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _fbo_x);
    glFramebufferTexture(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, _tmp_texture, 0);
    if (glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        throw std::runtime_error("Incomplete framebuffer!");

    if (fbo == -1) {
        glGenFramebuffers(1, &_fbo_y);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _fbo_y);
        glFramebufferTexture(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texture, 0);
        if (glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            throw std::runtime_error("Incomplete framebuffer!");
    } else {
        _fbo_y = fbo;
    }
}

void blur_builder::do_blur(int N, float radius) {
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _fbo_x);
    glViewport(0, 0, _width, _height);
    glActiveTexture(GL_TEXTURE0 + _target_texture);
    glBindTexture(GL_TEXTURE_2D, _texture);

    _program.bind();
    glUniform1i(_program["target"], _target_texture);
    glUniform1i(_program["N"], N);
    glUniform1f(_program["radius"], radius);
    glUniform1i(_program["mode"], 0);
    glBindVertexArray(_vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _fbo_y);
    glActiveTexture(GL_TEXTURE0 + _target_texture);
    glBindTexture(GL_TEXTURE_2D, _tmp_texture);
    glUniform1i(_program["mode"], 1);

    glBindVertexArray(_vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

blur_builder::blur_builder(int target_texture, GLuint texture, int format, int width, int height, int fbo) {
    init(target_texture, texture, format, width, height, fbo);
}
