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
#include <kernel/mem/kernel_mem.h>

/* Driver includes */
#include <drivers/vga/textmode.h>
#include <drivers/pci/pci.h>

/* Architecture specific driver includes */
#include <drivers/pc/pit.h>
#include <drivers/pc/pckbd.h>
#include <drivers/pc/pckbd_us.h>

/* Architecture specific includes */
#include <pc.h>
#include <arch/i386/descriptors/gdt.h>
#include <arch/i386/descriptors/idt.h>
#include <arch/i386/irq.h>
#include <arch/i386/multiboot.h>
#include <arch/i386/io.h>
#include <arch/i386/mem.h>
extern void _i386_enter_pmode();


void kernel_early(uint32_t mboot_magic, multiboot_info_t *mboot_header) {
    // Set up kernel terminal for early output
    //kernel_terminal_init(14);

    // Verify multiboot magic
    if (mboot_magic != MULTIBOOT_BOOTLOADER_MAGIC) {
        printk_debug("Invalid Multiboot Magic!");
    }

    // Set up i386 tables and functions
    vga_textmode_initialize();

    gdt_install();
    printk_debug("GDT Installed!");
    _i386_enter_pmode();
    printk_debug("Protected mode entered!");
    idt_install();
    printk_debug("IDT Installed!");

    // Install drivers
    pit_timer_install_irq(); // Install PIT driver
    pckbd_install_irq(&pckbd_us_qwerty); // Install US PS/2 Driver

    // Add kernel task to PIT
    struct pit_routine kernel_task_pit_routine = {
        10000, // Call kernel_task every second
        kernel_task
    };
    pit_install_scheduler_routine(kernel_task_pit_routine);

    __asm__ __volatile__ ("sti");
    printk_debug("Interrupts Enabled!");

    i386_mem_init(mboot_header);
}

void kernel_main() {
    vga_textmode_writestring("Welcome to ");
    vga_textmode_setcolor(COLOR_CYAN);
    vga_textmode_writestring("ShawnOS ");
    vga_textmode_setcolor(make_color(COLOR_LIGHT_GREY, COLOR_BLACK));
    vga_textmode_writestring("Version ");
    vga_textmode_setcolor(COLOR_RED);
    vga_textmode_writestring("0.01 Alpha");
    vga_textmode_setcolor(make_color(COLOR_LIGHT_GREY, COLOR_BLACK));
    vga_textmode_writestring("!\n\n");

    pci_init();

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
    /*
    kernel_buffer_stdout_writestring("DEBUG: ", strlen("DEBUG: "));
    kernel_buffer_stdout_writestring(string, strlen(string));
    kernel_buffer_stdout_writechar('\n');
    */
    vga_textmode_setcolor(COLOR_RED);
    vga_textmode_writestring("DEBUG: ");
    vga_textmode_setcolor(make_color(COLOR_LIGHT_GREY, COLOR_BLACK));
    vga_textmode_writestring(string);
    vga_textmode_putchar('\n');
}
