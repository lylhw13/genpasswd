#include "generic.h"
#include "encrypt.h"

static char *passwdfile = "passwd.txt";
static char* tags[] ={"z", "z", "hello world", "your passwd", "your passwd", "hahaha", "acer", "screen shot 1234"};
static char *example = "hello hahaha";


static int tag_compare(char *tag1, int len1, char *tag2, int len2)
{
    int len = len1 < len2 ? len1 : len2 ;
    int cmp;
    cmp = memcmp(tag1, tag2, len);

    if (cmp)
        return cmp;
    if (len1 < len2)
        return -1;
    if (len1 > len2)
        return 1;
    return 0;
}

/* pos is negitave, start from 1*/
static int tag_pos(char *tag, int len)
{
    int first, last;
    int mid, cmp;

    first = 0;
    last = active_nr;

    while (first < last) {
        mid = (last - first) / 2 + first;
        struct record_entry *re = active_entry[mid];
        cmp = tag_compare(re->tag, re->taglen, tag, len);

        if (!cmp) {
            return -mid - 1;
        }
        else if(cmp < 0) {
            first = mid + 1;
        }
        else {
            last = mid;
        }
    }
    return first;
}

static struct record_entry* construct_record_entry(char *tag)
{
    struct record_entry* re;
    int taglen, size;

    taglen = strlen(tag);
    size = record_entry_size(taglen);

    re = (struct record_entry *)malloc(size);
    memset(re, 0, size);
    re->taglen = taglen;
    memcpy(re->tag, tag, taglen);
    return re;
}

int add_entry(struct record_entry *re)
{
    int pos;
    pos = tag_pos(re->tag, re->taglen);

    if (pos < 0)    // equal, don't need to add
        return 0;
    
    /* no active_entry */
    if (!active_entry) {
        active_nr = 0;
        active_alloc = alloc_nr(3);
        active_entry = calloc(active_alloc, sizeof(struct record_entry*));
    }

    if (active_nr == active_alloc) {
        active_alloc = alloc_nr(active_nr);
        active_entry = realloc(active_entry, active_alloc * sizeof(struct record_entry*));
    }

    memmove(active_entry + pos + 1, active_entry + pos, (active_nr - pos) * sizeof(struct record_entry*));

    active_nr++;
    active_entry[pos] = re;
    return 1;
}

int add_entry_by_tag(char *tag)
{
    int res;
    struct record_entry *re;

    re = construct_record_entry(tag);
    if ((res = add_entry(re)) != 1)
        free(re);

    return res;
}

/* pos start from 1 */
void remove_entry_by_id(unsigned int pos) 
{
    if (pos > 0 && pos <= active_nr) {
        memmove(active_entry + pos - 1, active_entry + pos, (active_nr - pos) * sizeof(struct record_entry*));
        active_nr--;
    }
    // return 0;
}

void remove_entry_by_tag(char *tag)
{
    int pos;

    pos = tag_pos(tag, strlen(tag));

    if (pos >= 0)   
        return;

    remove_entry_by_id(-pos);
}

void remove_entry(struct record_entry *re)
{
    remove_entry_by_tag(re->tag);
}

// static void free_active_entry()
// {
//     int i;
//     for (i = 0; i< active_nr; ++i) {
//         free(active_entry[i]);
//     }
//     free(active_entry);
// }

static int cal_records_len()
{
    int i, res = 0;

    res = sizeof(struct record_header);
    for (i = 0; i< active_nr; ++i) {
        res += re_size(active_entry[i]);
    }
    return res;
}


static int init_write_record_map(void)
{
    unsigned int expect_nr;
    struct record_header hdr = {CACHE_SIGNATURE, 1, 0};
    struct record_entry *re;
    int i, size;
    int fd;
    void *map;
    unsigned long offset;
    unsigned long fd_size;
    
    expect_nr = sizeof(tags) / sizeof(char *);
    active_alloc = alloc_nr(expect_nr);
    active_entry = calloc(active_alloc, sizeof(struct record_entry*));

    for (i = 0; i < expect_nr; ++i) {
        add_entry_by_tag(tags[i]);
    }

    hdr.entries = active_nr;

    fd = open(passwdfile, O_RDWR | O_CREAT | O_TRUNC, 0600);
    if (fd < 0) {
        fprintf(stderr, "error: fail open %s\n", passwdfile);
        return -1;
    }
    fd_size = cal_records_len();

    printf("fd_size %ld\n", fd_size);

    if (ftruncate(fd, fd_size) < 0) {
        error("error: ftruncate");
    }

    map = mmap(0, fd_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (map == MAP_FAILED) {
        perror("mmap");
        return -1;
    }

    memcpy(map, &hdr, sizeof(hdr));

    offset = sizeof(hdr);

    for (i = 0; i < active_nr; ++i) {
        re = active_entry[i];
        size = re_size(re);
        memcpy(map + offset, re, size);
        offset += size;
        // free(re);
    }

    munmap(map, fd_size);
    close(fd);

    free(active_entry);
    return 0;
}

static struct stat st;
static char *tmpfilename = "tmp.txt";

int parse_record_from_buffer(char *map)
{
    int i;
    unsigned long size, offset;
    struct record_header* hdr;

    hdr = (struct record_header *)map;

    if (hdr->signature != CACHE_SIGNATURE) {
        perror("bad signature");
        goto unmap;
    }

    if (hdr->version != 1) {
        perror("bad version");
        goto unmap;
    }

    printf("%d %d\n", hdr->version, hdr->entries);

    active_nr = hdr->entries;
    active_alloc = alloc_nr(active_nr);
    active_entry = calloc(active_alloc, sizeof(struct record_entry*));

    offset = sizeof(*hdr);

    for (i = 0; i < active_nr; ++i) {
        struct record_entry *re = (struct record_entry *)(map + offset);
        active_entry[i] = re;
        offset = offset + re_size(re);
        printf("%d %s\n", re->taglen, re->tag);
    }
    return active_nr;

unmap:
    munmap(map, size);
    // return error("parse record failed");
}

void copy_records_to_buffer(unsigned char **buf)
{
    int offset, size;
    int i;
    struct record_entry *re;
    struct record_header hdr = {CACHE_SIGNATURE, 1, active_nr};

    memcpy(*buf, &hdr, sizeof(hdr));
    offset = sizeof(hdr);

    for (i = 0; i < active_nr; ++i) {
        re = active_entry[i];
        size = re_size(re);
        memcpy(*buf + offset, re, size);
        offset += size;
        printf("copy %d %d %s\n", (int)offset, re->taglen, re->tag);
        // free(re);
    }
    // free(active_entry);
}


int read_content_and_decry(char **plaintext)
{
    unsigned char *ciphertext, *decryptedtext;
    int ciphertext_len, decryptedtext_len;
    int fd;
    unsigned long size;
    void *map;

    errno = ENOENT;
    fd = open(passwdfile, O_RDONLY);
    if (fd < 0) 
        return (errno == ENOENT) ? 0 : error("open failed");

    map = MAP_FAILED;
    if (!fstat(fd, &st)) {
        size = st.st_size;
        map = mmap(0, size, PROT_READ, MAP_PRIVATE, fd, 0);
    }
    close(fd);

    if (map == MAP_FAILED)
        return error("mmap failed");

    BIO_dump_fp(stdout, (const char*)map, size);

    decryptedtext = (unsigned char*) malloc(size);

    decryptedtext_len = decry((unsigned char *)map, size, decryptedtext);

    if (decryptedtext_len < 0)
        return DECRY_ERR;

    decryptedtext[decryptedtext_len] = '\0';
    
    *plaintext = decryptedtext;

    BIO_dump_fp(stdout, (const char *)decryptedtext, decryptedtext_len);

    return decryptedtext_len;
}

int read_content(char **plaintext)
{
    unsigned char *record;
    int record_len, ciphertext_len, decryptedtext_len;
    int fd;
    unsigned long size;
    void *map;

    errno = ENOENT;
    fd = open(passwdfile, O_RDONLY);
    if (fd < 0) 
        return (errno == ENOENT) ? 0 : error("open failed");

    map = MAP_FAILED;
    if (!fstat(fd, &st)) {
        size = st.st_size;
        map = mmap(0, size, PROT_READ, MAP_PRIVATE, fd, 0);
    }
    close(fd);

    if (map == MAP_FAILED)
        return error("mmap failed");
    
    *plaintext = (char *)map;

    return size;
}

int read_records_decry(void)
{
    char *record_plain;
    int record_len, entries;

    errno = EBUSY;
    if (active_entry) 
        error("more than one thread");
    
    record_len = read_content_and_decry(&record_plain);
    if (record_len <= 0) {
        if (record_len == 0)
            return 0;
        if (record_len == DECRY_ERR) {
            return DECRY_ERR;
        }
        error("read records file failed");
    }

    entries = parse_record_from_buffer(record_plain);
    if ( entries < 0) 
        error("parse record failed");
    

    return entries;
}

int read_records(void)
{
    char *record_plain;
    int record_len, entries;

    errno = EBUSY;
    if (active_entry)
        return error("more than one thread");

    record_len = read_content(&record_plain);
    if (record_len <= 0) 
        return (record_len == 0)? 0 : error("read record failed");
    
    entries = parse_record_from_buffer(record_plain);
    if ( entries < 0) 
        return error("parse record failed");
    
    return entries;
}

int write_records_encry(void)
{
    unsigned long plain_size, cipher_size;
    unsigned char *plaintext, *ciphertext;
    void *map;
    int fd;

    plain_size = cal_records_len();
    cipher_size = plain_size + 16;

    plaintext = (unsigned char *)malloc(plain_size);
    ciphertext = (unsigned char *)malloc(cipher_size);

    copy_records_to_buffer(&plaintext); 
    if (plaintext == NULL) {
        error("copy_records_to_buffer");
    }
    cipher_size = encry(plaintext, plain_size, ciphertext);

    BIO_dump_fp(stderr, (const char *)plaintext, plain_size);
    BIO_dump_fp(stderr, (const char *)ciphertext, cipher_size);

    fd = open(passwdfile, O_RDWR | O_CREAT | O_TRUNC, 0600);
    if (fd < 0) {
        perror(passwdfile);
        return -1;
    }

    if (ftruncate(fd, cipher_size) < 0) {
        perror("ftruncate");
        return -1;
    }

    map = mmap(0, cipher_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (map == MAP_FAILED) {
        perror("map");
        return -1;
    }
    memcpy(map, (const char *)ciphertext, cipher_size);
    munmap(map, cipher_size);
    close(fd);
    
    return 0;
}

int write_records(void)
{
    unsigned int expect_nr;
    struct record_header hdr = {CACHE_SIGNATURE, 1, 0};
    struct record_entry *re;
    int i,  size;
    int fd;

    hdr.entries = active_nr;

    fd = open(passwdfile, O_WRONLY | O_CREAT | O_TRUNC, 0600);
    if (fd < 0) {
        fprintf(stderr, "error: fail open %s\n", passwdfile);
        return -1;
    }

    if (write(fd, &hdr, sizeof(hdr)) != sizeof(hdr)) {
        error("fail write");
    }

    for (i = 0; i < active_nr; ++i) {
        re = active_entry[i];
        size = re_size(re);
        if (write(fd, re, size) != size) {
            perror("write");
            return -1;
        }
        free(re);
    }
    if (active_entry)
        free(active_entry);
    close(fd);
    return 0;
}



// int main(int argc, char* argv[])
// {
//     // init_write_record_map();
//     unsigned char *key = (unsigned char *)"01234567890123456789012345678901";
//     unsigned char *iv = (unsigned char *)"0123456789012345";
//     int entries;
//     enc_ctx_init(key, iv);

//     // entries = read_records_decry();

//     // if (entries < 0) {
//     //     return error("read_record failed");
//     // }

//     write_records_encry();
//     read_records_decry();


//     cleanup();


//     // write_records();
//     // init_records_encry();

//     return 0;
// }