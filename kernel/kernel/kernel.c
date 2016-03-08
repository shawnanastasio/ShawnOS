/**
 * mykernel
 */

/* General C language includes */
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

/* Driver Includes */
#include <drivers/vga/textmode.h>

/* Architecture specific includes */
#include <arch/i386/descriptors/gdt.h>
extern void _i386_enter_pmode();

void kernel_early() {
	vga_textmode_initialize();
	gdt_install();
	printf("DEBUG: GDT Installed!\n");
	_i386_enter_pmode();
	vga_textmode_writestring("DEBUG: Protected mode entered!\n");
}

void kernel_main() {
  // Display welcome message
  vga_textmode_writestring("Welcome to ");
  vga_textmode_setcolor(COLOR_CYAN);
  vga_textmode_writestring("ShawnOS ");
  vga_textmode_setcolor(make_color(COLOR_LIGHT_GREY, COLOR_BLACK));
  vga_textmode_writestring("Version ");
  vga_textmode_setcolor(COLOR_RED);
  vga_textmode_writestring("0.01 Alpha");
  vga_textmode_setcolor(make_color(COLOR_LIGHT_GREY, COLOR_BLACK));
  vga_textmode_writestring("!\n");
}
