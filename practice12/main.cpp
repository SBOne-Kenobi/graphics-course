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
#include <fstream>
#include <chrono>
#include <vector>
#include <map>

#define GLM_FORCE_SWIZZLE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/scalar_constants.hpp>
#include <glm/gtx/string_cast.hpp>

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

out vec3 position;

void main()
{
	position = in_position;
	gl_Position = projection * view * vec4(in_position, 1.0);
}
)";

const char fragment_shader_source[] =
R"(#version 330 core

uniform vec3 camera_position;
uniform vec3 light_dir;

in vec3 position;

layout (location = 0) out vec4 out_color;

void main()
{
	out_color = vec4(position * 0.5 + vec3(0.5), 1.0);
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

template<typename ... Shaders>
GLuint create_program(Shaders ... shaders) {
	GLuint result = glCreateProgram();
	(glAttachShader(result, shaders), ...);
	glLinkProgram(result);

	GLint status;
	glGetProgramiv(result, GL_LINK_STATUS, &status);
	if (status != GL_TRUE) {
		GLint info_log_length;
		glGetProgramiv(result, GL_INFO_LOG_LENGTH, &info_log_length);
		std::string info_log(info_log_length, '\0');
		glGetProgramInfoLog(result, info_log.size(), nullptr, info_log.data());
		throw std::runtime_error("Program linkage failed: " + info_log);
	}

	return result;
}

struct vertex
{
	glm::vec3 position;
};

static vertex cube_vertices[]
{
	{{-1.f, -1.f, -1.f}},
	{{ 1.f, -1.f, -1.f}},
	{{-1.f,  1.f, -1.f}},
	{{ 1.f,  1.f, -1.f}},
	{{-1.f, -1.f,  1.f}},
	{{ 1.f, -1.f,  1.f}},
	{{-1.f,  1.f,  1.f}},
	{{ 1.f,  1.f,  1.f}},
};

static std::uint32_t cube_indices[]
{
	// -Z
	0, 2, 1,
	1, 2, 3,
	// +Z
	4, 5, 6,
	6, 5, 7,
	// -Y
	0, 1, 4,
	4, 1, 5,
	// +Y
	2, 6, 3,
	3, 6, 7,
	// -X
	0, 4, 2,
	2, 4, 6,
	// +X
	1, 3, 5,
	5, 3, 7,
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

	SDL_Window * window = SDL_CreateWindow("Graphics course practice 12",
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

	GLuint view_location = glGetUniformLocation(program, "view");
	GLuint projection_location = glGetUniformLocation(program, "projection");
	GLuint camera_position_location = glGetUniformLocation(program, "camera_position");
	GLuint light_dir_location = glGetUniformLocation(program, "light_dir");

	GLuint vao, vbo, ebo;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertices), cube_vertices, GL_STATIC_DRAW);

	glGenBuffers(1, &ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cube_indices), cube_indices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

	auto last_frame_start = std::chrono::high_resolution_clock::now();

	float time = 0.f;

	glm::vec3 camera_position{0.f, 0.f, 3.f};

	float camera_rotation = 0.f;

	std::map<SDL_Keycode, bool> button_down;

	bool running = true;
	bool paused = false;
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
			if (event.key.keysym.sym == SDLK_SPACE)
				paused = !paused;
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

		if (!paused)
			time += dt;

		float camera_move_forward = 0.f;
		float camera_move_sideways = 0.f;

		if (button_down[SDLK_w])
			camera_move_forward -= 3.f * dt;
		if (button_down[SDLK_s])
			camera_move_forward += 3.f * dt;
		if (button_down[SDLK_a])
			camera_move_sideways -= 3.f * dt;
		if (button_down[SDLK_d])
			camera_move_sideways += 3.f * dt;

		camera_position += camera_move_forward * glm::vec3(-std::sin(camera_rotation), 0.f, std::cos(camera_rotation));
		camera_position += camera_move_sideways * glm::vec3(std::cos(camera_rotation), 0.f, std::sin(camera_rotation));

		if (button_down[SDLK_LEFT])
			camera_rotation -= 3.f * dt;
		if (button_down[SDLK_RIGHT])
			camera_rotation += 3.f * dt;

		if (button_down[SDLK_DOWN])
			camera_position.y -= 3.f * dt;
		if (button_down[SDLK_UP])
			camera_position.y += 3.f * dt;

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_FRONT);

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		float near = 0.1f;
		float far = 100.f;

		glm::mat4 view(1.f);
		view = glm::rotate(view, camera_rotation, {0.f, 1.f, 0.f});
		view = glm::translate(view, -camera_position);

		glm::mat4 projection = glm::perspective(glm::pi<float>() / 2.f, (1.f * width) / height, near, far);

		glm::vec3 light_dir = glm::normalize(glm::vec3(std::cos(time), 1.f, std::sin(time)));

		glUseProgram(program);
		glUniformMatrix4fv(view_location, 1, GL_FALSE, reinterpret_cast<float *>(&view));
		glUniformMatrix4fv(projection_location, 1, GL_FALSE, reinterpret_cast<float *>(&projection));
		glUniform3fv(camera_position_location, 1, reinterpret_cast<float *>(&camera_position));
		glUniform3fv(light_dir_location, 1, reinterpret_cast<float *>(&light_dir));

		glBindVertexArray(vao);
		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, nullptr);

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
