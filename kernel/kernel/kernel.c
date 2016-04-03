/**
* ShawnOS Kernel
*/

/* General C language includes */
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include <kernel/kernel.h>

/* Driver includes */
#include <drivers/vga/textmode.h>

/* Architecture specific driver includes */
#include <drivers/pc/pit.h>

/* Architecture specific includes */
#include <pc.h>
#include <arch/i386/descriptors/gdt.h>
#include <arch/i386/descriptors/idt.h>
#include <arch/i386/irq.h>
extern void _i386_enter_pmode();

void kernel_early() {
    // Set up i386 tables and functions
    vga_textmode_initialize();
    gdt_install();
    printf("DEBUG: GDT Installed!\n");
    _i386_enter_pmode();
    printf("DEBUG: Protected mode entered!\n");
    idt_install();
    printf("DEBUG: IDT Installed!\n");
    __asm__ __volatile__ ("sti");
    printf("DEBUG: Interrupts Enabled!\n");

    // Install drivers
    pit_timer_install_irq();

    //Add kernel task to PIT
    struct pit_routine kernel_task_pit_routine = {
        1, // Call kernel_task every second
        kernel_task
    };
    pit_install_scheduler_routine(kernel_task_pit_routine);

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
    vga_textmode_writestring("!\n\n");

    //_isr_debug();
    /*
    int a = 4;
    int b = 0;
    double kek = a/b;
    printf("%f", kek);
    */

    //dump idt debug
    int i;
    for (i=45; i<48; i++) {
        printf("%d\n", (int)idt_debug[i].base);
    }

    for(;;);
}

/**
 * Kernel task to be called at defined interval
 */
void kernel_task() {
    printf("DEBUG: kernel_task called!\n");
}
