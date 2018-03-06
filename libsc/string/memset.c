#include <string.h>
#include <stdint.h>

void* memset(void* bufptr, int value, size_t size)
{
	unsigned char* buf = (unsigned char*) bufptr;
	for ( size_t i = 0; i < size; i++ )
		buf[i] = (unsigned char) value;
	return bufptr;
}

void *memset32(void* bufptr, uint32_t value, size_t size)
{
	uint32_t* buf = (uint32_t*) bufptr;
	for ( size_t i = 0; i < size; i++ )
		buf[i] = value;
	return bufptr;
}
