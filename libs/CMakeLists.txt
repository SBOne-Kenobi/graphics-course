cmake_minimum_required(VERSION 3.0)
project(libs)

add_executable(hexdumparray hexdumparray.cpp)

function(convertIntoHeader sourceFile headerFile arrayName)
    add_custom_command(
            OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/${headerFile}

            COMMAND hexdumparray ${CMAKE_CURRENT_SOURCE_DIR}/${sourceFile} ${CMAKE_CURRENT_SOURCE_DIR}/${headerFile} ${arrayName}

            DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${sourceFile} hexdumparray
    )
endfunction()