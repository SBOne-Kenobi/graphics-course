#pragma once


inline GLuint create_shader(GLenum type, const char *source) {
    GLuint result = glCreateShader(type);
    glShaderSource(result, 1, &source, nullptr);
    glCompileShader(result);
    GLint status;
    glGetShaderiv(result, GL_COMPILE_STATUS, &status);
    if (status != GL_TRUE) {
        GLint info_log_length;
        glGetShaderiv(result, GL_INFO_LOG_LENGTH, &info_log_length);
        std::string info_log(info_log_length, '\0');
        glGetShaderInfoLog(result, info_log.size(), nullptr, info_log.data());
        throw std::runtime_error("Shader compilation failed: " + info_log);
    }
    return result;
}

inline GLuint create_program(GLuint vertex_shader, GLuint fragment_shader) {
    GLuint result = glCreateProgram();
    glAttachShader(result, vertex_shader);
    glAttachShader(result, fragment_shader);
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

struct vec2 {
    float x;
    float y;

    inline vec2 operator+(const vec2 &other) const {
        return {x + other.x, y + other.y};
    }

    inline vec2 operator-(const vec2 &other) const {
        return {x - other.x, y - other.y};
    }

    inline vec2 operator*(float t) const {
        return {x * t, y * t};
    }
};

struct vec3 {
    float x;
    float y;
    float z;

    inline vec3 operator+(const vec3 &other) const {
        return {x + other.x, y + other.y, z + other.z};
    }

    inline vec3 operator-(const vec3 &other) const {
        return {x - other.x, y - other.y, z - other.z};
    }

    inline vec3 operator*(float t) const {
        return {x * t, y * t, z * t};
    }
};

struct changed_value {
    float value;
    float velocity;
};

class shader_program {
public:

    inline shader_program(const char *vertex_source, const char *fragment_source) {
        vertex_shader = create_shader(GL_VERTEX_SHADER, vertex_source);
        fragment_shader = create_shader(GL_FRAGMENT_SHADER, fragment_source);
        program = create_program(vertex_shader, fragment_shader);
    }

    inline operator GLuint() const {
        return program;
    }

private:

    GLuint vertex_shader;
    GLuint fragment_shader;
    GLuint program;

};
