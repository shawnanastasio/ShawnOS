#pragma once

#include <stdint.h>

#include <arch/i386/isr.h>

#define PIT_TIMER_CONSTANT 10000

/**
 * Scheduler structure
 */
struct pit_routine {
    uint32_t hz; // Number of milliseconds between each trigger
    void *func; // Function to call at each interval
};

uint16_t pit_install_scheduler_routine(struct pit_routine r);
void pit_uninstall_scheduler_routine(uint16_t index);

void pit_set_timer_phase(int16_t hz);
void pit_irq_timer_handler(i386_registers_t *r);
void pit_timer_install_irq();
uint32_t pit_get_total_ticks();
void pit_timer_wait(uint32_t seconds);
void pit_timer_wait_ms(uint32_t ms);
