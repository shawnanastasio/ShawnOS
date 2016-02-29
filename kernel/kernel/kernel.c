/**
 * mykernel
 */

/* General C language includes */
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

/* Driver Includes */
#include <drivers/vga/textmode.h>

void kernel_main() {
	/* Initialize terminal interface */
	vga_textmode_initialize();

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

	printf("God bless %s!\n", "printf");

}
