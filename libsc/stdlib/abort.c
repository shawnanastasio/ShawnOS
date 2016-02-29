#include <stdio.h>
#include <stdlib.h>

__attribute__((__noreturn__))
void abort(void)
{
#if defined(__is_shawnos_kernel)
	// TODO: Add proper kernel panic.
	printf("Kernel Panic: abort()\n");
	while ( 1 ) { }
	__builtin_unreachable();
#else
	// TODO: implement proper hosted abort
#endif
}
