#define STB_IMAGE_IMPLEMENTATION
#define STBI_NO_STDIO
#define STBI_NO_THREAD_LOCALS
#define STBI_ONLY_PNG
#define STBI_ONLY_JPEG
#define STBI_ONLY_BMP

// Provide standard library functions used by stb_image for a freestanding environment
#include "../kernel/string.h"
#include "../kernel/memory/memory.h"
#include "../kernel/kernel.h"

#define STBI_MALLOC(sz)           kmalloc(sz)
#define STBI_REALLOC(p,newsz)     krealloc(p, newsz)
#define STBI_FREE(p)              kfree(p)

// Provide missing math functions required by stb_image (stubs or simple implementations)
#define pow(x,y) (x) // Stub for gamma correction if needed
#define ldexp(x,exp) (x) // Stub for JPEG
#define STBI_NO_LINEAR
#define STBI_NO_HDR
#define STBI_NO_SIMD

#include "stb_image.h"
