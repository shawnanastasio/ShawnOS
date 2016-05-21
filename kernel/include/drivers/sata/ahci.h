#pragma once

typedef enum {
    FIS_TYPE_REG_H2D        = 0x27, // Register FIS, host to device
    FIS_TYPE_REG_D2H        = 0x34, // Register FIS, device to host
    FIS_TYPE_DMA_ACT        = 0x39, // DMA activate FIS, device to host
    FIS_TYPE_DMA_SETUP      = 0x41, // DMA setup FIS, bidirectional
    FIS_TYPE_DATA           = 0x46, // Data FIS, bidirectional
    FIS_TYPE_BIST           = 0x58, // BIST activate FIS, bidirectional
    FIS_TYPE_PIO_SETUP      = 0x5F, // PIO setup FIS, device to host
    FIS_TYPE_DEV_BITS       = 0xA1, // Set device bits FIS, device to host
} FIS_TYPE;
