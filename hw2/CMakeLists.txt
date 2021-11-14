cmake_minimum_required(VERSION 3.0)
project(hw2)

cmake_policy(SET CMP0072 NEW)
cmake_policy(SET CMP0074 NEW)

set(CMAKE_CXX_STANDARD 20)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake/modules")

find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(SDL2 REQUIRED)

if(APPLE)
	# brew version of glew doesn't provide GLEW_* variables
	get_target_property(GLEW_INCLUDE_DIRS GLEW::GLEW INTERFACE_INCLUDE_DIRECTORIES)
	get_target_property(GLEW_LIBRARIES GLEW::GLEW INTERFACE_LINK_LIBRARIES)
	get_target_property(GLEW_LIBRARY GLEW::GLEW LOCATION)
	list(APPEND GLEW_LIBRARIES "${GLEW_LIBRARY}")
endif()

if (NOT TARGET glm)
	add_subdirectory(glm)
endif()

set(TARGET_NAME "${PROJECT_NAME}")

convertIntoHeader(shaders/blur_vertex_shader.glsl shaders/blur_vertex_shader.h blur_vertex_shader_source)
convertIntoHeader(shaders/blur_fragment_shader.glsl shaders/blur_fragment_shader.h blur_fragment_shader_source)
convertIntoHeader(shaders/shadow_vertex_shader.glsl shaders/shadow_vertex_shader.h shadow_vertex_shader_source)
convertIntoHeader(shaders/shadow_fragment_shader.glsl shaders/shadow_fragment_shader.h shadow_fragment_shader_source)
convertIntoHeader(shaders/object_vertex_shader.glsl shaders/object_vertex_shader.h object_vertex_shader_source)
convertIntoHeader(shaders/object_fragment_shader.glsl shaders/object_fragment_shader.h object_fragment_shader_source)

add_executable(${TARGET_NAME}
	main.cpp
	include/utils.hpp include/utils.cpp
	include/shader_program.cpp include/shader_program.hpp
	include/object.cpp include/object.hpp
	include/blur_builder.cpp include/blur_builder.hpp
	shaders/blur_vertex_shader.h shaders/blur_fragment_shader.h
	include/scene_storage.cpp include/scene_storage.hpp
	include/shadow_map_builder.cpp include/shadow_map_builder.hpp
	shaders/shadow_vertex_shader.h shaders/shadow_fragment_shader.h
	include/direction_light_object.cpp include/direction_light_object.hpp
	include/point_light_object.cpp include/point_light_object.hpp
	shaders/object_vertex_shader.h shaders/object_fragment_shader.h
	include/wavefront_parser.hpp include/wavefront_parser.cpp
	lodepng/lodepng.h lodepng/lodepng.cpp
)

target_compile_definitions(${TARGET_NAME} PUBLIC
	"PROJECT_SOURCE_DIRECTORY=\"${CMAKE_CURRENT_SOURCE_DIR}\""
)
target_include_directories(${TARGET_NAME} PUBLIC
	"${SDL2_INCLUDE_DIRS}"
	"${GLEW_INCLUDE_DIRS}"
	"${OPENGL_INCLUDE_DIRS}"
	include
	shaders
	lodepng
)
target_link_libraries(${TARGET_NAME} PUBLIC
	glm
	"${GLEW_LIBRARIES}"
	"${SDL2_LIBRARIES}"
	"${OPENGL_LIBRARIES}"
)
