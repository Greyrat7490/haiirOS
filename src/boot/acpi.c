#include "acpi.h"
#include "boot/boot_info.h"
#include "io/io.h"
#include "memory/paging.h"
#include "types.h"

static bool xsdp_detected = false;

static xsdp_t* xsdp = 0x0;
static madt_t* madt = 0x0;
static fadt_t* fadt = 0x0;

madt_t* get_madt(void) { return madt; }
fadt_t* get_fadt(void) { return fadt; }

static void map_sdt(acpi_sdt_header_t* sdt_header) {
    map_frame(to_page((uint64_t)sdt_header), to_frame((uint64_t)sdt_header), Present | Writeable);
    for (uint32_t i = 1; i < (sdt_header->Length) / PAGE_SIZE + 1; i++) {
        map_frame(to_page((uint64_t)sdt_header + i*PAGE_SIZE), to_frame((uint64_t)sdt_header + i*PAGE_SIZE), Present | Writeable);
    }
}

static bool signature_cmp(char signature[4], char target[4]) {
    return signature[0] == target[0] &&
        signature[1] == target[1] &&
        signature[2] == target[2] &&
        signature[3] == target[3];
}

void init_with_rsdp(rsdt_t* root_rsdt) {
    map_sdt((acpi_sdt_header_t*) root_rsdt);

    uint32_t entries = (root_rsdt->header.Length - sizeof(acpi_sdt_header_t)) / 4;
 
    for (uint32_t i = 0; i < entries; i++) {
        acpi_sdt_header_t* header = (acpi_sdt_header_t*)(uint64_t) root_rsdt->otherSDTs[i];
        map_sdt((acpi_sdt_header_t*) header);

        if (signature_cmp(header->Signature, "APIC")) {
            madt = (madt_t*) header;
            kprintln("header: APIC");
        }

        if (signature_cmp(header->Signature, "FACP")) {
            fadt = (fadt_t*) header;
            kprintln("header: FACP");
        }
    }
}

void init_with_xsdp(xsdt_t* root_xsdt) {
    map_sdt((acpi_sdt_header_t*) root_xsdt);

    uint32_t entries = (root_xsdt->header.Length - sizeof(root_xsdt->header)) / 8;

    for (uint32_t i = 0; i < entries; i++) {
        acpi_sdt_header_t* header = (acpi_sdt_header_t*) root_xsdt->otherSDTs[i];
        map_sdt((acpi_sdt_header_t*) header);

        if (signature_cmp(header->Signature, "APIC")) {
            madt = (madt_t*) header;
            kprintln("header: APIC");
        }

        if (signature_cmp(header->Signature, "FACP")) {
            fadt = (fadt_t*) header;
            kprintln("header: FACP");
        }
    }
}

void init_acpi(bloader_boot_info_t* boot_info) {
    kprintln("%x", boot_info->rsdp);
    xsdp = (xsdp_t*) boot_info->rsdp;
    if (xsdp->Revision != 0) {
        xsdp_detected = true;
        init_with_xsdp((xsdt_t*)xsdp->XsdtAddress);
    } else {
        init_with_rsdp((rsdt_t*)(uint64_t)xsdp->RsdtAddress);
    }
}
