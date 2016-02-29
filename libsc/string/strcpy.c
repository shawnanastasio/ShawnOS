char *strcpy(char *restrict dest, const char *restrict src)
{
    char *ret = dest;
    while (*dest++ = *src++)
        ;
    return ret;
}
