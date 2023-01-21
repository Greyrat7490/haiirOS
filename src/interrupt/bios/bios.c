#include "bios.h"

extern void bios_service_wrapped(uint32_t service_num);
void* bios_service;

void init_bios_services(bloader_boot_info_t* boot_info) {
    bios_service = boot_info->call_bios_service;
}

void printA(void) {
    bios_service_wrapped(BIOS_SERV_PRINTA);
}

void printB(void) {
    bios_service_wrapped(BIOS_SERV_PRINTB);
}
