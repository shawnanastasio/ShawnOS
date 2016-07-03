#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stddef.h>

#include <drivers/pci/pci.h>
#include <arch/i386/io.h>

#include <kernel/kernel.h>
#include <kernel/kernel_stdio.h>
#include <kernel/kernel_terminal.h>
#include <kernel/kernel_thread.h>

const char* PCI_CLASS_IDS[18] =
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

pci_device pci_devices[16];
uint32_t devices = 0;

/**
 * Read word at offset from pci device at bus `bus`, device `slot`, and function `func` (for multifunc device)
 */
uint16_t pci_config_read_word(uint16_t bus, uint16_t slot, uint16_t func, uint8_t offset) {
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

/**
 * Get vendor ID of PCI device at bus `bus`, device `slot`
 */
uint16_t pci_get_vendor_id(uint16_t bus, uint16_t slot, uint16_t func) {
    uint16_t vendor;

    vendor = pci_config_read_word(bus, slot, func, 0);
    return vendor;
}

/**
 * Get device ID of PCI device at bus `bus`, device `slot`
 */
uint16_t pci_get_device_id(uint16_t bus, uint16_t slot, uint16_t func) {
    uint16_t device;

    device = pci_config_read_word(bus, slot, func, 2);
    return device;
}

/**
 * Get class ID of pci device device on bus `bus`
 */
uint16_t pci_get_device_class_id(uint16_t bus, uint16_t slot, uint16_t func) {
    uint32_t class_id_seg;
    class_id_seg = pci_config_read_word(bus, slot, func, 0xA);

    return (class_id_seg & ~0x00FF) >> 8;
}

/**
 * Get subclassclass ID of pci device device on bus `bus`
 */
uint16_t pci_get_device_subclass_id(uint16_t bus, uint16_t slot, uint16_t func) {
    uint32_t subclass_id_seg;
    subclass_id_seg = pci_config_read_word(bus, slot, func, 0xA);

    return (subclass_id_seg & ~0xFF00);
}

/**
 * Initialize PCI device array and such
 */
 void pci_init() {
     printk_debug("Scanning for PCI devices.");
     devices = 0;
     pci_probe();
 }

 /**
  * Loop through PCI devices and print out information for the first 16 devices
  */
 void pci_probe() {
     for(uint16_t bus = 0; bus < 256; bus++) {
         for (uint16_t slot = 0; slot < 32; slot++) {
             for(uint16_t function = 0; function < 8; function++) {

                 uint16_t vendor_id = pci_get_vendor_id(bus, slot, function);
                 if(vendor_id == 0xFFFF) continue;
                 uint16_t device_id = pci_get_device_id(bus, slot, function);
                 uint16_t class_id = pci_get_device_class_id(bus, slot, function);
                 printf("[pci] %x:%x - %s\n", vendor_id, device_id, PCI_CLASS_IDS[class_id]);

                 if(devices > 15) {
                     printf("[pci] not adding above device... limit reached\n");
                     continue;
                 }
                 pci_device pcidev;
                 pcidev.vendor = vendor_id;
                 pcidev.device = device_id;
                 pcidev.func = function;

                 pci_devices[devices] = pcidev;
                 devices++;
             }
         }
     }
 }
