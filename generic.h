#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <stddef.h>
#include <errno.h>
#include <fcntl.h>

#define CACHE_SIGNATURE 0x44495243 // "DIRC"

#define alloc_nr(x) (((x)+16)*3/2)

#define record_entry_size(len) ((offsetof(struct record_entry, tag) + (len) + 8) & ~7)
#define re_size(re) record_entry_size((re)->taglen)

#define LOCK_FILE ".index.lock"
#define RECORD_FILE "passwd.txt"

#define DECRY_ERR -2

struct record_header {
    unsigned int signature;
    unsigned int version;
    unsigned int entries;
};

struct record_entry {
    unsigned short taglen;
    char tag[0];
};

struct record_entry **active_entry;
unsigned int active_nr, active_alloc;

/* sha fun */
extern char *sha_to_hex(unsigned char *sha1);
extern unsigned char *sha512_once(const char *passwd);
extern unsigned char *sha512_multi(const char *passwd, int num);
extern unsigned char *sha512_multi_salt(const char *passwd, const char *salt, int num);

/* manipulate records */
extern int read_records_decry(void);
extern int write_records_encry(void);

extern int read_records(void);  /* for test */
extern int write_records(void); /* for test */

extern void list_records(void);

extern int add_entry_by_id(unsigned int pos);
extern int add_entry_by_tag(char *tag);
extern int add_entry(struct record_entry *re);

extern void remove_entry_by_id(unsigned int pos);
extern void remove_entry_by_tag(char *tag);
extern void remove_entry(struct record_entry *re);

/* password */
extern char * init_passwd(void);

/* utility */
extern int error(const char*string);
extern void *xmalloc(size_t size);
extern void xfree(void *ptr);


// static char *xstrdup(char *ori)
// {
//    return strcpy( xmalloc( strlen( ori) + 1), ori);
// }


