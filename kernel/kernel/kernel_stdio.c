/**
 * Kernel STDIO buffers
 */

#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include <kernel/kernel.h>
#include <kernel/kernel_stdio.h>

char kernel_buffer_stdin[STDIN_MAX_BUFFER];
char kernel_buffer_stdout[STDOUT_MAX_BUFFER];

uint16_t kernel_buffer_stdin_length = 0;
uint16_t kernel_buffer_stdout_length = 0;

void kernel_buffer_stdin_flush() {
    /*
     * We don't actually need to wipe the buffer,
     * we just need to set its length to 0 so it can be
     * marked to be overwritten
     */

    kernel_buffer_stdin_length = 0;
}

void kernel_buffer_stdout_flush() {
    /*
     * We don't actually need to wipe the buffer,
     * we just need to set its length to 0 so it can be
     * marked to be overwritten
     */

    kernel_buffer_stdout_length = 0;
}

void kernel_buffer_stdin_writechar(char c) {
    if (kernel_buffer_stdin_length >= STDIN_MAX_BUFFER) {
        kernel_buffer_stdin_flush();
    }

    kernel_buffer_stdin[kernel_buffer_stdin_length++] = c;
}

void kernel_buffer_stdout_writechar(char c) {
    if (kernel_buffer_stdout_length >= STDOUT_MAX_BUFFER) {
        kernel_buffer_stdout_flush();
    }

    kernel_buffer_stdout[kernel_buffer_stdout_length++] = c;
}

void kernel_buffer_stdout_writestring(char *str, size_t length) {
  size_t i;
  for (i=0; i<length; i++) {
    kernel_buffer_stdout_writechar(str[i]);
  }
}

/**
 * Get stdout buffer and flush
 * @param char *buffer buffer to place stdout in
 * @return uint16_t size of buffer
 */
uint16_t kernel_buffer_stdout_get(char *buffer) {
    strncpy(buffer, kernel_buffer_stdout, kernel_buffer_stdout_length);
    uint16_t temp = kernel_buffer_stdout_length;

    kernel_buffer_stdout_flush();

    return temp;
}
