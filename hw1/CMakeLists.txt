cmake_minimum_required(VERSION 3.0)
project(hw1)

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

set(TARGET_NAME "${PROJECT_NAME}")

convertIntoHeader(shaders/graph_fragment_shader.glsl shaders/graph_fragment_shader.h graph_fragment_shader_source)
convertIntoHeader(shaders/graph_vertex_shader.glsl shaders/graph_vertex_shader.h graph_vertex_shader_source)
convertIntoHeader(shaders/isoline_vertex_shader.glsl shaders/isoline_vertex_shader.h isoline_vertex_shader_source)
convertIntoHeader(shaders/isoline_fragment_shader.glsl shaders/isoline_fragment_shader.h isoline_fragment_shader_source)
convertIntoHeader(shaders/grid_fragment_shader.glsl shaders/grid_fragment_shader.h grid_fragment_shader_source)
convertIntoHeader(shaders/grid_vertex_shader.glsl shaders/grid_vertex_shader.h grid_vertex_shader_source)

add_executable(${TARGET_NAME} main.cpp
	shaders/graph_fragment_shader.h
	shaders/graph_vertex_shader.h
	shaders/isoline_vertex_shader.h
	shaders/isoline_fragment_shader.h
	shaders/grid_fragment_shader.h
	shaders/grid_vertex_shader.h
	include/metaballs.hpp include/utils.hpp include/isoline.hpp include/graph.hpp)

target_include_directories(${TARGET_NAME} PUBLIC
	"${SDL2_INCLUDE_DIRS}"
	"${GLEW_INCLUDE_DIRS}"
	"${OPENGL_INCLUDE_DIRS}"
	shaders
	include
)
target_link_libraries(${TARGET_NAME} PUBLIC
	"${GLEW_LIBRARIES}"
	"${SDL2_LIBRARIES}"
	"${OPENGL_LIBRARIES}"
)
