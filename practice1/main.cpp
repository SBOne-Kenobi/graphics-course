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

GLuint create_shader(GLenum shader_type, const char * shader_source) {
    auto shader = glCreateShader(shader_type);
    glShaderSource(shader, 1, &shader_source, nullptr);
    glCompileShader(shader);

    GLint ret;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &ret);
    if (ret != GL_TRUE) {
        GLint log_len;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_len);
        std::string info_log(log_len, '\0');
        glGetShaderInfoLog(shader, log_len, nullptr, info_log.data());
        throw std::runtime_error("create_shader: compile error\n" + info_log);
    }

    return shader;
}

GLuint create_program(GLuint ver_shader, GLuint frag_shader) {
    auto prog = glCreateProgram();
    glAttachShader(prog, ver_shader);
    glAttachShader(prog, frag_shader);
    glLinkProgram(prog);

    GLint ret;
    glGetProgramiv(prog, GL_COMPILE_STATUS, &ret);
    if (ret != GL_TRUE) {
        GLint log_len;
        glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &log_len);
        std::string info_log(log_len, '\0');
        glGetProgramInfoLog(prog, log_len, nullptr, info_log.data());
        throw std::runtime_error("create_program: compile error\n" + info_log);
    }

    return prog;
}

int main() try
{
	if (SDL_Init(SDL_INIT_VIDEO) != 0)
		sdl2_fail("SDL_Init: ");

	SDL_Window * window = SDL_CreateWindow("Graphics course practice 1",
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		800, 600,
		SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_MAXIMIZED);

	if (!window)
		sdl2_fail("SDL_CreateWindow: ");

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	SDL_GLContext gl_context = SDL_GL_CreateContext(window);
	if (!gl_context)
		sdl2_fail("SDL_GL_CreateContext: ");

	if (auto result = glewInit(); result != GLEW_NO_ERROR)
		glew_fail("glewInit: ", result);

	if (!GLEW_VERSION_3_3)
		throw std::runtime_error("OpenGL 3.3 is not supported");

	glClearColor(0.8f, 0.8f, 1.f, 0.f);
    auto source_frag_shader_code = R"(
        #version 330 core

        layout (location = 0) out vec4 out_color;
        flat in vec3 color;

        void main()
        {
            out_color = vec4(color, 1.0);
        }
        )";
    auto frag_shader = create_shader(GL_FRAGMENT_SHADER, source_frag_shader_code);

    auto source_ver_shader_code = R"(
        #version 330 core

        const vec2 VERTICES[3] = vec2[3](
            vec2(0.0, 0.0),
            vec2(1.0, 0.0),
            vec2(0.0, 1.0));

        const vec3 COLORS[3] = vec3[3](
            vec3(1, 0, 0),
            vec3(0, 1, 0),
            vec3(0, 0, 1));

        flat out vec3 color;

        void main()
        {
            gl_Position = vec4(VERTICES[gl_VertexID], 0.0, 1.0);
            color = COLORS[gl_VertexID];
        }
        )";
    auto ver_shader = create_shader(GL_VERTEX_SHADER, source_ver_shader_code);
    auto program = create_program(ver_shader, frag_shader);

	bool running = true;
	while (running)
	{
		for (SDL_Event event; SDL_PollEvent(&event);) switch (event.type)
		{
		case SDL_QUIT:
			running = false;
			break;
		}

		if (!running)
			break;

		glClear(GL_COLOR_BUFFER_BIT);

        GLuint ver;
        glGenVertexArrays(3, &ver);
        glUseProgram(program);
        glBindVertexArray(ver);
        glProvokingVertex(GL_FIRST_VERTEX_CONVENTION);
        glDrawArrays(GL_TRIANGLES, 0, 3);

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
