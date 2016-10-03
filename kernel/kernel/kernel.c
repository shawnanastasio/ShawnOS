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

/* Kernel includes */
#include <kernel/kernel.h>
#include <kernel/kernel_thread.h>
#include <kernel/kernel_stdio.h>
#include <kernel/kernel_terminal.h>
#include <kernel/kernel_mem.h>
#include <kernel/bitset.h>

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
#include <arch/i386/paging.h>

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
    idt_install();
    printk_debug("IDT Installed!");
    i386_mem_init(mboot_header);
    printk_debug("Memory Allocation functions enabled!");
    i386_paging_init();
    printk_debug("Paging enabled!");

    // Install drivers
    pit_timer_install_irq(); // Install PIT driver
    pckbd_install_irq(&pckbd_us_qwerty); // Install US PS/2 driver
    //pci_init(); // Install PCI driver

    // Add kernel task to PIT
    struct pit_routine kernel_task_pit_routine = {
        PIT_TIMER_CONSTANT, // Call kernel_task every second
        kernel_task
    };
    pit_install_scheduler_routine(kernel_task_pit_routine);

    _i386_print_reserved();

    __asm__ __volatile__ ("sti");
    printk_debug("Interrupts Enabled!");
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

    i386_allocate_page(0xA0000000, PT_PRESENT | PT_RW, PD_PRESENT | PD_RW);
    i386_free_page(0xA0000000);
    i386_allocate_page(0xA0000000, PT_PRESENT | PT_RW, PD_PRESENT | PD_RW);
    uint32_t *test = (uint32_t *)0xA0000000;
    printf("test: %d\n", *test);
    i386_allocate_page(0x10001001, PT_PRESENT | PT_RW, PD_PRESENT | PD_RW);

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
