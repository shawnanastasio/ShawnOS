/**
 * Driver for standard PC PS/2 Keyboards
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stddef.h>
#include <pc.h>

#include <kernel/kernel.h>
#include <arch/i386/isr.h>
#include <arch/i386/irq.h>
#include <drivers/pc/pckbd.h>

struct pckbd_driver *pckbd_selected_driver;
bool pckbd_is_capslock = false;
bool pckbd_is_shift = false;

static inline bool set_contains_sc(pckbd_scancode_set_t *scs, int i) {
	size_t s;
	for(s = 0; s < scs->scancode_arr_len; s++) {
		if(scs->scancode_arr[s] == i) return true;
	}
	return false;
}

void pckbd_irq_input_handler(i386_registers_t *r) {
    if ((r->int_no-32) != 1) {
        printf("ERROR: this routine needs to be triggered from IRQ 1\n");
        abort();
    }

    unsigned char cur_scancode;

    // Read from PS/2 buffer
    cur_scancode = inportb(0x60);

    if (cur_scancode & 0x80) {
        // Key has just been released

        // Handle Shift
        if (set_contains_sc(pckbd_selected_driver->pckbd_shift, cur_scancode-128)) { 
            pckbd_is_shift = !pckbd_is_shift;
        }

    } else {
        //Look up char in scancode table
        int cur_char;

        // Handle shift
        if (set_contains_sc(pckbd_selected_driver->pckbd_shift, cur_scancode)) { 
            pckbd_is_shift = !pckbd_is_shift;
        }

        // Handle Caps Lock
        if (cur_scancode == pckbd_selected_driver->pckbd_caps_sc) {
            pckbd_is_capslock = !pckbd_is_capslock;
        }

        // Check if letter and make capital
        if (pckbd_is_capslock && pckbd_is_shift) {
            cur_char = pckbd_selected_driver->pckbd_sc_shift_capslock->scancode_arr[cur_scancode];        
        } else if (pckbd_is_shift) {
            cur_char = pckbd_selected_driver->pckbd_sc_shift->scancode_arr[cur_scancode];
        } else if (pckbd_is_capslock) {
            cur_char = pckbd_selected_driver->pckbd_sc_capslock->scancode_arr[cur_scancode];
        }

        else {
            cur_char = pckbd_selected_driver->pckbd_sc->scancode_arr[cur_scancode];
        }

        //For now, just print out debug to screen
        if (cur_char) {
            printf("%c", cur_char);
        }
    }
}

// Install pckbd handler with specificed scancode table to IRQ1
void pckbd_install_irq(struct pckbd_driver *d) {
	pckbd_selected_driver = d;
    irq_install_handler(1, pckbd_irq_input_handler);
}
