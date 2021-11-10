cmake_minimum_required(VERSION 3.0)
project(graphics-course)

set(GLEW_ROOT "D:/glew")
set(SDL2_ROOT "D:/sdl2")

set(GLEW_DLL "${GLEW_ROOT}/bin/Release/x64/glew32.dll")
set(SDL2_DLL "${SDL2_ROOT}/bin/SDL2.dll")

set(SUBDIRS practice1 practice2 practice3 practice4 practice5 hw1 practice6 practice7 practice8 hw2 practice9)

add_subdirectory(libs)

foreach(subdir ${SUBDIRS})
    add_subdirectory(${subdir})

    configure_file(${GLEW_DLL} ${CMAKE_CURRENT_BINARY_DIR}/${subdir} COPYONLY)
    configure_file(${SDL2_DLL} ${CMAKE_CURRENT_BINARY_DIR}/${subdir} COPYONLY)
endforeach()
