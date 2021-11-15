#include "cubemap_builder.hpp"
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <stdexcept>

glm::mat4 get_camera_view(glm::vec3 position, int index) {
    static const glm::vec3 targets[6] = {
        glm::vec3(+1.0f, 0.0f, 0.0f),
        glm::vec3(-1.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, +1.0f, 0.0f),
        glm::vec3(0.0f, -1.0f, 0.0f),
        glm::vec3(0.0f, 0.0f, +1.0f),
        glm::vec3(0.0f, 0.0f, -1.0f)
    };

    static const glm::vec3 ups[6] = {
        glm::vec3(0.0f, 1.0f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f),
        glm::vec3(0.0f, 0.0f, -1.0f),
        glm::vec3(0.0f, 0.0f, 1.0f),
        glm::vec3(0.0f, 1.0f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f)
    };

    glm::mat4 view = glm::lookAt(position, position + targets[index], ups[index]);
    view[0][2] *= -1.0f;
    view[1][2] *= -1.0f;
    view[2][2] *= -1.0f;
    view[3][2] *= -1.0f;
    return view;
}


void cubemap_builder::draw(
    glm::vec3 position,
    const std::vector<scene_storage*>& scenes,
    shader_program &program,
    float near, float far
) const {
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _fbo);
    glViewport(0, 0, _resolution, _resolution);

    glm::mat4 projection = glm::perspective(glm::radians(90.f), 1.f, near, far);
    glUniformMatrix4fv(program["projection"], 1, GL_FALSE, reinterpret_cast<float *>(&projection));

    for (int i = 0; i < 6; i++) {
        glm::mat4 view = get_camera_view(position, i);
        glUniformMatrix4fv(program["view"], 1, GL_FALSE, reinterpret_cast<float *>(&view));
        glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, cubemap, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);

        glEnable(GL_CULL_FACE);
        glCullFace(GL_FRONT);

        for (auto scene : scenes) {
            scene->draw_objects(program, true, true);
        }
    }
}

void cubemap_builder::init(int resolution) {
    _resolution = resolution;

    glGenTextures(1, &cubemap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    for (int i = 0; i < 6; i++) {
        glTexImage2D(
            GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
            0, GL_RGB, _resolution, _resolution, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr
        );
    }

    glGenRenderbuffers(1, &_rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, _rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, _resolution, _resolution);

    glGenFramebuffers(1, &_fbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _fbo);
    glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _rbo);

    if (glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        throw std::runtime_error("Incomplete framebuffer 21 21!");
}

cubemap_builder::cubemap_builder(int resolution) {
    init(resolution);
}
