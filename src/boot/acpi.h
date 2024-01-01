#ifndef ACPI_H_
#define ACPI_H_

#include "boot/boot_info.h"

typedef struct {
  char Signature[4];
  uint32_t Length;
  uint8_t Revision;
  uint8_t Checksum;
  char OEMID[6];
  char OEMTableID[8];
  uint32_t OEMRevision;
  uint32_t CreatorID;
  uint32_t CreatorRevision;
} __attribute__((packed)) acpi_sdt_header_t;

typedef struct {
  acpi_sdt_header_t header;
  uint32_t apic_addr;
  uint32_t flags;
  uint8_t entries[2];     // variable size and different sized
} __attribute__((packed)) madt_t;

typedef struct {
    uint8_t type;
    uint8_t lenght;
    uint8_t processor_id;
    uint8_t apic_id;
    uint32_t flags;
} __attribute__((packed)) madt_local_apic_t;

typedef struct {
    uint8_t type;
    uint8_t lenght;
    uint8_t id;
    uint8_t reserved;
    uint32_t addr;
    uint32_t gsi_base;
} __attribute__((packed)) madt_io_apic_t;

typedef struct {
    uint8_t type;
    uint8_t lenght;
    uint8_t bus_src;    // 0 const 
    uint8_t irq_src;
    uint32_t global_system_int;
    uint16_t flags;
} __attribute__((packed)) madt_io_apic_override_t;

typedef struct {
  uint8_t AddressSpace;
  uint8_t BitWidth;
  uint8_t BitOffset;
  uint8_t AccessSize;
  uint64_t Address;
} __attribute__((packed)) GenericAddressStructure;

typedef struct {
    acpi_sdt_header_t header;
    uint32_t FirmwareCtrl;
    uint32_t Dsdt;
 
    // field used in ACPI 1.0; unused since ACPI 2.0
    uint8_t  Reserved;
 
    uint8_t  PreferredPowerManagementProfile;
    uint16_t SCI_Interrupt;
    uint32_t SMI_CommandPort;
    uint8_t  AcpiEnable;
    uint8_t  AcpiDisable;
    uint8_t  S4BIOS_REQ;
    uint8_t  PSTATE_Control;
    uint32_t PM1aEventBlock;
    uint32_t PM1bEventBlock;
    uint32_t PM1aControlBlock;
    uint32_t PM1bControlBlock;
    uint32_t PM2ControlBlock;
    uint32_t PMTimerBlock;
    uint32_t GPE0Block;
    uint32_t GPE1Block;
    uint8_t  PM1EventLength;
    uint8_t  PM1ControlLength;
    uint8_t  PM2ControlLength;
    uint8_t  PMTimerLength;
    uint8_t  GPE0Length;
    uint8_t  GPE1Length;
    uint8_t  GPE1Base;
    uint8_t  CStateControl;
    uint16_t WorstC2Latency;
    uint16_t WorstC3Latency;
    uint16_t FlushSize;
    uint16_t FlushStride;
    uint8_t  DutyOffset;
    uint8_t  DutyWidth;
    uint8_t  DayAlarm;
    uint8_t  MonthAlarm;
    uint8_t  Century;

    uint16_t BootArchitectureFlags; // reserved in ACPI 1.0, used since ACPI 2.0+
 
    uint8_t  Reserved2;
    uint32_t Flags;
 
    GenericAddressStructure ResetReg;
 
    uint8_t  ResetValue;
    uint8_t  Reserved3[3];
 
    // pointers (available since ACPI 2.0)
    uint64_t                X_FirmwareControl;
    uint64_t                X_Dsdt;
 
    GenericAddressStructure X_PM1aEventBlock;
    GenericAddressStructure X_PM1bEventBlock;
    GenericAddressStructure X_PM1aControlBlock;
    GenericAddressStructure X_PM1bControlBlock;
    GenericAddressStructure X_PM2ControlBlock;
    GenericAddressStructure X_PMTimerBlock;
    GenericAddressStructure X_GPE0Block;
    GenericAddressStructure X_GPE1Block;
} __attribute__((packed)) fadt_t;

typedef struct {
  acpi_sdt_header_t header;
  uint64_t otherSDTs[1]; // variable size
} __attribute__((packed)) xsdt_t;

typedef struct {
  acpi_sdt_header_t header;
  uint32_t otherSDTs[1]; // variable size
} __attribute__((packed)) rsdt_t;


#define MADT_ENTRY_APIC 0
#define MADT_ENTRY_IOAPIC 1
#define MADT_ENTRY_INT_SRC_OVERRIDE 2


void init_acpi(bloader_boot_info_t* boot_info);

madt_t* get_madt(void);
fadt_t* get_fadt(void);

#endif // ACPI_H_
