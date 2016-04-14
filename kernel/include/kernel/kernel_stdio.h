#pragma once

#include <stdint.h>

#define STDIN_MAX_BUFFER 256
#define STDOUT_MAX_BUFFER 256

/**
 * Get stdout buffer and flush
 * @param char *buffer buffer to place stdout in
 * @return uint16_t size of buffer
 */
uint16_t kernel_buffer_stdout_get(char *buffer);

void kernel_buffer_stdin_writechar(char c);
void kernel_buffer_stdout_writechar(char c);

void kernel_buffer_stdout_writestring(char *str, size_t length);

void kernel_buffer_stdin_flush();
void kernel_buffer_stdout_flush();
