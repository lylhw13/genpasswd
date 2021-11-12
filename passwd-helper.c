#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#define PASSWD_LEN 16

char * init_passwd(void)
{
    char *passwd;
    char pwd_local[PASSWD_LEN + 1];
    int i, j;
    int state = 0;

    for (i = 0 ;i < 3 ; ++i) {
        passwd = getpass("New password for database: ");
        if (strlen(passwd) > PASSWD_LEN) {
            printf("Your passwd is too long. The max length is %d\n", PASSWD_LEN);
            continue;
        }
        if (strlen(passwd) == 0) {
            puts("Empty password.");
            continue;
        }
        for (j = 0; j < strlen(passwd); ++j) {
            if (!isprint(passwd[j])) {
                puts("Contains non-printable character.");
                exit(EXIT_FAILURE);
            }
        }

        strcpy(pwd_local, passwd);

        passwd = getpass("Retype new password for database: ");
        if (strcmp(pwd_local, passwd)) {
            puts("Sorry, passwords do not match.");
            continue;
        }
        state = 1;
        break;
    }

    if (state)
        return passwd;
    else
        return NULL;
}