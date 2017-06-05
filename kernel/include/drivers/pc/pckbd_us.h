#pragma once

#include <stdint.h>
#include <drivers/pc/pckbd.h>

// US QWERTY
int _pckbd_us_qwerty_sc_arr[128] = {
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
	
pckbd_scancode_set_t pckbd_us_qwerty_sc = {
	.scancode_arr = _pckbd_us_qwerty_sc_arr,
	.scancode_arr_len = 128,
};

// Shift keys
int _pckbd_us_qwerty_shift_arr[2] = {
    42, //LShift
    54  //RShift
};

pckbd_scancode_set_t pckbd_us_qwerty_shift = {
	.scancode_arr = _pckbd_us_qwerty_shift_arr,
	.scancode_arr_len = 2,
};

// US QWERTY

struct pckbd_driver pckbd_us_qwerty = {
	.pckbd_sc = &pckbd_us_qwerty_sc,
	.pckbd_shift = &pckbd_us_qwerty_shift,
	.pckbd_caps_sc = 58,
	.pckbd_special = NULL // Temporary
};
