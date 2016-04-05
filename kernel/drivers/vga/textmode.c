/**
 * Functions for interfacing with the basic VGA text-mode buffer
 * Used for primitive video output
 */

 #include <stdbool.h>
 #include <stddef.h>
 #include <stdint.h>
 #include <pc.h>

 #include <string.h>

 static const size_t VGA_WIDTH = 80;
 static const size_t VGA_HEIGHT = 25;

 size_t vga_textmode_row;
 size_t vga_textmode_column;
 uint8_t vga_textmode_color;
 uint16_t* vga_textmode_buffer;

/* Hardware text mode color constants. */
enum vga_color {
	COLOR_BLACK = 0,
	COLOR_BLUE = 1,
	COLOR_GREEN = 2,
	COLOR_CYAN = 3,
	COLOR_RED = 4,
	COLOR_MAGENTA = 5,
	COLOR_BROWN = 6,
	COLOR_LIGHT_GREY = 7,
	COLOR_DARK_GREY = 8,
	COLOR_LIGHT_BLUE = 9,
	COLOR_LIGHT_GREEN = 10,
	COLOR_LIGHT_CYAN = 11,
	COLOR_LIGHT_RED = 12,
	COLOR_LIGHT_MAGENTA = 13,
	COLOR_LIGHT_BROWN = 14,
	COLOR_WHITE = 15,
};

/**
 * Use fancy bit operations to make color
 * @param  fg foreground color
 * @param  bg background color
 * @return    vga color code
 */
uint8_t make_color(enum vga_color fg, enum vga_color bg) {
	return fg | bg << 4;
}

/**
 * Make formatted vga entry to use in textmode buffer
 * @param  c     character
 * @param  color color of character
 * @return       formatted entry
 */
uint16_t make_vgaentry(char c, uint8_t color) {
	uint16_t c16 = c;
	uint16_t color16 = color;
	return c16 | color16 << 8;
}

/**
 * Set color of terminal
 * @param color color code
 */
void vga_textmode_setcolor(uint8_t color) {
	vga_textmode_color = color;
}

/**
 * Initialize text-mode buffer terminal
 */
void vga_textmode_initialize() {
	vga_textmode_row = 0;
	vga_textmode_column = 0;
	vga_textmode_setcolor(make_color(COLOR_LIGHT_GREY, COLOR_BLACK));
	vga_textmode_buffer = (uint16_t*) 0xB8000;
	for (size_t y = 0; y < VGA_HEIGHT; y++) {
		for (size_t x = 0; x < VGA_WIDTH; x++) {
			const size_t index = y * VGA_WIDTH + x;
			vga_textmode_buffer[index] = make_vgaentry(' ', vga_textmode_color);
		}
	}
}

/**
 * Put entry in text-mode buffer
 * @param c     character to put
 * @param color color of character
 * @param x     x coordinate of character
 * @param y     y coordinate of character
 */
void vga_textmode_putentryat(char c, uint8_t color, size_t x, size_t y) {
	const size_t index = y * VGA_WIDTH + x;
	vga_textmode_buffer[index] = make_vgaentry(c, color);
}

/**
 * Scrolls textmode buffer up one
 */
void vga_textmode_scroll() {
    for (size_t y = 0; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            const size_t index = y * VGA_WIDTH + x;
            const size_t nextrow_index = index + VGA_WIDTH;

            if (y == VGA_HEIGHT) //Last line, make it blank
                vga_textmode_buffer[index] = make_vgaentry(' ', vga_textmode_color);
            else
                vga_textmode_buffer[index] = vga_textmode_buffer[nextrow_index];
        }
    }
    vga_textmode_row = VGA_HEIGHT;
    vga_textmode_column = 0;
}

/**
 * Put character in text-mode buffer
 * @param c character
 */
void vga_textmode_putchar(char c) {

  //Handle \n newlines
  if (c == '\n') {
    ++vga_textmode_row;
    vga_textmode_column = 0;
  } else {
    vga_textmode_putentryat(c, vga_textmode_color, vga_textmode_column, vga_textmode_row);

    if (++vga_textmode_column == VGA_WIDTH) {
      vga_textmode_column = 0;
      ++vga_textmode_row;
    }
  }

  if (vga_textmode_row == VGA_HEIGHT+1) {
    //Scroll if all rows are taken up
    //vga_textmode_row = 0;
    vga_textmode_scroll();
  }

  // Update cursor position
  size_t cur_pos = vga_textmode_row * 80 + vga_textmode_column;

  // Write position to index 14 and 15 of VGA CRT control register
  outportb(0x3D4, 14);
  outportb(0x3D5, cur_pos >> 8);
  outportb(0x3D4, 15);
  outportb(0x3D5, cur_pos);
}

/**
 * Write string to text-mode buffer
 * @param data string to write
 */
void vga_textmode_writestring(const char* data) {
	size_t datalen = strlen(data);
	for (size_t i = 0; i < datalen; i++)
		vga_textmode_putchar(data[i]);
}

/**
 * Clear screen
 */
void vga_textmode_clear() {
 vga_textmode_row = 0;
 vga_textmode_column = 0;
 vga_textmode_setcolor(make_color(COLOR_LIGHT_GREY, COLOR_BLACK));
 for (size_t y = 0; y < VGA_HEIGHT; y++) {
   for (size_t x = 0; x < VGA_WIDTH; x++) {
     const size_t index = y * VGA_WIDTH + x;
     vga_textmode_buffer[index] = make_vgaentry(' ', vga_textmode_color);
   }
 }

 // Reset cursor position
 size_t cur_pos = 0;

 // Write position to index 14 and 15 of VGA CRT control register
 outportb(0x3D4, 14);
 outportb(0x3D5, cur_pos >> 8);
 outportb(0x3D4, 15);
 outportb(0x3D5, cur_pos);
}
