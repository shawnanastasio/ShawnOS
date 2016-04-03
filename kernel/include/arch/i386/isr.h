#pragma once

#include <stdint.h>

void _isr_install();

/* This defines what the stack looks like after an ISR was running */
struct regs
{
    uint32_t gs, fs, es, ds;
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
    uint32_t int_no, err_code;
    uint32_t eip, cs, eflags, useresp, ss;
};
