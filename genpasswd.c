#include "generic.h"

#define LENSIZE 1024

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

void xfree(void *ptr)
{
    if (ptr)
        free(ptr);
}

char *xstrdup(char *ori)
{
   return strcpy( xmalloc( strlen( ori) + 1), ori);
}

// int main(int argc, char *argv[])
// {
//     char *passwd;
//     char *passwd_verify;

//     passwd = xstrdup( getpass("please input your passwd: "));
//     // passwd_verify = xstrdup(getpass("re-input your passwd: "));

//     if (passwd[0] == '\0') {
//         puts("your passwd is empty");
//         return 0;
//     }

//     printf("your passwd is %s\n", passwd);
//     // printf("your re passwd is %s\n", passwd_verify);

//     return 0;
// }