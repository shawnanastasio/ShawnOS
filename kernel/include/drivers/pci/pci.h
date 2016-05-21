#pragma once

#define PCI_CONFIG_ADDR     0xCF8
#define PCI_CONFIG_DATA     0xCFC

typedef struct {
    uint32_t vendor;
    uint32_t device;
    uint32_t func;
} pci_device;

uint16_t pci_config_read_word(uint16_t bus, uint16_t slot, uint16_t func, uint8_t offset);
uint16_t pci_get_vendor_id(uint16_t bus, uint16_t slot, uint16_t func);
uint16_t pci_get_device_id(uint16_t bus, uint16_t slot, uint16_t func);
uint16_t pci_get_device_class_id(uint16_t bus, uint16_t slot, uint16_t func);
uint16_t pci_get_device_subclass_id(uint16_t bus, uint16_t slot, uint16_t func);
void pci_probe();
void pci_init();

char* PCI_CLASS_IDS[] =
{
    "no class specification",
    "Mass Storage Controller",
    "Network Controller",
    "Display Controller",
    "Multimedia Device",
    "Memory Controller",
    "Bridge Device",
    "Simple Communication Controller",
    "Base System Peripheral",
    "Input Device",
    "Docking Station",
    "Processor",
    "Serial Bus Controller",
    "Wireless Controller",
    "Intelligent I/O Controller",
    "Satellite Communication Controller",
    "Encryption/Decryption Controller",
    "Data Acquisition and Signal Processing Controller"
};
