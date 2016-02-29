#ifndef _VGA_TEXTMODE_H
#define _VGA_TEXTMODE_H 1

#include <stddef.h>
#include <stdint.h>



/**
 * Functions for interfacing with the basic VGA text-mode buffer
 * Used for primitive video output
 */

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
 uint8_t make_color(enum vga_color fg, enum vga_color bg);

 /**
  * Make formatted vga entry to use in textmode buffer
  * @param  c     character
  * @param  color color of character
  * @return       formatted entry
  */
 uint16_t make_vgaentry(char c, uint8_t color);

 /**
  * Set color of terminal
  * @param color color code
  */
 void vga_textmode_setcolor(uint8_t color);

 /**
  * Initialize text-mode buffer terminal
  */
 void vga_textmode_initialize();

 /**
  * Put entry in text-mode buffer
  * @param c     character to put
  * @param color color of character
  * @param x     x coordinate of character
  * @param y     y coordinate of character
  */
 void vga_textmode_putentryat(char c, uint8_t color, size_t x, size_t y);

 /**
  * Scrolls textmode buffer up one
  */
 void vga_textmode_scroll();

 /**
  * Put character in text-mode buffer
  * @param c character
  */
 void vga_textmode_putchar(char c);

 /**
  * Write string to text-mode buffer
  * @param data string to write
  */
 void vga_textmode_writestring(const char* data);

 #endif
