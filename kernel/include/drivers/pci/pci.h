#pragma once

#define PCI_CONFIG_ADDR     0xCF8
#define PCI_CONFIG_DATA     0xCFC

uint16_t pci_config_read_word(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset);
uint16_t pci_get_vendor_id(uint8_t bus, uint8_t slot);
uint16_t pci_get_device_id(uint8_t bus, uint8_t slot);
