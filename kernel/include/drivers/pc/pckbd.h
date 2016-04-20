#pragma once

#include <stdint.h>

#define PCKBD_SC_SIZE 128
#define PCKBD_SPECIAL_SIZE 128

struct pckbd_driver {
  unsigned char *pckbd_sc;
  uint8_t *pckbd_special;
};

void pckbd_install_irq(struct pckbd_driver *d);
bool pckbd_check_special(char c);
