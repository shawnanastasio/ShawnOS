#include <string.h>

char *strncpy(char *restrict dest, const char *restrict src, size_t n)
{
    char *ret = dest;
    while (n--) {
        *dest++ = *src++;
    }
    return ret;
}
