#ifndef BIOS_H_
#define BIOS_H_

#include "boot/boot_info.h"

void init_bios_services(bloader_boot_info_t* boot_info);

void printA(void);
void printB(void);

#endif // BIOS_H_
