/**
 * Driver for standard PC PS/2 Keyboards
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pc.h>

#include <kernel/kernel.h>
#include <arch/i386/isr.h>
#include <arch/i386/irq.h>

unsigned char *pckbd_selected_scancode_table;

void pckbd_irq_input_handler(struct regs *r) {
    if ((r->int_no-32) != 1) {
        printf("ERROR: this routine needs to be triggered from IRQ 1\n");
        abort();
    }

    unsigned char cur_scancode;

    // Read from PS/2 buffer
    cur_scancode = inportb(0x60);

    if (cur_scancode & 0x80) {
        //Key has just been released

    } else {
        //Look up char in scancode table
        unsigned char cur_char = pckbd_selected_scancode_table[cur_scancode];

        //For now, just print out debug to screen
        printf("KEY PRESSED: %c\n", cur_char);
    }
}

// Install pckbd handler with specificed scancode table to IRQ1
void pckbd_install_irq(unsigned char *scancode_table) {
    pckbd_selected_scancode_table = scancode_table;
    irq_install_handler(1, pckbd_irq_input_handler);
}
