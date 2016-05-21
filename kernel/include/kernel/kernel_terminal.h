#pragma once

#include <stdint.h>
#include <stdbool.h>

#define ESCAPECODE_LENGTH 15

void kernel_terminal_init(uint16_t refresh_rate);
void kernel_terminal_update_tick();
void kernel_terminal_handle_escapecode(char c);
bool kernel_terminal_check_escapecode(char c);
void kernel_terminal_handle_overflow();
void kernel_terminal_backspace();
void kernel_terminal_putentry(char c, uint8_t color, size_t x, size_t y);
void kernel_terminal_handle_scroll();
void kernel_terminal_update_cursor();
