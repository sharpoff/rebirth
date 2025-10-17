# rebirth engine
file(GLOB_RECURSE REBIRTH_SOURCE_FILES "*.cpp")
file(GLOB_RECURSE REBIRTH_HEADER_FILES "*.h")

add_library(rebirth-engine
    STATIC
    ${REBIRTH_SOURCE_FILES}
    ${REBIRTH_HEADER_FILES}
)

target_compile_options(rebirth-engine PRIVATE -Wall -fno-exceptions -fno-rtti)

target_include_directories(rebirth-engine PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

find_package(Vulkan REQUIRED)
target_link_libraries(rebirth-engine
    PUBLIC

    Vulkan::Vulkan
    volk
    VulkanMemoryAllocator
    SDL3::SDL3
    glm
    stb
    cgltf
    imgui
    Jolt
    Tracy::TracyClient
	EASTL
    nlohmann_json
)

add_executable(main
    main.cpp
)
target_link_libraries(main rebirth-engine)

# compile shaders
file(GLOB_RECURSE GLSL_SOURCE_FILES "../shaders/*.vert" "../shaders/*.frag" "../shaders/*.tesc" "../shaders/*.tese" "../shaders/*.comp")

set(GLSL_VALIDATOR "glslangValidator")
foreach(GLSL ${GLSL_SOURCE_FILES})
    get_filename_component(FILE_NAME ${GLSL} NAME)
    set(SPIRV "${PROJECT_BINARY_DIR}/shaders/${FILE_NAME}.spv")
    add_custom_command(
        OUTPUT ${SPIRV}
        COMMAND ${CMAKE_COMMAND} -E make_directory "${PROJECT_BINARY_DIR}/shaders/"
        COMMAND ${GLSL_VALIDATOR} -V ${GLSL} -o ${SPIRV}
        DEPENDS ${GLSL})
    list(APPEND SPIRV_BINARY_FILES ${SPIRV})
endforeach(GLSL)

add_custom_target(shaders DEPENDS ${SPIRV_BINARY_FILES})
add_dependencies(rebirth-engine shaders)