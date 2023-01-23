#ifndef BIOS_H_
#define BIOS_H_

#include "boot/boot_info.h"

void init_bios_services(bloader_boot_info_t* boot_info);

void set_vbe_mode(vbe_mode_info_t* vbe_mode);
void readCHS(void);

#endif // BIOS_H_
