#include "bios.h"
#include "interrupt/pic/pic.h"

extern void bios_service_wrapped(uint32_t service_num, uint32_t args);
void* bios_service;

void init_bios_services(bloader_boot_info_t* boot_info) {
    bios_service = boot_info->call_bios_service;
}

void set_vbe_mode(vbe_mode_info_t* vbe_mode) {
    bios_service_wrapped(BIOS_SERV_SET_VBE, vbe_mode->mode);
    remap_pic();                        // for some reason only needed on real hardware
}

void readCHS(void) {
    bios_service_wrapped(BIOS_SERV_READ_CHS, 0x0);
    remap_pic();
}
