# rebirth engine
file(GLOB_RECURSE REBIRTH_SOURCE_FILES "rebirth/*.cpp")
file(GLOB_RECURSE REBIRTH_HEADER_FILES "rebirth/*.h")

add_library(rebirth-engine
    STATIC
    ${REBIRTH_SOURCE_FILES}
    ${REBIRTH_HEADER_FILES}
)

target_compile_options(rebirth-engine PRIVATE -Wall)

target_include_directories(rebirth-engine PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

find_package(Vulkan REQUIRED)
target_link_libraries(rebirth-engine
    PUBLIC

    Vulkan::Vulkan
    volk
    VulkanMemoryAllocator
    SDL3::SDL3
    stb
    glm
    cgltf
    imgui
)

# main executable
add_executable(main main.cpp)
target_link_libraries(main PUBLIC rebirth-engine)