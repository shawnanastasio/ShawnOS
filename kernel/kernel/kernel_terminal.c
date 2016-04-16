/**
 * Basic kernel terminal driver
 * TODO: allow use of alternative video drivers
 */

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include <kernel/kernel_terminal.h>
#include <kernel/kernel_stdio.h>
#include <drivers/pc/pit.h>
#include <drivers/vga/textmode.h>

char kernel_terminal_escapecodes[15] =
{
    '\n',
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0
};


size_t cur_xpos = 0;
size_t cur_ypos = 0;
uint8_t color;

uint16_t terminal_buffer[VGA_HEIGHT * VGA_WIDTH];

void kernel_terminal_init(uint16_t refresh_rate) {
  // Initalize driver for device to output to
  vga_textmode_initialize();
  color = make_color(COLOR_LIGHT_GREY, COLOR_BLACK);

  // Initialize empty terminal_buffer
  for (size_t y = 0; y < VGA_HEIGHT; y++) {
    for (size_t x = 0; x < VGA_WIDTH; x++) {
      const size_t index = y * VGA_WIDTH + x;
      terminal_buffer[index] = make_vgaentry(' ', color);
    }
  }

  // Install terminal update tick to timer
  // TODO: un-hardcode use of PIT
  struct pit_routine kernel_terminal_task = {
      10000/refresh_rate,
      kernel_terminal_update_tick
  };
  pit_install_scheduler_routine(kernel_terminal_task);

  // Initially update terminal
  kernel_terminal_update_tick();
}

void kernel_terminal_update_tick() {
  uint16_t i;

  // Get kernel STDOUT buffer
  char stdout_buffer[STDOUT_MAX_BUFFER];
  uint16_t stdout_buffer_length = kernel_buffer_stdout_get(stdout_buffer);

  if(stdout_buffer_length != 0) {
    // Refresh current console
    vga_textmode_clear();

    // Restore old text from our buffer
    vga_textmode_writebuffer(terminal_buffer, VGA_WIDTH*VGA_HEIGHT);

    // Update buffer
    for(i=0; i<stdout_buffer_length; i++) {
      char cur_char = stdout_buffer[i];

      // Parse char for terminal escape codes
      if (kernel_terminal_check_escapecode(cur_char)) {
        kernel_terminal_handle_escapecode(cur_char);
      } else { // No escape codes
        // Check for overflow and wrap text accordingly
        //kernel_terminal_check_overflow();

        vga_textmode_putentryat(cur_char, color, cur_xpos++, cur_ypos);
        terminal_buffer[cur_ypos * VGA_WIDTH + cur_xpos] = make_vgaentry(cur_char, color);
      }
    }

  }
}

/**
 * Determines whether char is an escape code or not
 * @param c char to test
 */
bool kernel_terminal_check_escapecode(char c) {
  size_t i;
  for (i=0; i<ESCAPECODE_LENGTH; i++) {
    if (c == kernel_terminal_escapecodes[i]) {
      return true;
    }
  }

  return false;
}

void kernel_terminal_handle_escapecode(char c) {
  //TODO: Insert logic to handle all escape codes

  switch(c) {
    case '\n':
      cur_ypos++;
      cur_xpos = 0;
      break;
  }
}
