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
#include <fstream>
#include <sstream>
#include <random>

#define GLM_FORCE_SWIZZLE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/scalar_constants.hpp>
#include <glm/gtx/quaternion.hpp>
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

layout (location = 0) in vec3 in_position;
layout (location = 1) in float in_size;
layout (location = 2) in float in_angle;

out float size;
out float angle;

void main()
{
	gl_Position = vec4(in_position, 1.0);
    size = in_size;
    angle = in_angle;
}
)";

const char geometry_shader_source[] =
R"(#version 330 core

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 cam_position;

layout (points) in;
layout (triangle_strip, max_vertices = 4) out;

in float size[];
in float angle[];

out vec2 texcoord;

void main()
{
	vec3 center = gl_in[0].gl_Position.xyz;
    vec3 z_dir = normalize(cam_position - center);
    vec3 x_dir = normalize(cross(vec3(0.0, 0.0, 1.0), z_dir));
    vec3 y_dir = cross(z_dir, x_dir);
    x_dir = x_dir * cos(angle[0]) - y_dir * sin(angle[0]);
    y_dir = cross(z_dir, x_dir);
    mat3 tbn = mat3(x_dir, y_dir, z_dir);

    for (int i = -1; i <= 1; i += 2) {
        for (int j = -1; j <= 1; j += 2) {
            vec3 shift = tbn * vec3(i * size[0], -j * size[0], 0);
            gl_Position = projection * view * model * vec4(center + shift, 1.0);
            texcoord = 0.5 * vec2(i, -j) + 0.5;
            EmitVertex();
        }
    }
    EndPrimitive();
}

)";

const char fragment_shader_source[] =
R"(#version 330 core

uniform sampler2D tex;

layout (location = 0) out vec4 out_color;

in vec2 texcoord;

void main()
{
	out_color = vec4(1, 0, 0, texture(tex, texcoord).r);
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

template <typename ... Shaders>
GLuint create_program(Shaders ... shaders)
{
	GLuint result = glCreateProgram();
	(glAttachShader(result, shaders), ...);
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

struct particle
{
	glm::vec3 position;
	float size;
	float angle;
	glm::vec3 move_speed;
	float rotation_speed;
};

int main() try
{
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

	SDL_Window * window = SDL_CreateWindow("Graphics course practice 10",
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

	auto vertex_shader = create_shader(GL_VERTEX_SHADER, vertex_shader_source);
	auto geometry_shader = create_shader(GL_GEOMETRY_SHADER, geometry_shader_source);
	auto fragment_shader = create_shader(GL_FRAGMENT_SHADER, fragment_shader_source);
	auto program = create_program(vertex_shader, geometry_shader, fragment_shader);

	GLuint model_location = glGetUniformLocation(program, "model");
	GLuint view_location = glGetUniformLocation(program, "view");
	GLuint projection_location = glGetUniformLocation(program, "projection");
	GLuint cam_position_location = glGetUniformLocation(program, "cam_position");
	GLuint tex_location = glGetUniformLocation(program, "tex");

	std::default_random_engine rng;

	std::vector<particle> particles;
	particles.reserve(256);

	auto gen_particle = [&]() {
	    particle p{};
        p.position.x = std::uniform_real_distribution<float>{-1.f, 1.f}(rng);
        p.position.y = 0.f;
        p.position.z = std::uniform_real_distribution<float>{-1.f, 1.f}(rng);
        p.size = std::uniform_real_distribution<float>{0.1f, 0.3f}(rng);
        p.angle = std::uniform_real_distribution<float>{-10.f, 10.f}(rng);
        p.move_speed.x = std::uniform_real_distribution<float>{-1.f, 1.f}(rng);
        p.move_speed.y = std::uniform_real_distribution<float>{-1.f, 1.f}(rng);
        p.move_speed.z = std::uniform_real_distribution<float>{-1.f, 1.f}(rng);
        p.rotation_speed = std::uniform_real_distribution<float>{-10.f, 10.f}(rng);
        return p;
	};

	GLuint vao, vbo;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, particles.size() * sizeof(particle), particles.data(), GL_STATIC_COPY);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(particle), (void*)(0));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, sizeof(particle), (void*)(12));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(particle), (void*)(16));

    std::vector<unsigned char> texture(1024 * 1024);
    {
        std::ifstream in(PRACTICE_SOURCE_DIRECTORY "/particle.gray", std::ios_base::binary | std::ios_base::in);
        in.read((char*)texture.data(), texture.size());
    }
    GLuint tex;
    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, 1024, 1024, 0, GL_RED, GL_UNSIGNED_BYTE, texture.data());
    glGenerateMipmap(GL_TEXTURE_2D);

    glUniform1i(tex_location, 0);

    glPointSize(5.f);

    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);

	auto last_frame_start = std::chrono::high_resolution_clock::now();

	float time = 0.f;

	std::map<SDL_Keycode, bool> button_down;

	float view_angle = 0.f;
	float camera_distance = 3.f;
	float camera_height = 1.2f;

	float camera_rotation = 0.f;

	bool paused = false;

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
		time += dt;

		if (button_down[SDLK_UP])
			camera_distance -= 3.f * dt;
		if (button_down[SDLK_DOWN])
			camera_distance += 3.f * dt;

		if (button_down[SDLK_LEFT])
			camera_rotation -= 3.f * dt;
		if (button_down[SDLK_RIGHT])
			camera_rotation += 3.f * dt;

		if (!paused) {
            if (particles.size() < 256) {
                particles.push_back(gen_particle());
            }

            const float A = 3.f;
            const float C = 1.5f;
            const float D = 0.8f;

            for (auto &p : particles) {
                p.move_speed.y += dt * A;
                p.position += dt * p.move_speed;
                p.angle += dt * p.rotation_speed;
                p.move_speed *= std::exp(-C * dt);
                p.size *= std::exp(-D * dt);

                if (p.position.y >= 3.f) {
                    p = gen_particle();
                }
            }
        }

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);

		float near = 0.1f;
		float far = 100.f;

		glm::mat4 model(1.f);

		glm::mat4 view(1.f);
		view = glm::translate(view, {0.f, -camera_height, -camera_distance});
		view = glm::rotate(view, view_angle, {1.f, 0.f, 0.f});
		view = glm::rotate(view, camera_rotation, {0.f, 1.f, 0.f});

		glm::mat4 projection = glm::perspective(glm::pi<float>() / 2.f, (1.f * width) / height, near, far);

		glm::vec3 camera_position = (glm::inverse(view) * glm::vec4(0.f, 0.f, 0.f, 1.f)).xyz();

        std::sort(particles.begin(), particles.end(), [&](particle& a, particle& b) {
            return glm::length(a.position - camera_position) > glm::length(b.position - camera_position);
        });

        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, particles.size() * sizeof(particle), particles.data(), GL_STATIC_COPY);

		glUseProgram(program);

		glUniformMatrix4fv(model_location, 1, GL_FALSE, reinterpret_cast<float *>(&model));
		glUniformMatrix4fv(view_location, 1, GL_FALSE, reinterpret_cast<float *>(&view));
		glUniformMatrix4fv(projection_location, 1, GL_FALSE, reinterpret_cast<float *>(&projection));
		glUniform3fv(cam_position_location, 1, reinterpret_cast<float *>(&camera_position));

		glBindVertexArray(vao);
		glDrawArrays(GL_POINTS, 0, particles.size());

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
