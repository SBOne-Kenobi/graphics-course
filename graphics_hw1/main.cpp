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

std::string to_string(std::string_view str)
{
    return std::string(str.begin(), str.end());
}

void sdl2_fail(std::string_view message)
{
    throw std::runtime_error(to_string(message) + SDL_GetError());
}

void glew_fail(std::string_view message, GLenum error)
{
    throw std::runtime_error(to_string(message) + reinterpret_cast<const char *>(glewGetErrorString(error)));
}

const char vertex_shader_source[] =
    R"(#version 330 core

uniform mat4 view;
uniform mat4 transform;

layout (location = 0) in vec2 in_position;
layout (location = 1) in float in_value;

out vec4 color;

void main()
{
	gl_Position = view * transform * vec4(in_position, in_value, 1.0);
	color = vec4(0.2 * -in_value, 0.7, 0.2, 1.0);
}
)";

const char fragment_shader_source[] =
    R"(#version 330 core

in vec4 color;

layout (location = 0) out vec4 out_color;

void main()
{
	out_color = color;
}
)";

GLuint create_shader(GLenum type, const char * source)
{
    GLuint result = glCreateShader(type);
    glShaderSource(result, 1, &source, nullptr);
    glCompileShader(result);
    GLint status;
    glGetShaderiv(result, GL_COMPILE_STATUS, &status);
    if (status != GL_TRUE)
    {
        GLint info_log_length;
        glGetShaderiv(result, GL_INFO_LOG_LENGTH, &info_log_length);
        std::string info_log(info_log_length, '\0');
        glGetShaderInfoLog(result, info_log.size(), nullptr, info_log.data());
        throw std::runtime_error("Shader compilation failed: " + info_log);
    }
    return result;
}

GLuint create_program(GLuint vertex_shader, GLuint fragment_shader)
{
    GLuint result = glCreateProgram();
    glAttachShader(result, vertex_shader);
    glAttachShader(result, fragment_shader);
    glLinkProgram(result);

    GLint status;
    glGetProgramiv(result, GL_LINK_STATUS, &status);
    if (status != GL_TRUE)
    {
        GLint info_log_length;
        glGetProgramiv(result, GL_INFO_LOG_LENGTH, &info_log_length);
        std::string info_log(info_log_length, '\0');
        glGetProgramInfoLog(result, info_log.size(), nullptr, info_log.data());
        throw std::runtime_error("Program linkage failed: " + info_log);
    }

    return result;
}

struct vec2 {
    float x;
    float y;
};

std::pair<std::vector<vec2>, std::vector<uint32_t>> build_grid(float x0, float y0, float x1, float y1,
                                                               uint32_t width, uint32_t height) {
    std::vector<vec2> grid;
    std::vector<uint32_t> order;

    grid.reserve((width + 1) * (height + 1));
    for (uint32_t i = 0; i <= width; i++) {
        float x = x0 + (x1 - x0) * (float) i / (float) width;
        for (uint32_t j = 0; j <= height; j++) {
            float y = y0 + (y1 - y0) * (float) j / (float) height;
            grid.push_back({x, y});
        }
    }

    uint32_t restart_index = grid.size();

    order.reserve(2 * (height + 1) * width);
    for (uint32_t i = 0; i < width; i++) {
        for (uint32_t j = 0; j <= height; j++) {
            order.push_back(i * (height + 1) + j);
            order.push_back((i + 1) * (height + 1) + j);
        }
        order.push_back(restart_index);
    }

    glPrimitiveRestartIndex(restart_index);

    return {grid, order};
}

template<typename F>
std::vector<float> compute_values(const std::vector<vec2>& grid, float time, F func) {
    std::vector<float> values(grid.size());
    for (uint32_t i = 0; i < grid.size(); i++) {
        values[i] = func(grid[i].x, grid[i].y, time);
    }
    return values;
}

std::vector<float> dot4(const std::vector<float>& a, const std::vector<float>& b) {
    std::vector<float> res(16);
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            for (int k = 0; k < 4; k++) {
                res[i * 4 + j] += a[i * 4 + k] * b[k * 4 + j];
            }
        }
    }
    return res;
}

int main() try
{
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

    SDL_Window * window = SDL_CreateWindow("Graphics course practice 4",
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

    glClearColor(0.8f, 0.8f, 1.f, 0.f);

    auto vertex_shader = create_shader(GL_VERTEX_SHADER, vertex_shader_source);
    auto fragment_shader = create_shader(GL_FRAGMENT_SHADER, fragment_shader_source);
    auto program = create_program(vertex_shader, fragment_shader);

    GLint view_location = glGetUniformLocation(program, "view");
    GLint transform_location = glGetUniformLocation(program, "transform");

    GLuint vbo_grid, vbo_value;
    glGenBuffers(1, &vbo_grid);
    glGenBuffers(1, &vbo_value);

    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_grid);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(vec2), (void *) 0);

    glBindBuffer(GL_ARRAY_BUFFER, vbo_value);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, sizeof(float), (void *) 0);

    GLuint ebo;
    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_PRIMITIVE_RESTART);

    float x0 = -1.f;
    float y0 = -1.f;
    float x1 = 1.f;
    float y1 = 1.f;
    uint32_t grid_width = 10;
    uint32_t grid_height = 10;

    auto func = [](float x, float y, float t) {
        return 0.f;
    };

    auto [grid, order] = build_grid(x0, y0, x1, y1, grid_width, grid_height);

    glBindBuffer(GL_ARRAY_BUFFER, vbo_grid);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vec2) * grid.size(), grid.data(), GL_STREAM_COPY);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * order.size(), order.data(), GL_STREAM_COPY);

    auto last_frame_start = std::chrono::high_resolution_clock::now();

    float time = 0.f;

    float near = 0.001f;
    float far = 1000.f;
    float fov = 45.f * (float) std::asin(1) / 90.f;
    float right = near * std::tan(fov);
    float top = right * (float) height / (float) width;

    std::vector<float> view =
        {
            near / right,   0.f,        0.f, 0.f,
            0.f,            near / top, 0.f, 0.f,
            0.f,            0.f,        -(far + near) / (far - near), -2.f * far * near / (far - near),
            0.f,            0.f,        -1.f, 0.f,
        };

    std::vector<float> rev_cam_pos =
        {
            1.f, 0.f, 0.f, 0.f,
            0.f, 1.f, 0.f, 0.f,
            0.f, 0.f, 1.f, 0.f,
            0.f, 0.f, 0.f, 1.f,
        };

    view = dot4(view, rev_cam_pos);

    std::map<SDL_Keycode, bool> button_down;

    bool running = true;
    while (running)
    {
        for (SDL_Event event; SDL_PollEvent(&event);) switch (event.type)
            {
                case SDL_QUIT:
                    running = false;
                    break;
                case SDL_WINDOWEVENT: switch (event.window.event)
                    {
                        case SDL_WINDOWEVENT_RESIZED:
                            width = event.window.data1;
                            height = event.window.data2;
                            glViewport(0, 0, width, height);
                            top = right * (float) height / (float) width;
                            break;
                    }
                    break;
                case SDL_KEYDOWN:
                    button_down[event.key.keysym.sym] = true;
                    break;
                case SDL_KEYUP:
                    button_down[event.key.keysym.sym] = false;
                    break;
            }

        if (!running)
            break;

        auto now = std::chrono::high_resolution_clock::now();
        float dt = std::chrono::duration_cast<std::chrono::duration<float>>(now - last_frame_start).count();
        last_frame_start = now;
        time += dt;

        auto values = compute_values(grid, time, func);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_value);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * values.size(), values.data(), GL_STREAM_COPY);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        float transform[] = {
            1.f, 0.f, 0.f, 0.f,
            0.f, 1.f, 0.f, 0.f,
            0.f, 0.f, 1.f, 0.f,
            0.f, 0.f, 0.f, 1.f,
        };

        glUseProgram(program);
        glUniformMatrix4fv(view_location, 1, GL_TRUE, view.data());
        glUniformMatrix4fv(transform_location, 1, GL_TRUE, transform);

        glDrawElements(GL_TRIANGLE_STRIP, order.size(), GL_UNSIGNED_INT, (void *) 0);

        SDL_GL_SwapWindow(window);
    }

    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
}
catch (std::exception const & e)
{
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
}
