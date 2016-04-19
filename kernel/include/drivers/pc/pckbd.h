#pragma once

#include <stdint.h>

#define PCKBD_SC_SIZE 128
#define PCKBD_SPECIAL_SIZE 128

void pckbd_install_irq(unsigned char *scancode_table, uint8_t *special_table);
bool pckbd_check_special(char c);

/**
 * Define scancode tables for different PS/2 keyboards
 */

// US QWERTY
unsigned char pckbd_us_qwerty_sc[128] = {
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8',
    '9', '0', '-', '=', '\b',
    '\t',
    'q', 'w', 'e', 'r',
    't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0,
    'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',
    '\'', '`',   0, // Left Shift
    '\\', 'z', 'x', 'c', 'v', 'b', 'n',
    'm', ',', '.', '/',   0, // Right Shift
    '*',
    0,	// Alt
    ' ',// Space bar
    0,	// Caps lock
    0,	// 59 - F1 key ... >
    0,   0,   0,   0,   0,   0,   0,   0,
    0,	// < ... F10
    0,	// 69 - Num lock
    0,	// Scroll Lock
    0,	// Home key
    0,	// Up Arrow
    0,	// Page Up
    '-',
    0,	// Left Arrow
    0,
    0,	// Right Arrow
    '+',
    0,	// 79 - End key
    0,	// Down Arrow
    0,	// Page Down
    0,	// Insert Key
    0,	// Delete Key
    0,   0,   0,
    0,	// F11 Key
    0,	// F12 Key
    0,	// All other keys are undefined
};

/**
 * Special keys layout
 * Order:
 * 	LShift
 * 	Caps Lock
 */

// US QWERTY
uint8_t pckbd_us_qwerty_special[128] = {
    42,
    58
};
