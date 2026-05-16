// stb_image.c wrapper for Kernel Mode
#include "memory_manager.h"

// Define custom memory allocators for stb_image in freestanding environment
#define STBI_MALLOC kmalloc
#define STBI_REALLOC krealloc
#define STBI_FREE kfree

// Kernel doesn't have standard I/O headers or functions
#define STBI_NO_STDIO
#define STBI_ASSERT(x) ((void)(x))

// Kernel doesn't have math.h, so disable float/hdr functions
#define STBI_NO_LINEAR
#define STBI_NO_HDR

// Limit maximum dimensions to prevent memory exhaustion and large stack frames
#define STBI_MAX_DIMENSIONS 4096

// Kernel doesn't have standard stdlib, disable SIMD and provide abs
#define STBI_NO_SIMD
#define STBI_NO_PSD
#define STBI_NO_PIC
#define STBI_NO_PNM
static inline int abs(int x) { return x < 0 ? -x : x; }

// Include the implementation
#define STB_IMAGE_IMPLEMENTATION
#include "userland/stb_image.h"
