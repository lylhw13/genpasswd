#include "generic.h"

int error(const char* string) 
{
    fprintf(stderr, "error: %s\n", string);
    return -1;
}

char *xmalloc(size_t size)
{
    char *ptr;
    ptr = (char *)malloc(size);
    if (NULL == ptr) {
        perror("malloc");
        exit(1);
    }
    return ptr;
}
