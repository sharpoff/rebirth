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

# eastl
set(EASTL_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/EASTL")
include_directories(${EASTL_ROOT_DIR}/include)
include_directories(${EASTL_ROOT_DIR}/test/packages/EAAssert/include)
include_directories(${EASTL_ROOT_DIR}/test/packages/EABase/include/Common)
include_directories(${EASTL_ROOT_DIR}/test/packages/EAMain/include)
include_directories(${EASTL_ROOT_DIR}/test/packages/EAStdC/include)
include_directories(${EASTL_ROOT_DIR}/test/packages/EATest/include)
include_directories(${EASTL_ROOT_DIR}/test/packages/EAThread/include)
set(EASTL_LIBRARY ${EASTL_ROOT_DIR}/build/EASTL.lib)