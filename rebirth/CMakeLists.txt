# rebirth engine
file(GLOB_RECURSE REBIRTH_SOURCE_FILES "src/*.cpp")
file(GLOB_RECURSE REBIRTH_HEADER_FILES "include/*.h")

add_library(rebirth-engine
    STATIC
    ${REBIRTH_SOURCE_FILES}
    ${REBIRTH_HEADER_FILES}
)

target_compile_options(rebirth-engine PRIVATE -Wall -fno-exceptions -fno-rtti)

target_include_directories(rebirth-engine PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")

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
    Jolt
)

add_executable(main
    src/main.cpp
)
target_link_libraries(main PUBLIC rebirth-engine)