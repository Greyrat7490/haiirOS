#ifndef BLOADER_BOOT_INFO_H_
#define BLOADER_BOOT_INFO_H_

#include <stdbool.h>

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;

typedef char int8_t;
typedef short int16_t;
typedef int int32_t;
typedef long int64_t;


typedef struct {
    char signature[4];
    uint16_t version;       // VBE version (high byte is major version, low byte is minor version)
    uint32_t oem;
    uint32_t capabilities;  // card capabilities (bitfield)
    uint32_t video_modes;   // pointer to supported video modes (offset + segment)
    uint16_t video_memory;  // video memory in 64KB blocks
    uint16_t software_rev;
    uint32_t vendor;
    uint32_t product_name;
    uint32_t product_rev;
    char reserved[222];
    char oem_data[256];
} __attribute__((packed)) vbe_info_t;

typedef struct {
    uint16_t mode;
    uint32_t framebuffer;
    uint16_t pitch;
    uint16_t width;
    uint16_t height;
    uint8_t bpp;
} __attribute__((packed)) vbe_mode_info_t;

typedef struct {
    uint16_t len;   // should be 0x3ff
    uint32_t base;  // should be 0x0
} __attribute__((packed)) BIOS_IDT_t;


#define MEMORY_RANGE_USABLE     1
#define MEMORY_RANGE_RESERVED   2
#define MEMORY_RANGE_ACPI_RECL  3   // reclaimable
#define MEMORY_RANGE_ACPI_NVS   4
#define MEMORY_RANGE_BAD        5   // or any other value

typedef struct {
    uint64_t base;
    uint64_t len;
    uint32_t type;
    uint32_t acpi3_ext;     // most of the time unused
} __attribute__((packed)) memory_map_entry_t;

typedef struct {
    memory_map_entry_t* entries;
    uint16_t count;
} __attribute__((packed)) memory_map_t;


typedef struct {
    memory_map_t memory_map;
    uint16_t lower_memory_KiB;
    uint16_t upper_memory_64KiB;
    uint64_t PML4_addr;
    uint64_t gdt32_addr;
    uint64_t gdt64_addr;
    BIOS_IDT_t* biosIDT_addr;
    vbe_info_t* vbe;
    vbe_mode_info_t* vbe_mode;
    uint64_t kernel_addr;
    uint32_t kernel_size;
} __attribute__((packed)) bloader_boot_info_t;

#endif // BLOADER_BOOT_INFO_H_
