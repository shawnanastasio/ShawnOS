#pragma once

#include <stdint.h>

struct pckbd_scancode_set {
	int *scancode_arr;
	size_t scancode_arr_len;
};
typedef struct pckbd_scancode_set pckbd_scancode_set_t;

struct pckbd_driver {
  pckbd_scancode_set_t *pckbd_sc; // Default scancode characters
  pckbd_scancode_set_t *pckbd_sc_shift; // Scancode characters modified by shift
  pckbd_scancode_set_t *pckbd_shift; // Scancodes of shift keys
  pckbd_scancode_set_t *pckbd_sc_capslock;
  pckbd_scancode_set_t *pckbd_special;
  int                   pckbd_caps_sc; // Caps lock scancode
};

void pckbd_install_irq(struct pckbd_driver *d);
bool pckbd_check_special(char c);
