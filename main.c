#include "generic.h"
#include "encrypt.h"
#include <time.h>

#define KEY_LEN 32
#define IV_LEN 16

#define PROGRAM_NAME "genpasswd"

static int hashtimes = 3;
static int taghash_len = 16;
unsigned short key_offset= 64;  /* todo check the offset */
unsigned short iv_offset = 100; /* todo check the offset */
unsigned char key[KEY_LEN + 1]; /* A 256 bit key, 32 byte */
unsigned char iv[IV_LEN + 1];   /* A 128 bit IV, 16 byte */

enum OP_type {
    INIT_RECORDS,
    LIST_RECORDS,
    INSERT_RECORDS,
    REMOVE_RECORDS,
};

static void usage(int status)
{
    printf("\
Usage: %s [OPTION][ARG]\n\
   or: %s [OPTION]\n\n", PROGRAM_NAME, PROGRAM_NAME);

    fputs("\
    -i          init the database\n\
    -l          list all tags\n\
    -t [ARG]    generate a passwd for the arg. And add the arg to the database\n\
    -r [ARG]    remove the arg from the database\n\
    -h          show usage\n\n\
    without argument, the program will generate a random password\n", stdout);
    exit(status);
}

static char * generate_random_password(void)
{
    int length;
    char str[16];
    char *hash;
    int base = 9973;    /* 9973 is a random prime number */
    int hashtimes = random() % base + 97;
    time_t now = time(0);

    /* convert int to string  */
    length = snprintf(NULL, 0, "%d", (int)now);
    snprintf(str, length + 1, "%d", (int)now);

    hash = sha_to_hex(sha512_multi(str, hashtimes));
    printf("Random passwd is %.*s\n", taghash_len, hash);
    return hash;
}


static char * init_ctx(char *passwd)
{
    char * hash;
    hash = sha_to_hex(sha512_multi(passwd, hashtimes));

    memcpy(key, hash + key_offset, KEY_LEN);
    key[KEY_LEN] = '\0';
    memcpy(iv, hash + iv_offset, IV_LEN);
    iv[IV_LEN] = '\0';

    enc_ctx_init(key, iv);
    return hash;
}

int main(int argc, char *argv[])
{
    int c, i, lockfd;
    enum OP_type op_type;
    struct record_entry * re;
    int entries;

    int hashtimes = 1;
    char *passwd, *passwd_hash;
    char *tag, *tag_hash;

    if (argc == 1) {
        generate_random_password();
        exit(EXIT_SUCCESS);
    }

    if (argc > 3) 
        usage(EXIT_FAILURE);

    while ((c = getopt(argc, argv, "ilt:r:h")) != -1) {
        switch (c)
        {
        case 'i':
            op_type = INIT_RECORDS; 
            break;
        case 'l':
            op_type = LIST_RECORDS;
            break;
        case 't':
            tag = optarg;
            op_type = INSERT_RECORDS;
            break;
        case 'r':
            tag = optarg;
            op_type = REMOVE_RECORDS;
            break;
        case 'h':
            usage(EXIT_SUCCESS);
            break;

        default:
            usage(EXIT_FAILURE);
        }
    }
    if (argc != optind) {
        fputs("Have non-option argument\n", stdout);
        usage(EXIT_FAILURE);
    }

    /* read the lock file */
    errno = 0;
    lockfd = open(LOCK_FILE, O_RDWR | O_CREAT | O_EXCL, 0600);
    if (lockfd < 0) {
        if (errno == EACCES)
            error("Unable to start: Permission denied.");

        if (errno == EEXIST)
            error("Unable to start: More than one process access the directory.");
        
        return error("Unable to create lockfile");
    }
    
    if (op_type == INIT_RECORDS) {
        passwd = init_passwd();
        init_ctx(passwd);
        if (write_records_encry() == 0)
            puts("Init records successed");
        else 
            puts("Fail to init records");
        goto out1;
    }

    /* verify password three times */
    passwd_hash = NULL;
    for (i = 0; i < 3; ++i) {
        passwd = getpass("Database Password: ");
        passwd_hash = init_ctx(passwd);

        entries = read_records_decry();
        if (entries < 0) {
            if (entries == DECRY_ERR) {
                fprintf(stdout, "Wrong passwd");
                cleanup();
                continue;
            }
            else {
                goto out1;
            }
        }
        break;
    }

    if (entries < 0 || passwd_hash == NULL)
        goto out1;
    

    if (op_type == LIST_RECORDS) {
        if (entries == 0)
            printf("%s\n", "There no records");
        for (i = 0; i < active_nr; ++i) 
        {
            re = active_entry[i];
            printf("%d %s\n",  i + 1, re->tag);
        } 
        goto out1;
    }
    else if (op_type == INSERT_RECORDS) {
        add_entry_by_tag(tag);
        tag_hash = sha_to_hex(sha512_multi_salt(passwd, tag, hashtimes));
        printf("The password for %s is %.*s\n", tag, taghash_len, tag_hash);
    }
    else if (op_type == REMOVE_RECORDS) {
        remove_entry_by_tag(tag);
        printf("remove %s from database\n", tag);
    }

    write_records_encry();
out1:
    cleanup();
    unlink(LOCK_FILE);
    return 0;
}