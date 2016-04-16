/**
 * Basic kernel terminal driver
 * TODO: allow use of alternative video drivers
 */

#include <stdint.h>
#include <stddef.h>

#include <kernel/kernel_terminal.h>
#include <kernel/kernel_stdio.h>
#include <drivers/pc/pit.h>
#include <drivers/vga/textmode.h>

size_t cur_xpos = 0;
size_t cur_ypos = 0;
uint8_t color;

char terminal_buffer[VGA_HEIGHT * VGA_WIDTH];
uint16_t terminal_buffer_length = 0;

void kernel_terminal_init(uint16_t refresh_rate) {
  // Initalize driver for device to output to
  vga_textmode_initialize();

  color = make_color(COLOR_LIGHT_GREY, COLOR_BLACK);

  // Install terminal update tick to timer
  // TODO: un-hardcode use of PIT
  struct pit_routine kernel_terminal_task = {
      10000/refresh_rate,
      kernel_terminal_update_tick
  };
  pit_install_scheduler_routine(kernel_terminal_task);

}

void kernel_terminal_update_tick() {
  uint16_t i;

  // Get kernel STDOUT buffer
  char stdout_buffer[STDOUT_MAX_BUFFER];
  uint16_t stdout_buffer_length = kernel_buffer_stdout_get(stdout_buffer);

  if(stdout_buffer_length != 0) {
    // refresh current console
    vga_textmode_clear();

    // Restore old text from our buffer
    vga_textmode_writestring(terminal_buffer);

    // Update buffer
    for(i=0; i<stdout_buffer_length; i++) {
      vga_textmode_putentryat(stdout_buffer[i], color, cur_xpos++, cur_ypos);
      terminal_buffer[terminal_buffer_length++] = stdout_buffer[i];
    }
  }
}
