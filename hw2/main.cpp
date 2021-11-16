#ifdef WIN32

#include <SDL.h>

#undef main
#else
#include <SDL2/SDL.h>
#endif

#include <GL/glew.h>

#include <string_view>
#include <stdexcept>
#include <sstream>
#include <iostream>
#include <chrono>
#include <vector>
#include <map>

#define GLM_FORCE_SWIZZLE
#define GLM_ENABLE_EXPERIMENTAL

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/gtx/string_cast.hpp>

#include "object.hpp"
#include "shader_program.hpp"
#include "scene_storage.hpp"
#include "wavefront_parser.hpp"
#include "object_vertex_shader.h"
#include "object_fragment_shader.h"
#include "direction_light_object.hpp"
#include "shadow_map_builder.hpp"
#include "cubemap_builder.hpp"

std::string to_string(std::string_view str) {
    return std::string(str.begin(), str.end());
}

void sdl2_fail(std::string_view message) {
    throw std::runtime_error(to_string(message) + SDL_GetError());
}

void glew_fail(std::string_view message, GLenum error) {
    throw std::runtime_error(to_string(message) + reinterpret_cast<const char *>(glewGetErrorString(error)));
}

std::pair<glm::vec3, glm::vec3> get_bbox(scene_storage &scene) {
    glm::vec3 bbox_min(0.f);
    glm::vec3 bbox_max(0.f);

    scene.apply([&bbox_min, &bbox_max](object &obj) {
        for (auto v : obj.vertices) {

            auto pos = obj.model * glm::vec4(v.position, 1.f);

            bbox_min.x = std::min(bbox_min.x, pos.x);
            bbox_min.y = std::min(bbox_min.y, pos.y);
            bbox_min.z = std::min(bbox_min.z, pos.z);

            bbox_max.x = std::max(bbox_max.x, pos.x);
            bbox_max.y = std::max(bbox_max.y, pos.y);
            bbox_max.z = std::max(bbox_max.z, pos.z);
        }
    });

    return {bbox_min, bbox_max};
}

int main() try {
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
        sdl2_fail("SDL_Init: ");

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    SDL_Window *window = SDL_CreateWindow("hw2",
                                          SDL_WINDOWPOS_CENTERED,
                                          SDL_WINDOWPOS_CENTERED,
                                          800, 600,
                                          SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_MAXIMIZED);

    if (!window)
        sdl2_fail("SDL_CreateWindow: ");

    int width, height;
    SDL_GetWindowSize(window, &width, &height);

    SDL_GLContext gl_context = SDL_GL_CreateContext(window);
    if (!gl_context)
        sdl2_fail("SDL_GL_CreateContext: ");

    if (auto result = glewInit(); result != GLEW_NO_ERROR)
        glew_fail("glewInit: ", result);

    if (!GLEW_VERSION_3_3)
        throw std::runtime_error("OpenGL 3.3 is not supported");

    scene_storage main_scene;
    parse_scene(PROJECT_SOURCE_DIRECTORY "/scenes/sponza/sponza.obj", main_scene, true);

    main_scene.apply([](object &obj) {
        obj.model = glm::translate(obj.model, glm::vec3(0.f, -15.f, 0.f));
        obj.model = glm::scale(obj.model, glm::vec3(0.1f));
    });

    scene_storage helmet;
    parse_scene(PROJECT_SOURCE_DIRECTORY "/scenes/helmet/helmet_armet_2.obj", helmet, false);

    float helmet_scale = 0.5f;
    glm::mat4 helmet_model(1.f);
    helmet_model = glm::translate(helmet_model, {0.f, 0.f, -15.f});
    helmet_model = glm::rotate(helmet_model, glm::radians(-90.f), {1.f, 0.f, 0.f});
    helmet_model = glm::scale(helmet_model, glm::vec3(helmet_scale));

    glm::vec3 helmet_position = helmet_model * glm::vec4(0.f, 0.f, 0.f, 1.f);

    shader_program main_program(object_vertex_shader_source, object_fragment_shader_source);

    shadow_map_builder shadow(0, 6 * 512);
    cubemap_builder cubemap(256);

    helmet.apply([&helmet_model, &cubemap](object &obj) {
        obj.model = helmet_model;
        obj.with_env_map(cubemap.cubemap);
    });

    auto main_bbox = get_bbox(main_scene);

    float s0 = 5.f;
    float s1 = 15.f;
    float s2 = 10.f;

    direction_light_object direction_light(glm::vec3(0.f), {
        1.f * s0, 0.81f * s0, 0.28f * s0
    });

    std::vector<point_light_object> point_lights = {
        {{-111.5f, 3.0f,  -40.5f}, {0.88f * s1, 0.35f * s1, 0.13f * s1}, {0.1f, 0.f, 0.01f}},
        {{111.5f,  3.0f,  -40.5f}, {0.20f * s1, 0.95f * s1, 0.20f * s1}, {0.1f, 0.f, 0.01f}},
        {{-111.5f, 3.0f,  40.5f},  {0.58f * s1, 0.72f * s1, 0.90f * s1}, {0.1f, 0.f, 0.01f}},
        {{111.5f,  3.0f,  40.5f},  {0.55f * s1, 0.43f * s1, 0.31f * s1}, {0.1f, 0.f, 0.01f}},

        {{-61.5f,  -1.5f, 13.0f},  {0.88f * s2, 0.35f * s2, 0.13f * s2}, {0.5f, 0.f, 0.1f}},
        {{-61.5f,  -1.5f, -20.0f}, {0.88f * s2, 0.35f * s2, 0.13f * s2}, {0.5f, 0.f, 0.1f}},
        {{48.5f,   -1.5f, 13.0f},  {0.88f * s2, 0.35f * s2, 0.13f * s2}, {0.5f, 0.f, 0.1f}},
        {{48.5f,   -1.5f, -20.0f}, {0.88f * s2, 0.35f * s2, 0.13f * s2}, {0.5f, 0.f, 0.1f}},
    };

    auto last_frame_start = std::chrono::high_resolution_clock::now();

    float time = 0.f;

    std::map<SDL_Keycode, bool> button_down;

    float near = 0.01f;
    float far = 1000.f;

    glm::mat4 cam_pos(1.f);
    float angle = 0.f;

    glm::mat4 projection = glm::perspective(glm::pi<float>() / 2.f, (1.f * width) / height, near, far);

    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    bool running = true;
    bool helmet_follow = false;
    while (running) {
        float rot_ang = 0.f;
        bool in_window = false;
        float d_scale_helmet = 0.f;
        float d_angle = 0.f;
        for (SDL_Event event; SDL_PollEvent(&event);)
            switch (event.type) {
                case SDL_QUIT:
                    running = false;
                    break;
                case SDL_WINDOWEVENT:
                    switch (event.window.event) {
                        case SDL_WINDOWEVENT_RESIZED:
                            width = event.window.data1;
                            height = event.window.data2;
                            glViewport(0, 0, width, height);
                            projection = glm::perspective(glm::pi<float>() / 2.f, (1.f * width) / height, near, far);
                            break;
                    }
                    break;
                case SDL_KEYDOWN:
                    if (event.key.keysym.sym == SDLK_ESCAPE) {
                        running = false;
                    }
                    if (event.key.keysym.sym == SDLK_q) {
                        helmet_follow = !helmet_follow;
                    }
                    button_down[event.key.keysym.sym] = true;
                    break;
                case SDL_KEYUP:
                    button_down[event.key.keysym.sym] = false;
                    break;
                case SDL_MOUSEMOTION:
                    d_angle -= 0.003f * (float) (event.motion.yrel);
                    rot_ang -= 0.003f * (float) (event.motion.xrel);
                    break;
                case SDL_MOUSEWHEEL:
                    d_scale_helmet += 0.01f * (float) (event.wheel.y);
                    break;
                case SDL_MOUSEBUTTONDOWN:
                    if (event.button.button == SDL_BUTTON_LEFT) {
                        helmet_follow = !helmet_follow;
                    }
                    break;
            }

        if (!running)
            break;

        auto now = std::chrono::high_resolution_clock::now();
        float dt = std::chrono::duration_cast<std::chrono::duration<float>>(now - last_frame_start).count();
        last_frame_start = now;
        time += dt;
        std::cout << 1.f / dt << std::endl;

        if (!button_down[SDLK_LCTRL]) {
            if (SDL_GetWindowFlags(window) & SDL_WINDOW_MOUSE_FOCUS) {
                SDL_SetRelativeMouseMode(SDL_TRUE);
                SDL_ShowCursor(SDL_DISABLE);
                SDL_WarpMouseInWindow(window, width / 2, height / 2);
            }
            if (button_down[SDLK_UP]) {
                angle += 2.f * dt;
            }
            if (button_down[SDLK_DOWN]) {
                angle -= 2.f * dt;
            }

            if (button_down[SDLK_LEFT]) {
                rot_ang += 2.f * dt;
            }
            if (button_down[SDLK_RIGHT]) {
                rot_ang -= 2.f * dt;
            }
            cam_pos = glm::rotate(cam_pos, rot_ang, {0, 1, 0});

            glm::vec3 move_vector(0.f);

            if (button_down[SDLK_w]) {
                move_vector.z -= 50.f * dt;
            }
            if (button_down[SDLK_s]) {
                move_vector.z += 50.f * dt;
            }
            if (button_down[SDLK_a]) {
                move_vector.x -= 50.f * dt;
            }
            if (button_down[SDLK_d]) {
                move_vector.x += 50.f * dt;
            }
            if (button_down[SDLK_SPACE]) {
                move_vector.y += 50.f * dt;
            }
            if (button_down[SDLK_LSHIFT]) {
                move_vector.y -= 50.f * dt;
            }
            cam_pos = glm::translate(cam_pos, move_vector);
        } else {
            SDL_SetRelativeMouseMode(SDL_FALSE);
            SDL_ShowCursor(SDL_ENABLE);
            d_scale_helmet = 0.f;
            d_angle = 0.f;
        }

        angle += d_angle;
        glm::mat4 cam_pos_upd = glm::rotate(cam_pos, angle, {1, 0, 0});

        direction_light.direction = glm::normalize(glm::vec3(
            2.0f * std::cos(0.5f * time),
            3.f,
            1.1f * std::sin(0.5f * time)
        ));

        if (helmet_follow) {
            helmet_scale += d_scale_helmet;
            helmet_model = cam_pos_upd;
            helmet_model = glm::translate(helmet_model, {0.f, 0.f, -15.f});
            helmet_model = glm::rotate(helmet_model, glm::radians(-90.f), {1.f, 0.f, 0.f});
            helmet_model = glm::scale(helmet_model, glm::vec3(helmet_scale));
            helmet_position = helmet_model * glm::vec4(0.f, 0.f, 0.f, 1.f);

            helmet.apply([&helmet_model](object &obj) {
                obj.model = helmet_model;
            });
        }

        shadow.draw({&main_scene, &helmet}, main_bbox, direction_light);

        glClearColor(0.8f, 0.6f, 0.4f, 1.f);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, shadow.shadow_map);

        glm::mat4 transform = direction_light.get_transform(main_bbox);

        main_program.bind();

        glUniform3f(main_program["ambient"], 0.3f, 0.3f, 0.3f);

        glUniform3fv(main_program["light_direction"], 1, reinterpret_cast<float *>(&direction_light.direction));
        glUniform3fv(main_program["light_color"], 1, reinterpret_cast<float *>(&direction_light.light));

        glUniform1i(main_program["shadow_map"], 0);
        glUniformMatrix4fv(main_program["transform"], 1, GL_FALSE, reinterpret_cast<float *>(&transform));

        glUniform1i(main_program["point_light_number"], point_lights.size());

        for (int i = 0; i < point_lights.size(); i++) {
            std::stringstream name;
            name << "point_light_color[" << i << "]";
            glUniform3fv(main_program[name.str()], 1, reinterpret_cast<float *>(&point_lights[i].light));
            name.str("");
            name << "point_light_attenuation[" << i << "]";
            glUniform3fv(main_program[name.str()], 1, reinterpret_cast<float *>(&point_lights[i].attenuation));
            name.str("");
            name << "point_light_position[" << i << "]";
            glUniform3fv(main_program[name.str()], 1, reinterpret_cast<float *>(&point_lights[i].position));
        }

        cubemap.draw(helmet_position, {&main_scene}, main_program, near, far);

        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);

        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);

        glm::mat4 view = glm::inverse(cam_pos_upd);
        glUniformMatrix4fv(main_program["view"], 1, GL_FALSE, reinterpret_cast<float *>(&view));
        glUniformMatrix4fv(main_program["projection"], 1, GL_FALSE, reinterpret_cast<float *>(&projection));

        main_scene.draw_objects(main_program);
        helmet.draw_objects(main_program);

        SDL_GL_SwapWindow(window);
    }
    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
}
catch (std::exception const &e) {
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
}
