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
#include <mm/asa.h>

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

    // Init Address Space Allocator before enabling paging
    ASSERT(asa_init(PAGE_SIZE) == K_SUCCESS);

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

    //_i386_print_reserved();

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

    //kernel_thread_sleep(10);

    printf("Heap start: 0x%x\n", meminfo.kernel_heap_start);
    printf("Heap curpos: 0x%x\n", meminfo.kernel_heap_curpos);
    printf("kHighest page: 0x%x\n", kpaging_data.highest_page);

    asa_alloc(1000);
    uintptr_t phys, virt;
    k_return_t ret = i386_allocate_empty_pages(&i386_kernel_mmu_data, 1, &phys, &virt);
    ASSERT(!K_FAILED(ret));
    printk_debug("i386_a_e_p returned phys: 0x%x virt: 0x%x", phys, virt);

    for (;;) {
        void *tmp = asa_alloc(1);
        if (!tmp) {
            PANIC("Out of address space!");
        }
        ret = i386_allocate_page(&i386_kernel_mmu_data, (uint32_t)tmp,
            PT_PRESENT | PT_RW, PD_PRESENT | PD_RW, NULL);
        if (K_FAILED(ret)) {
            PANIC("FAILED TO ALLOCATE PAGE!");
        }
    }
#if 0
    for (;;) {
        uint32_t tmp = (uint32_t)kmalloc(0x999, KALLOC_GENERAL);
        if (!tmp) {printk_debug("REEEEE"); break;}
        tmp = tmp;
        //printk_debug("writing ff to 0x%x", tmp);
        //memset((void *)tmp, 0xFF, 0x1000);
        //kfree((void*)tmp);
        //printk_debug("Got 0x10000 bytes at 0x%x", tmp);
    }
#endif

    for (;;) {
        // Stress ASA
        void *tmp = asa_alloc(1);
        if (!tmp) {
            PANIC("GOOD: ASA returned NULL");
        }
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
