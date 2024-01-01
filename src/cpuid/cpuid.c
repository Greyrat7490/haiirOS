#include "cpuid.h"
#include <cpuid.h>

bool cpuid_apic_available(void) {
    uint32_t eax, unused, edx = 0;
    __get_cpuid(1, &eax, &unused, &unused, &edx);
    return edx & CPUID_FEAT_EDX_APIC;
}
