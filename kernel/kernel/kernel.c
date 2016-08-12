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
    i386_mem_init(mboot_header);
    printk_debug("Memory Allocation functions enabled!");

    // Install drivers
    pit_timer_install_irq(); // Install PIT driver
    pckbd_install_irq(&pckbd_us_qwerty); // Install US PS/2 driver
    pci_init(); // Install PCI driver

    // Add kernel task to PIT
    struct pit_routine kernel_task_pit_routine = {
        10000, // Call kernel_task every second
        kernel_task
    };
    pit_install_scheduler_routine(kernel_task_pit_routine);

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

    uint32_t chunk = kernel_mem_kmalloc(61441);
    printf("Got chunk size 61440 at 0x%x\n", chunk);

    // DEBUG: Dump the first 16 frames
    uint32_t mem_counter = 1;
    uint32_t frame_num, frame_addr;
    uint32_t i;
    for(i=0; i<16; i++) {
        frame_num = i386_mem_peek_frame(&mem_counter);
        frame_addr = i386_mem_get_frame_start_addr(frame_num);
        if (frame_num == 0) break;
        printf("[mem] count: %d, num: %d, start: 0x%x\n", mem_counter, frame_num, frame_addr);
        //pit_timer_wait_ms(800);
    }

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
