#include <stdio.h>


#if defined(__is_shawnos_kernel)
#include <kernel/kernel_stdio.h>
#endif

int putchar(int ic)
{
#if defined(__is_shawnos_kernel)
	char c = (char) ic;
	kernel_buffer_stdout_writechar(c);
#else
	// TODO: You need to implement a write system call.
#endif
	return ic;
}
