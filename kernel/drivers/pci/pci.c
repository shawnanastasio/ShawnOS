#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stddef.h>

#include <drivers/pci/pci.h>
#include <arch/i386/io.h>

uint16_t pci_config_read_word(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {

  uint32_t address;
  uint32_t lbus = (uint32_t)bus;
  uint32_t lslot = (uint32_t)slot;
  uint32_t lfunc = (uint32_t)func;
  uint16_t tmp = 0;

  // Create config addr
  address = (uint32_t)((lbus << 16) | (lslot << 11) | (lfunc << 8) | (offset & 0xFC) | ((uint32_t)0x80000000));

  // Write out PCI config address
  IoWrite32(PCI_CONFIG_ADDR, address);

  // Read PCI config data
  // (offset & 2) * 8 = 0 will choose first word of 32bits register
  tmp = (uint16_t)((IoRead32(PCI_CONFIG_DATA) >> ((offset & 2) * 8)) & 0xffff);
  return(tmp);
}

// get pci device vendor id at bus slot
uint16_t pci_get_vendor_id(uint8_t bus, uint8_t slot) {
  uint16_t vendor;

  vendor = pci_config_read_word(bus, slot, 0, 0);
  return vendor;
}

// get pci device ddevice id at bus slot
uint16_t pci_get_device_id(uint8_t bus, uint8_t slot) {
  uint16_t device;

  device = pci_config_read_word(bus, slot, 0, 2);
  return device;
}
