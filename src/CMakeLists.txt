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

add_executable(main main.cpp)

target_link_libraries(main rebirth-engine)
set_target_properties(main PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")