// CPUID driver for Onken OS — reads real CPU hardware identity
#include "cpuid.h"
#include "../kernel/string.h"

void cpuid(uint32_t leaf, uint32_t* eax, uint32_t* ebx, uint32_t* ecx, uint32_t* edx) {
    asm volatile (
        "cpuid"
        : "=a"(*eax), "=b"(*ebx), "=c"(*ecx), "=d"(*edx)
        : "a"(leaf)
        : 
    );
}

void cpuid_get_vendor(char* buf) {
    uint32_t eax, ebx, ecx, edx;
    cpuid(0, &eax, &ebx, &ecx, &edx);
    // Vendor is EBX + EDX + ECX
    memcpy(buf,     &ebx, 4);
    memcpy(buf + 4, &edx, 4);
    memcpy(buf + 8, &ecx, 4);
    buf[12] = '\0';
}

void cpuid_get_brand_string(char* buf) {
    uint32_t eax, ebx, ecx, edx;

    // Check if extended CPUID brand string is supported
    cpuid(0x80000000, &eax, &ebx, &ecx, &edx);
    if (eax < 0x80000004) {
        // Fall back to vendor string
        cpuid_get_vendor(buf);
        return;
    }

    // Read 3 leaves, each returns 16 bytes of brand string
    uint32_t* dst = (uint32_t*)buf;
    for (uint32_t leaf = 0x80000002; leaf <= 0x80000004; leaf++) {
        cpuid(leaf, &eax, &ebx, &ecx, &edx);
        *dst++ = eax;
        *dst++ = ebx;
        *dst++ = ecx;
        *dst++ = edx;
    }
    buf[48] = '\0';

    // Trim leading spaces (some CPUs pad with spaces)
    char* start = buf;
    while (*start == ' ') start++;
    if (start != buf) {
        int len = strlen(start);
        memcpy(buf, start, len + 1);
    }
}

void cpuid_get_info(char* buf, int buf_size) {
    uint32_t eax, ebx, ecx, edx;
    cpuid(1, &eax, &ebx, &ecx, &edx);

    uint32_t family   = (eax >> 8)  & 0xF;
    uint32_t model    = (eax >> 4)  & 0xF;
    uint32_t stepping = eax & 0xF;
    uint32_t ext_family = (eax >> 20) & 0xFF;
    uint32_t ext_model  = (eax >> 16) & 0xF;

    if (family == 0xF) family += ext_family;
    if (family == 0x6 || family == 0xF) model += (ext_model << 4);

    xsprintf(buf, "Family %d Model %d Stepping %d", family, model, stepping);
    (void)buf_size;
}
