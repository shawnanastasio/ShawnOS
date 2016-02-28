/**
 * Libsk -
 * Basic freestanding libc functions for kernel use
 */

 #include <stdbool.h>
 #include <stddef.h>
 #include <stdint.h>

size_t strlen(const char* str) {
	size_t ret = 0;
	while (str[ret] != 0)
		ret++;
	return ret;
}

int memcmp(const void* s1, const void* s2,size_t n) {
    const unsigned char *p1 = s1, *p2 = s2;

    size_t i;
    for(i=0; i<n; i++) {
        if(*p1 != *p2) {
            return *p1 - *p2;
        } else {
            p1++;
            p2++;
        }
    }
    return 0;
}

void *memcpy(void *str1, const void *str2, size_t n) {
  char *csrc = (char *)str2;
  char *cdest = (char *)str1;

  size_t i;
  for (i=0; i<n; i++)
      cdest[i] = csrc[i];

  return str1;
}


void *memmove(void *dest, const void *src, size_t n) {
    unsigned char tmp[n];
    memcpy(tmp,src,n);
    memcpy(dest,tmp,n);
    return dest;
}

void *memset(void *s, int c, size_t n) {
    unsigned char* p=s;
    size_t i;
    for(i=0; i<n; i++) {
        *p++ = (unsigned char)c;
    }
    return s;
}
