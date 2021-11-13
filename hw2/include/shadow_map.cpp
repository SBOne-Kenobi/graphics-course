#include <stdexcept>

#include "shadow_map.hpp"
#include "shadow_vertex_shader.h"
#include "shadow_fragment_shader.h"


void shadow_map::init(int target_texture, int resolution) {
    _resolution = resolution;
    _program.init(shadow_vertex_shader_source, shadow_fragment_shader_source);

    glGenTextures(1, &_shadow_map);
    glActiveTexture(GL_TEXTURE0 + target_texture);
    glBindTexture(GL_TEXTURE_2D, _shadow_map);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RG32F, _resolution, _resolution, 0, GL_RGBA, GL_FLOAT, nullptr);

    glGenRenderbuffers(1, &_rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, _rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, _resolution, _resolution);

    glGenFramebuffers(1, &_fbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _fbo);
    glFramebufferTexture(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, _shadow_map, 0);
    glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _fbo);

    if (glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        throw std::runtime_error("Incomplete framebuffer!");

    _blur.init(target_texture, _shadow_map, GL_RG32F, _resolution, _resolution, _fbo);
}

shadow_map::shadow_map(int target_texture, int resolution) {
    init(target_texture, resolution);
}

void shadow_map::draw(
    scene_storage& scene,
    const std::pair<glm::vec3, glm::vec3>& bbox,
    const direction_light_object& light_obj
) {
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _fbo);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0, _resolution, _resolution);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    glm::mat4 transform = light_obj.get_transform(bbox);

    _program.bind();
    glUniformMatrix4fv(_program["transform"], 1, GL_FALSE, reinterpret_cast<float *>(&transform));
    scene.draw_objects(_program);

    _blur.do_blur();
    glBindTexture(GL_TEXTURE_2D, _shadow_map);
    glGenerateMipmap(GL_TEXTURE_2D);

}

void shadow_map::draw(
    scene_storage &scene,
    const std::pair<glm::vec3, glm::vec3> &bbox,
    const point_light_object &light_obj,
    int cube_index
) {
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _fbo);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0, _resolution, _resolution);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    glm::mat4 transform = light_obj.get_transform(bbox, cube_index);

    _program.bind();
    glUniformMatrix4fv(_program["transform"], 1, GL_FALSE, reinterpret_cast<float *>(&transform));
    scene.draw_objects(_program);

    _blur.do_blur();
    glBindTexture(GL_TEXTURE_2D, _shadow_map);
    glGenerateMipmap(GL_TEXTURE_2D);
}
