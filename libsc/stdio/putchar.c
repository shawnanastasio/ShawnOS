#include <stdio.h>


#if defined(__is_shawnos_kernel)
#include <drivers/vga/textmode.h>
#endif

int putchar(int ic)
{
#if defined(__is_shawnos_kernel)
	char c = (char) ic;
	vga_textmode_putchar(c);
#else
	// TODO: You need to implement a write system call.
#endif
	return ic;
}
