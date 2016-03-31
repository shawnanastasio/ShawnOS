/**
 * ShawnOS Kernel
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
#include <arch/i386/descriptors/idt.h>
extern void _i386_enter_pmode();
extern void _isr_debug();
extern void _isr0();

void kernel_early() {
	vga_textmode_initialize();

	gdt_install();
	printf("DEBUG: GDT Installed!\n");

	_i386_enter_pmode();
	printf("DEBUG: Protected mode entered!\n");

    idt_install();
    printf("DEBUG: IDT Installed!\n");
}

extern struct idt_gate_debug idt_debug[256];

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

  printf("ISR0 at: %d\n", (unsigned)_isr0);

  // Dump IDT
  int i;
  for (i=0;i<10;i++) {
      printf("%d\n", (int)idt_debug[i].base);
  }

  //_isr_debug();
  int a = 4;
  int b = 0;
  double kek = a/b;
  printf("%f", kek);
}
