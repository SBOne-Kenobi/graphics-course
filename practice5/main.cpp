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
#include "test_image.h"

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
uniform mat4 projection;

layout (location = 0) in vec3 in_position;
layout (location = 1) in vec2 in_tex_coords;

out vec4 color;
out vec2 tex_coords;

void main()
{
	gl_Position = projection * view * vec4(in_position, 1.0);
	color = vec4(1.0, 0.0, 1.0, 1.0);
    tex_coords = in_tex_coords;
}
)";

const char fragment_shader_source[] =
R"(#version 330 core

uniform sampler2D tex;
uniform sampler2D test_tex;

in vec4 color;
in vec2 tex_coords;

layout (location = 0) out vec4 out_color;

void main()
{
    float cat = 0.3;
	out_color = (1.0 - cat) * texture(tex, tex_coords) + cat * texture(test_tex, tex_coords);
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
    float tex_coords[2];
};

static vertex plane_vertices[]
{
	{{-10.f, 0.f, -10.f}, {0, 0}},
	{{-10.f, 0.f,  10.f}, {0, 1}},
	{{ 10.f, 0.f, -10.f}, {1, 0}},
	{{ 10.f, 0.f,  10.f}, {1, 1}},
};

static std::uint32_t plane_indices[]
{
	0, 1, 2, 2, 1, 3,
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

	SDL_Window * window = SDL_CreateWindow("Graphics course practice 5",
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
	GLint projection_location = glGetUniformLocation(program, "projection");
    GLint tex_location = glGetUniformLocation(program, "tex");
    GLint test_tex_location = glGetUniformLocation(program, "test_tex");

	GLuint vao, vbo, ebo;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(plane_vertices), plane_vertices, GL_STATIC_DRAW);

	glGenBuffers(1, &ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(plane_indices), plane_indices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)(0));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(vertex), (void *) sizeof(vec3));

    GLuint tex;
    glGenTextures(1, &tex);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    std::vector<uint8_t> image;

    int sz = 1024;

    for (int i = 0; i < sz; i++) {
        for (int j = 0; j < sz; j++) {
            uint8_t c = 0;
            if ((i + j) % 2 == 0)
                c = 255;
            for (int k = 0; k < 3; k++)
                image.push_back(c);
            image.push_back(255);
        }
    }

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, sz, sz, 0, GL_RGBA, GL_UNSIGNED_BYTE, image.data());
    glGenerateMipmap(GL_TEXTURE_2D);

    std::vector<uint8_t> red;
    for (int i = 0; i < sz / 2; i++) {
        for (int j = 0; j < sz / 2; j++) {
            red.push_back(255);
            red.push_back(0);
            red.push_back(0);
            red.push_back(255);
        }
    }
    glTexImage2D(GL_TEXTURE_2D, 1, GL_RGBA, sz / 2, sz / 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, red.data());
    std::vector<uint8_t> blue;
    for (int i = 0; i < sz / 4; i++) {
        for (int j = 0; j < sz / 4; j++) {
            blue.push_back(0);
            blue.push_back(0);
            blue.push_back(255);
            blue.push_back(255);
        }
    }
    glTexImage2D(GL_TEXTURE_2D, 2, GL_RGBA, sz / 4, sz / 4, 0, GL_RGBA, GL_UNSIGNED_BYTE, blue.data());
    std::vector<uint8_t> green;
    for (int i = 0; i < sz / 8; i++) {
        for (int j = 0; j < sz / 8; j++) {
            green.push_back(255);
            green.push_back(255);
            green.push_back(255);
            green.push_back(255);
        }
    }
    glTexImage2D(GL_TEXTURE_2D, 3, GL_RGBA, sz / 8, sz / 8, 0, GL_RGBA, GL_UNSIGNED_BYTE, green.data());


    GLuint test_tex;
    glGenTextures(1, &test_tex);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, test_tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, test_image_width, test_image_height, 0, GL_RGB, GL_UNSIGNED_BYTE, test_image);

    auto last_frame_start = std::chrono::high_resolution_clock::now();

	float time = 0.f;

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

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);

		float near = 0.1f;
		float far = 100.f;
		float top = near;
		float right = (top * width) / height;

		float view_angle = time;

		float view[16] =
		{
			1.f, 0.f, 0.f, 0.f,
			0.f, std::cos(view_angle), -std::sin(view_angle), 0.f,
			0.f, std::sin(view_angle), std::cos(view_angle), -15.f,
			0.f, 0.f, 0.f, 1.f,
		};

		float projection[16] =
		{
			near / right, 0.f, 0.f, 0.f,
			0.f, near / top, 0.f, 0.f,
			0.f, 0.f, - (far + near) / (far - near), - 2.f * far * near / (far - near),
			0.f, 0.f, -1.f, 0.f,
		};

		glUseProgram(program);
		glUniformMatrix4fv(view_location, 1, GL_TRUE, view);
		glUniformMatrix4fv(projection_location, 1, GL_TRUE, projection);
        glUniform1i(tex_location, 0);
        glUniform1i(test_tex_location, 1);

		glBindVertexArray(vao);
		glDrawElements(GL_TRIANGLES, std::size(plane_indices), GL_UNSIGNED_INT, nullptr);

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
