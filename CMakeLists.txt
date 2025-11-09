cmake_minimum_required(VERSION 3.50)
project(Rebirth LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

add_subdirectory(external)
add_subdirectory(src)

SET(PROJECT_ROOT "${CMAKE_CURRENT_SOURCE_DIR}" CACHE INTERNAL "PROJECT_ROOT")
SET(SHADERS_DIR "${PROJECT_ROOT}/shaders" CACHE INTERNAL "SHADERS_DIR")
SET(ASSETS_DIR "${PROJECT_ROOT}/assets" CACHE INTERNAL "ASSETS_DIR")

# compile shaders
file(GLOB_RECURSE GLSL_SOURCE_FILES "${SHADERS_DIR}/*.vert" "${SHADERS_DIR}/*.frag" "${SHADERS_DIR}/*.tesc" "${SHADERS_DIR}/*.tese" "${SHADERS_DIR}/*.comp")

set(GLSL_VALIDATOR "glslangValidator")
foreach(GLSL ${GLSL_SOURCE_FILES})
    get_filename_component(FILE_NAME ${GLSL} NAME)
    set(SPIRV "${SHADERS_DIR}/bin/${FILE_NAME}.spv")
    add_custom_command(
        OUTPUT ${SPIRV}
        COMMAND ${CMAKE_COMMAND} -E make_directory "${SHADERS_DIR}/bin"
        COMMAND ${GLSL_VALIDATOR} -V ${GLSL} -o ${SPIRV}
        DEPENDS ${GLSL})
    list(APPEND SPIRV_BINARY_FILES ${SPIRV})
endforeach(GLSL)

add_custom_target(shaders DEPENDS ${SPIRV_BINARY_FILES})
add_dependencies(rebirth-engine shaders)