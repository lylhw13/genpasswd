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

void xfree(void *ptr)
{
    if (ptr)
        free(ptr);
}

// static char *xstrdup(char *ori)
// {
//    return strcpy( xmalloc( strlen( ori) + 1), ori);
// }