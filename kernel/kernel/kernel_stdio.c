/**
 * Kernel STDIO buffers
 */

#include <stdint.h>
#include <string.h>

#include <kernel/kernel.h>

char kernel_buffer_stdin[265];
char kernel_buffer_stdout[256];

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
    if (kernel_buffer_stdin_length >= 256) {
        kernel_buffer_stdin_flush();
    }

    kernel_buffer_stdin[kernel_buffer_stdin_length++] = c;
}

void kernel_buffer_stdout_writechar(char c) {
    if (kernel_buffer_stdout_length >= 256) {
        kernel_buffer_stdout_flush();
    }

    kernel_buffer_stdout[kernel_buffer_stdout_length++] = c;
}

char * kernel_buffer_stdout_get() {
    char temp[kernel_buffer_stdout_length];
    strncpy(temp, kernel_buffer_stdout, kernel_buffer_stdout_length);
    kernel_buffer_stdout_flush();
    return temp;
}
