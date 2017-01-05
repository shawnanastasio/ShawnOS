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
#include <kernel/bitset.h>
#include <mm/heap.h>
#include <mm/paging.h>
#include <mm/alloc.h>

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

extern struct idt_entry idt[256];

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
    
    
    // Install kernel heap as default malloc/free provider
    kheap_kalloc_install();


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

    /*
    for (;;) {
        printf("allocd: %d\n", i386_mem_allocate_frame());
        kernel_thread_sleep(1);
    }
    */

    printf("Heap start: 0x%x\n", meminfo.kernel_heap_start);
    printf("Heap curpos: 0x%x\n", meminfo.kernel_heap_curpos);
    printf("kHighest page: 0x%x\n", kpaging_data.highest_page);

    // Test heap
    uintptr_t *test1 = kmalloc(0x1050);
    printf("Got 0x1050 bytes at 0x%x\n", (uintptr_t)test1);
    *test1 = 0xDEADBEEF;
    kfree(test1);
    test1 = kmalloc(0x10050);
    printf("Got 0x10050 bytes at 0x%x\n", (uintptr_t)test1);


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
void printk_debug(char *fmt, ...) {
    vga_textmode_setcolor(COLOR_RED);
    vga_textmode_writestring("DEBUG: ");
    vga_textmode_setcolor(make_color(COLOR_LIGHT_GREY, COLOR_BLACK));

    va_list args;
    va_start(args,fmt);
    vprintf(fmt, args);
    va_end(args);

    putchar('\n');
}
