#include "generic.h"

int error(const char* string) 
{
    fprintf(stderr, "error: %s\n", string);
    return -1;
}

void *xmalloc(size_t size)
{
    void *ptr;
    ptr = malloc(size);
    if (NULL == ptr) {
        perror("malloc");
        exit(1);
    }
    return ptr;
}
