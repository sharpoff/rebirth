# volk
add_subdirectory(volk)

# SDL3
set(SDL_TESTS OFF)
add_subdirectory(SDL)

# Vulkan Memory Allocator
add_subdirectory(VulkanMemoryAllocator)

# stb
add_library(stb INTERFACE)
target_include_directories(stb INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/stb")

# cgltf
add_library(cgltf INTERFACE)
target_include_directories(cgltf INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/cgltf")

# Dear ImGUI
file(GLOB_RECURSE IMGUI_SRC "${CMAKE_CURRENT_SOURCE_DIR}/imgui/*.cpp")
file(GLOB_RECURSE IMGUI_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/imgui/*.h")
add_library(imgui STATIC "${IMGUI_SRC}" "${IMGUI_HEADERS}")
target_link_libraries(imgui PUBLIC volk)
target_include_directories(imgui PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/imgui")

# Jolt Physics
add_subdirectory(JoltPhysics/Build)

# glm
add_subdirectory(glm)

# tracy client
option(TRACY_ENABLE " " ON)
option(TRACY_ON_DEMAND " " ON) # capture only when connected to profiler server
option(TRACY_ONLY_LOCALHOST " " ON)
add_subdirectory(tracy)

# EASTL
set(EASTL_EASTDC_VSNPRINTF ON)
add_subdirectory(EASTL)
