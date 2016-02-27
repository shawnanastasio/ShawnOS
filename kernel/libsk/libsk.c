/**
 * Libc functions
 */

 #include <stdbool.h>
 #include <stddef.h>
 #include <stdint.h>

size_t strlen(const char* str) {
	size_t ret = 0;
	while ( str[ret] != 0 )
		ret++;
	return ret;
}

void *memcpy(void *str1, const void *str2, size_t n) {
  char *csrc = (char *)str2;
  char *cdest = (char *)str1;

  // Copy contents of src[] to dest[]
  size_t i;
  for (i=0; i<n; i++)
      cdest[i] = csrc[i];

  return str1;
}
