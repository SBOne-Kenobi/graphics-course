#ifdef WIN32
#include <SDL.h>
#undef main
#else
#include <SDL2/SDL.h>
#endif

#include <GL/glew.h>

#include <string_view>
#include <stdexcept>
#include <iostream>
#include <chrono>
#include <vector>
#include <map>
#include <cmath>

#include "graph_fragment_shader.h"
#include "graph_vertex_shader.h"
#include "isoline_fragment_shader.h"
#include "isoline_vertex_shader.h"
#include "grid_fragment_shader.h"
#include "grid_vertex_shader.h"
#include "metaballs.hpp"
#include "utils.hpp"
#include "isoline.hpp"
#include "graph.hpp"

using std::cos, std::sin;

std::string to_string(std::string_view str) {
    return std::string(str.begin(), str.end());
}

void sdl2_fail(std::string_view message) {
    throw std::runtime_error(to_string(message) + SDL_GetError());
}

void glew_fail(std::string_view message, GLenum error) {
    throw std::runtime_error(to_string(message) + reinterpret_cast<const char *>(glewGetErrorString(error)));
}

int main() try {
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
        sdl2_fail("SDL_Init: ");

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    SDL_Window *window = SDL_CreateWindow("Graphics course practice 4",
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

    glClearColor(0.f, 0.f, 0.f, 0.f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_PRIMITIVE_RESTART);
    glLineWidth(2);
    glPointSize(3);

    // Graph settings

    shader_program graph_program(graph_vertex_shader_source, graph_fragment_shader_source);

    GLint graph_view_location = glGetUniformLocation(graph_program, "view");
    GLint graph_transform_location = glGetUniformLocation(graph_program, "transform");
    GLint graph_projection_location = glGetUniformLocation(graph_program, "projection");

    GLuint vbo_grid, vbo_value;
    glGenBuffers(1, &vbo_grid);
    glGenBuffers(1, &vbo_value);

    GLuint graph_vao;
    glGenVertexArrays(1, &graph_vao);
    glBindVertexArray(graph_vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo_grid);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(vec2), (void *) 0);

    glBindBuffer(GL_ARRAY_BUFFER, vbo_value);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, sizeof(float), (void *) 0);

    GLuint graph_ebo;
    glGenBuffers(1, &graph_ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, graph_ebo);

    float x0 = -10.121;
    float x1 = 10.41;
    float y0 = -2.0312;
    float y1 = 18.232;
    float k = 4.f / std::min(y1 - y0, x1 - x0);
    float z0 = -3.00123;
    float z1 = 3.00121;
    uint32_t grid_width = 50;
    uint32_t grid_height = 50;
    int balls_count = 30;

    auto func = metaballs_graph(x0, x1, y0, y1, balls_count);

    auto[grid, graph_order] = build_grid(grid_width, grid_height);

    glBindBuffer(GL_ARRAY_BUFFER, vbo_grid);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vec2) * grid.size(), grid.data(), GL_DYNAMIC_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * graph_order.size(), graph_order.data(), GL_DYNAMIC_DRAW);

    // Isoline settings

    shader_program isoline_program(isoline_vertex_shader_source, isoline_fragment_shader_source);

    GLint isoline_view_location = glGetUniformLocation(isoline_program, "view");
    GLint isoline_transform_location = glGetUniformLocation(isoline_program, "transform");
    GLint isoline_projection_location = glGetUniformLocation(isoline_program, "projection");

    uint32_t isoline_count = 40;
    bool isoline_on = true;

    GLuint vbo_isoline;
    glGenBuffers(1, &vbo_isoline);

    GLuint isoline_vao;
    glGenVertexArrays(1, &isoline_vao);
    glBindVertexArray(isoline_vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo_isoline);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (void *) 0);

    GLuint isoline_ebo;
    glGenBuffers(1, &isoline_ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, isoline_ebo);

    std::vector<float> C(isoline_count);
    for (int i = 0; i < isoline_count; i++) {
        C[i] = z0 + (z1 - z0) * ((float) i / float(isoline_count - 1));
    }

    // Grid settings

    GLuint grid_vao;
    glGenVertexArrays(1, &grid_vao);

    shader_program grid_program(grid_vertex_shader_source, grid_fragment_shader_source);

    bool grid_on = true;
    int grid_x_size = 15;
    int grid_y_size = 15;
    int grid_z_size = 10;

    GLint grid_view_location = glGetUniformLocation(grid_program, "view");
    GLint grid_transform_location = glGetUniformLocation(grid_program, "transform");
    GLint grid_projection_location = glGetUniformLocation(grid_program, "projection");
    GLint grid_z0_location = glGetUniformLocation(grid_program, "z0");
    GLint grid_z1_location = glGetUniformLocation(grid_program, "z1");
    GLint grid_x_size_location = glGetUniformLocation(grid_program, "x_size");
    GLint grid_y_size_location = glGetUniformLocation(grid_program, "y_size");
    GLint grid_z_size_location = glGetUniformLocation(grid_program, "z_size");

    // End

    auto last_frame_start = std::chrono::high_resolution_clock::now();

    float time = 0.f;

    float near = 0.001f;
    float far = 1000.f;
    float fov = 45.f * (float) std::asin(1) / 90.f;
    float right = near * std::tan(fov);
    float top = right * (float) height / (float) width;

    changed_value z{-2.7f, 2.f};
    changed_value angle_z{0.5f, 1.f};
    changed_value angle_x{0.9f, 1.f};
    bool pause = false;

    std::map<SDL_Keycode, bool> button_down;

    bool running = true;
    while (running) {
        int wheel = 0;
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
                            top = right * (float) height / (float) width;
                            break;
                    }
                    break;
                case SDL_KEYDOWN:
                    if (event.key.keysym.sym == SDLK_SPACE) {
                        pause = !pause;
                    }
                    if (event.key.keysym.sym == SDLK_LCTRL) {
                        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                    }
                    if (event.key.keysym.sym == SDLK_LALT) {
                        glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
                    }
                    if (event.key.keysym.sym == SDLK_1) {
                        isoline_on = !isoline_on;
                    }
                    if (event.key.keysym.sym == SDLK_2) {
                        grid_on = !grid_on;
                    }
                    button_down[event.key.keysym.sym] = true;
                    break;
                case SDL_KEYUP:
                    button_down[event.key.keysym.sym] = false;
                    if (event.key.keysym.sym == SDLK_LCTRL || event.key.keysym.sym == SDLK_LALT) {
                        if (button_down[SDLK_LCTRL]) {
                            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                        } else if (button_down[SDLK_LALT]) {
                            glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
                        } else {
                            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                        }
                    }
                    break;
                case SDL_MOUSEWHEEL:
                    wheel = event.wheel.y;
                    break;
                case SDL_MOUSEBUTTONDOWN:
                    if (event.button.button == SDL_BUTTON_LEFT) {
                        func.add_metaball();
                    } else if (event.button.button == SDL_BUTTON_RIGHT) {
                        func.remove_metaball();
                    }
                    break;
            }

        if (!running)
            break;

        auto now = std::chrono::high_resolution_clock::now();
        float dt = std::chrono::duration_cast<std::chrono::duration<float>>(now - last_frame_start).count();
        last_frame_start = now;
        if (!pause) {
            time += dt;
        }

        if (button_down[SDLK_RIGHT] | button_down[SDLK_d]) {
            angle_z.value += angle_z.velocity * dt;
        }
        if (button_down[SDLK_LEFT] | button_down[SDLK_a]) {
            angle_z.value -= angle_z.velocity * dt;
        }
        if (button_down[SDLK_UP]) {
            angle_x.value += angle_x.velocity * dt;
        }
        if (button_down[SDLK_DOWN]) {
            angle_x.value -= angle_x.velocity * dt;
        }
        if (button_down[SDLK_w]) {
            z.value += z.velocity * dt;
        }
        if (button_down[SDLK_s]) {
            z.value -= z.velocity * dt;
        }

        if (wheel != 0) {
            if (button_down[SDLK_LSHIFT]) {
                if (isoline_count + wheel >= 2) {
                    isoline_count += wheel;
                    C.resize(isoline_count);
                    for (int i = 0; i < isoline_count; i++) {
                        C[i] = z0 + (z1 - z0) * ((float) i / float(isoline_count - 1));
                    }
                }
            } else {
                if (std::min(grid_height, grid_width) >= 1 - wheel) {
                    grid_width += wheel;
                    grid_height += wheel;
                    std::tie(grid, graph_order) = build_grid(grid_width, grid_height);
                    glBindVertexArray(graph_vao);
                    glBindBuffer(GL_ARRAY_BUFFER, vbo_grid);
                    glBufferData(GL_ARRAY_BUFFER, sizeof(vec2) * grid.size(), grid.data(), GL_DYNAMIC_DRAW);
                    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * graph_order.size(), graph_order.data(),
                                 GL_DYNAMIC_DRAW);
                }
            }
        }

        float transform[] = {
            cos(angle_z.value), sin(angle_z.value), 0.f, 0.f,
            -sin(angle_z.value), cos(angle_z.value), 0.f, 0.f,
            0.f, 0.f, k, 0.f,
            0.f, 0.f, 0.f, 1.f,
        };

        float view[] = {
            1.f, 0.f, 0.f, 0.f,
            0.f, cos(angle_x.value), sin(angle_x.value), 0.f,
            0.f, -sin(angle_x.value), cos(angle_x.value), z.value,
            0.f, 0.f, 0.f, 1.f,
        };

        float projection[] = {
            near / right, 0.f, 0.f, 0.f,
            0.f, near / top, 0.f, 0.f,
            0.f, 0.f, -(far + near) / (far - near), -2.f * far * near / (far - near),
            0.f, 0.f, -1.f, 0.f,
        };

        auto values = compute_values(x0, x1, y0, y1, grid, time, func);
        glBindVertexArray(graph_vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_value);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * values.size(), values.data(), GL_STREAM_COPY);

        int isoline_points_count = 0;
        if (isoline_on) {
            auto [isoline_points, isoline_order] = build_isoline(grid_width, grid_height, grid, values, C);
            glBindVertexArray(isoline_vao);
            glBindBuffer(GL_ARRAY_BUFFER, vbo_isoline);
            glBufferData(GL_ARRAY_BUFFER, sizeof(vec3) * isoline_points.size(), isoline_points.data(), GL_STREAM_COPY);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, isoline_ebo);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * isoline_order.size(), isoline_order.data(),
                         GL_STREAM_COPY);
            isoline_points_count = (int) isoline_order.size();
        }

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Graph draw

        glBindVertexArray(grid_vao);
        glUseProgram(graph_program);
        glUniformMatrix4fv(graph_view_location, 1, GL_TRUE, view);
        glUniformMatrix4fv(graph_transform_location, 1, GL_TRUE, transform);
        glUniformMatrix4fv(graph_projection_location, 1, GL_TRUE, projection);

        glEnable(GL_PRIMITIVE_RESTART);
        glBindVertexArray(graph_vao);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, graph_ebo);
        glDrawElements(GL_TRIANGLE_STRIP, graph_order.size(), GL_UNSIGNED_INT, (void *) 0);

        // Isoline draw

        if (isoline_on) {
            glUseProgram(isoline_program);
            glUniformMatrix4fv(isoline_view_location, 1, GL_TRUE, view);
            glUniformMatrix4fv(isoline_transform_location, 1, GL_TRUE, transform);
            glUniformMatrix4fv(isoline_projection_location, 1, GL_TRUE, projection);

            glDisable(GL_PRIMITIVE_RESTART);
            glBindVertexArray(isoline_vao);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, isoline_ebo);
            glDrawElements(GL_LINES, isoline_points_count, GL_UNSIGNED_INT, (void *) 0);
        }

        // Grid draw

        if (grid_on) {
            glUseProgram(grid_program);
            glUniformMatrix4fv(grid_view_location, 1, GL_TRUE, view);
            glUniformMatrix4fv(grid_transform_location, 1, GL_TRUE, transform);
            glUniformMatrix4fv(grid_projection_location, 1, GL_TRUE, projection);
            glUniform1f(grid_z0_location, z0);
            glUniform1f(grid_z1_location, z1);
            glUniform1i(grid_x_size_location, grid_x_size);
            glUniform1i(grid_y_size_location, grid_y_size);
            glUniform1i(grid_z_size_location, grid_z_size);

            glDrawArrays(GL_LINES, 0, 2 * (grid_x_size + grid_y_size + 2 * grid_z_size));
        }

        SDL_GL_SwapWindow(window);
    }

    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
}
catch (std::exception const &e) {
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
}
