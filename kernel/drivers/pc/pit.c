/**
 * Driver for the standard Programmable Interval Timer (PIT) (AKA 8253/4)
 * found on x86 PC systems
 *
 * Adapted from: http://www.osdever.net/bkerndev/Docs/pit.htm
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <pc.h>

#include <arch/i386/isr.h>
#include <arch/i386/irq.h>

#include <drivers/pc/pit.h>

/**
 * Array containing pit_routine structs to be checked at each
 * PIT call.
 */
struct pit_routine _pit_routines[64] =
{
    {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
    {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
    {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
    {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
    {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
    {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
    {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0},
    {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}
};

uint16_t _pit_routines_size = 0;

/**
 * Installs a routine to be added to scheduler
 * returns index of routine in array, or -1 if full
 */
uint16_t pit_install_scheduler_routine(struct pit_routine r) {
    if (_pit_routines_size >= 64) { return -1; }
    _pit_routines[_pit_routines_size] = r;
    return _pit_routines_size++;
}

/**
 * Uninstalls a routine from the PIT scheduler
 */
void pit_uninstall_scheduler_routine(uint16_t index) {
    _pit_routines[index].hz = 0;
}

// Set rate in Hz for PIT ticks
void pit_set_timer_phase(int16_t hz) {
    int16_t divisor = 1193180 / hz;   /* Calculate our divisor */
    outportb(0x43, 0x36);             /* Set our command byte 0x36 */
    outportb(0x40, divisor & 0xFF);   /* Set low byte of divisor */
    outportb(0x40, divisor >> 8);     /* Set high byte of divisor */
}

// Total number of ticks since system has been running
volatile uint32_t pit_total_timer_ticks = 0;

// IRQ routine to handle PIT tick
void pit_irq_timer_handler(struct regs *r) {
    if ((r->int_no-32) != 0) {
        printf("ERROR: this routine needs to be triggered from IRQ 0\n");
        abort();
    }

    ++pit_total_timer_ticks;

    // Check if we have to trigger any pit routines
    int i;
    for (i=0; i<_pit_routines_size; i++) {
        if (pit_total_timer_ticks % ((PIT_TIMER_CONSTANT/10000) *
            _pit_routines[i].hz) == 0) //If this pit routine is to be triggered
        {
            void (*__current_routine)(void);
            __current_routine = _pit_routines[i].func;
            __current_routine();
        }
    }
}

// Set the PIT Tick rate and install IRQ handler
void pit_timer_install_irq() {
    pit_set_timer_phase(PIT_TIMER_CONSTANT);
    irq_install_handler(0, pit_irq_timer_handler);
}

// Return total number of ticks passed
uint32_t pit_get_total_ticks() {
    return pit_total_timer_ticks;
}

// Wait specified number of seconds
void pit_timer_wait(uint32_t seconds) {
  uint32_t desired_ticks = pit_total_timer_ticks + (seconds * PIT_TIMER_CONSTANT);
  while (desired_ticks > pit_total_timer_ticks);
}

// Wait specified number of milliseconds
void pit_timer_wait_ms(uint32_t ms) {
  uint32_t desired_ticks = pit_total_timer_ticks + (ms * (PIT_TIMER_CONSTANT/1000));
  while (desired_ticks > pit_total_timer_ticks);
}
