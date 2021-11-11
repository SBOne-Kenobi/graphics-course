#include <stdexcept>
#include "blur.hpp"
#include "blur_vertex_shader.h"
#include "blur_fragment_shader.h"

blur::blur(int target_texture, GLuint texture, int format, int width, int height, int fbo) :
    program(blur_vertex_shader_source, blur_fragment_shader_source),
    width(width), height(height), texture(texture), target_texture(target_texture) {
    glGenVertexArrays(1, &vao);

    glActiveTexture(GL_TEXTURE0 + target_texture);

    glGenTextures(1, &tmp_texture);
    glBindTexture(GL_TEXTURE_2D, tmp_texture);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, GL_RGBA, GL_FLOAT, nullptr);

    glGenFramebuffers(1, &fbo_x);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo_x);
    glFramebufferTexture(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, tmp_texture, 0);
    if (glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        throw std::runtime_error("Incomplete framebuffer!");

    if (fbo == -1) {
        glGenFramebuffers(1, &fbo_y);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo_y);
        glFramebufferTexture(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texture, 0);
        if (glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            throw std::runtime_error("Incomplete framebuffer!");
    } else {
        fbo_y = fbo;
    }
}

void blur::do_blur(int N, float radius) {
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo_x);
    glViewport(0, 0, width, height);
    glActiveTexture(GL_TEXTURE0 + target_texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    program.bind();
    glUniform1i(program["target"], target_texture);
    glUniform1i(program["N"], N);
    glUniform1f(program["radius"], radius);
    glUniform1i(program["mode"], 0);
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo_y);
    glActiveTexture(GL_TEXTURE0 + target_texture);
    glBindTexture(GL_TEXTURE_2D, tmp_texture);
    glUniform1i(program["mode"], 1);

    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}
