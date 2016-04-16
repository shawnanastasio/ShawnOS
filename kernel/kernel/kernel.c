/**
 * ShawnOS Kernel
 */

/* General C language includes */
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <kernel/kernel.h>
#include <kernel/kernel_thread.h>
#include <kernel/kernel_stdio.h>
#include <kernel/kernel_terminal.h>

/* Driver includes */
#include <drivers/vga/textmode.h>

/* Architecture specific driver includes */
#include <drivers/pc/pit.h>
#include <drivers/pc/pckbd.h>

/* Architecture specific includes */
#include <pc.h>
#include <arch/i386/descriptors/gdt.h>
#include <arch/i386/descriptors/idt.h>
#include <arch/i386/irq.h>
extern void _i386_enter_pmode();


void kernel_early() {
  // Set up kernel terminal for early output
  kernel_terminal_init(14);

  // Set up i386 tables and functions
  //vga_textmode_initialize();
  gdt_install();
  printk_debug("GDT Installed!");
  _i386_enter_pmode();
  printk_debug("Protected mode entered!");
  idt_install();
  printk_debug("IDT Installed!");

  // Install drivers
  pit_timer_install_irq(); // Install PIT driver
  pckbd_install_irq(pckbd_us_qwerty_sc); // Install US PS/2 Driver

  //Add kernel task to PIT
  struct pit_routine kernel_task_pit_routine = {
    10000, // Call kernel_task every second
    kernel_task
  };
  pit_install_scheduler_routine(kernel_task_pit_routine);

  __asm__ __volatile__ ("sti");
  printk_debug("Interrupts Enabled!");
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
  vga_textmode_writestring("!\n\n");

  // Test kernel terminal
  kernel_buffer_stdout_writestring("Hello, Terminal!", strlen("Hello, Terminal!"));
  for(;;);
}

/**
 * Kernel task to be called at defined interval
 */
void kernel_task() {
  //printk_debug("kernel_task called!");
}

/**
 * Prints a kernel DEBUG message
 */
void printk_debug(char *string) {
  vga_textmode_setcolor(COLOR_RED);
  vga_textmode_writestring("DEBUG: ");
  vga_textmode_setcolor(make_color(COLOR_LIGHT_GREY, COLOR_BLACK));
  vga_textmode_writestring(string);
  vga_textmode_putchar('\n');
}
