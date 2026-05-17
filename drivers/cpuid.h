#pragma once
#include <stdint.h>

// Read CPUID leaf
void cpuid(uint32_t leaf, uint32_t* eax, uint32_t* ebx, uint32_t* ecx, uint32_t* edx);

// Fill buf with real CPU brand string (e.g. "Intel(R) Core(TM) i7-9700K")
// buf must be at least 49 bytes
void cpuid_get_brand_string(char* buf);

// Get CPU family/model/stepping string
void cpuid_get_info(char* buf, int buf_size);

// Get vendor string (GenuineIntel, AuthenticAMD, TCGTCGTCGTCG=QEMU)
void cpuid_get_vendor(char* buf);
