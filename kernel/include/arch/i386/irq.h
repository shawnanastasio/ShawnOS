#pragma once

#include <stdint.h>

#include <arch/i386/isr.h>

void irq_install_handler(int32_t irq, void (*handler)(i386_registers_t *r));
void irq_uninstall_handler(int32_t irq);
void _irq_install();
void __irq_remap();
