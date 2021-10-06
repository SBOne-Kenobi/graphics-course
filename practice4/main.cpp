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

layout (location = 0) in vec3 in_position;
layout (location = 1) in vec4 in_color;

out vec4 color;

void main()
{
	gl_Position = view * transform * vec4(in_position, 1.0);
	color = in_color;
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

struct vec3
{
	float x;
	float y;
	float z;
};

struct vertex
{
	vec3 position;
	std::uint8_t color[4];
};

static vertex cube_vertices[]
{
	// -X
	{{-1.f, -1.f, -1.f}, {  0, 255, 255, 255}},
	{{-1.f, -1.f,  1.f}, {  0, 255, 255, 255}},
	{{-1.f,  1.f, -1.f}, {  0, 255, 255, 255}},
	{{-1.f,  1.f,  1.f}, {  0, 255, 255, 255}},
	// +X
	{{ 1.f, -1.f,  1.f}, {255,   0,   0, 255}},
	{{ 1.f, -1.f, -1.f}, {255,   0,   0, 255}},
	{{ 1.f,  1.f,  1.f}, {255,   0,   0, 255}},
	{{ 1.f,  1.f, -1.f}, {255,   0,   0, 255}},
	// -Y
	{{-1.f, -1.f, -1.f}, {255,   0, 255, 255}},
	{{ 1.f, -1.f, -1.f}, {255,   0, 255, 255}},
	{{-1.f, -1.f,  1.f}, {255,   0, 255, 255}},
	{{ 1.f, -1.f,  1.f}, {255,   0, 255, 255}},
	// +Y
	{{-1.f,  1.f,  1.f}, {  0, 255,   0, 255}},
	{{ 1.f,  1.f,  1.f}, {  0, 255,   0, 255}},
	{{-1.f,  1.f, -1.f}, {  0, 255,   0, 255}},
	{{ 1.f,  1.f, -1.f}, {  0, 255,   0, 255}},
	// -Z
	{{ 1.f, -1.f, -1.f}, {255, 255,   0, 255}},
	{{-1.f, -1.f, -1.f}, {255, 255,   0, 255}},
	{{ 1.f,  1.f, -1.f}, {255, 255,   0, 255}},
	{{-1.f,  1.f, -1.f}, {255, 255,   0, 255}},
	// +Z
	{{-1.f, -1.f,  1.f}, {  0,   0, 255, 255}},
	{{ 1.f, -1.f,  1.f}, {  0,   0, 255, 255}},
	{{-1.f,  1.f,  1.f}, {  0,   0, 255, 255}},
	{{ 1.f,  1.f,  1.f}, {  0,   0, 255, 255}},
};

static std::uint32_t cube_indices[]
{
	// -X
	0, 1, 2, 2, 1, 3,
	// +X
	4, 5, 6, 6, 5, 7,
	// -Y
	8, 9, 10, 10, 9, 11,
	// +Y
	12, 13, 14, 14, 13, 15,
	// -Z
	16, 17, 18, 18, 17, 19,
	// +Z
	20, 21, 22, 22, 21, 23,
};

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

    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), (void *) 0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(vertex), (void *) sizeof(vec3));

    GLuint ebo;
    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);

    glBufferData(GL_ARRAY_BUFFER, sizeof(vertex) * 24, cube_vertices, GL_STREAM_COPY);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * 36, cube_indices, GL_STREAM_COPY);

    glEnable(GL_DEPTH_TEST);

    auto last_frame_start = std::chrono::high_resolution_clock::now();

	float time = 0.f;

    float near = 0.001f;
    float far = 1000.f;
    float fov = 45.f * (float) std::asin(1) / 90.f;
    float right = near * std::tan(fov);
    float top = right * (float) height / (float) width;

    float angle = 0.f;
    float angle_speed = 1.f;

    float cube_x = 0.f;
    float x_speed = 2.f;
    float cube_y = 0.f;
    float y_speed = 2.f;

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

        angle += angle_speed * dt;
        if (button_down[SDLK_LEFT])
            cube_x -= x_speed * dt;
        if (button_down[SDLK_RIGHT])
            cube_x += x_speed * dt;
        if (button_down[SDLK_DOWN])
            cube_y -= y_speed * dt;
        if (button_down[SDLK_UP])
            cube_y += y_speed * dt;

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		float view[16] =
		{
			near / right,   0.f,        0.f, 0.f,
			0.f,            near / top, 0.f, 0.f,
			0.f,            0.f,        -(far + near) / (far - near), -2.f * far * near / (far - near),
			0.f,            0.f,        -1.f, 0.f,
		};

        std::vector<std::vector<float>> transform{
            {
                cos(angle), 0.f, sin(angle), cube_x - 1.2f,
                0.f, 1.f, 0.f, cube_y - 0.7f,
                -sin(angle), 0.f, cos(angle), -5.f,
                0.f, 0.f, 0.f, 1.f,
            },
            {
                cos(angle), sin(angle), 0.f, cube_x + 1.2f,
                -sin(angle), cos(angle), 0.f, cube_y - 0.7f,
                0.f, 0.f, 1.f, -5.f,
                0.f, 0.f, 0.f, 1.f,
            },
            {
                1.f, 0.f, 0.f, cube_x,
                0.f, cos(angle), sin(angle), cube_y + 1.f,
                0.f, -sin(angle), cos(angle), -5.f,
                0.f, 0.f, 0.f, 1.f,
            },
        };

        glUseProgram(program);
        glUniformMatrix4fv(view_location, 1, GL_TRUE, view);

        for (int i = 0; i < 3; i++) {

            for (int j = 0; j < 3 * 4; j++) {
                if (j % 4 != 3) {
                    transform[i][j] *= 0.5f;
                }
            }

            glUniformMatrix4fv(transform_location, 1, GL_TRUE, transform[i].data());
            glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, (void *) 0);
        }

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
