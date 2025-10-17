#include <new>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

namespace EA { namespace StdC {
    int Vsnprintf(char* buf, unsigned long bufsize, const char* fmt, va_list args) {
        return vsnprintf(buf, (size_t)bufsize, fmt, args);
    }
}}

void* operator new[](size_t size, const char* pName, int flags, unsigned debugFlags, const char* file, int line)
{
	return new uint8_t[size];
}

void* operator new[](size_t size, size_t alignment, size_t alignmentOffset, const char* pName, int flags, unsigned debugFlags, const char* file, int line)
{
	return new uint8_t[size];
}

// vma
#define VK_NO_PROTOTYPES
#define VMA_IMPLEMENTATION
#if 0
// debug allocations
#define VMA_DEBUG_LOG_FORMAT(format, ...)                                                                                                                                                              \
    do {                                                                                                                                                                                               \
        printf((format), __VA_ARGS__);                                                                                                                                                                 \
        printf("\n");                                                                                                                                                                                  \
    } while (false)
#endif
#include <vk_mem_alloc.h>

// volk
#define VOLK_IMPLEMENTATION
#include <volk.h>

// stb
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image.h>
#include <stb_image_write.h>

// cgltf
#define CGLTF_IMPLEMENTATION
#include <cgltf.h>

// jolt physics
#include <Jolt/Jolt.h>