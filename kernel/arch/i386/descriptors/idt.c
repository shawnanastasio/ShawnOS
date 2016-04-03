#include <stdint.h>
#include <string.h>

#include <arch/i386/descriptors/idt.h>
#include <arch/i386/isr.h>
#include <arch/i386/irq.h>

struct idt_entry idt[256];
struct idt_ptr idtp;

struct idt_gate_debug idt_debug[256];

extern void _idt_flush();

void _idt_set_gate(uint8_t num, uint64_t base, uint16_t sel,
                   uint8_t flags)
{

    //Add to debug gates
    idt_debug[num].num = num;
    idt_debug[num].base = base;
    idt_debug[num].sel = sel;
    idt_debug[num].flags = flags;

    idt[num].base_lo = (base & 0xFFFF);
    idt[num].base_hi = (base >> 16) & 0xFFFF;

    idt[num].sel = sel;
    idt[num].flags = flags;
}

/**
* Install the IDT tables with default settings
*/
void idt_install() {
    idtp.limit = (sizeof(struct idt_entry) * 256) - 1;
    idtp.base = (uint32_t)&idt;

    //Wipe IDT
    memset(&idt, 0, sizeof(struct idt_entry) * 256);

    //Install ISRs
    _isr_install();

    //Installs IRQs
    _irq_install();

    //Install IDT
    _idt_flush();
}
