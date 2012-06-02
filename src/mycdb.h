#ifndef __MYCDB__H
#define __MYCDB__H

#include <stdbool.h>

// Starting value for the hash as defined in the CDB database structure:
// http://cr.yp.to/cdb/cdb.txt
#define MYCDB_HASH_START 5381
#define vprintf(...) if (mycdb_options.verbose) fprintf(stderr, __VA_ARGS__)

extern struct mycdb_options {
    bool verbose;
    const char* filename;
    char* key;
} mycdb_options;


struct cdb {
    char *map; /* 0 if not initialized, using char to be able to use pointer
                  arithmetic on it. */
    int fd; /* file descriptor */
    size_t size; /* keeps the file size, only if *map is initialized */
    uint32_t loop; /* the numver of hash slots searched with the current key */
    uint32_t haddr; /* address of the target hash */
    uint32_t hslots; /* number of slots in the target hash */
    uint32_t maddr; /* address of the meta data for the current key */
    uint32_t ksize; /* size of the key */
    uint32_t dsize; /* size of the data */
    uint32_t daddr; /* address of the payload data for the current key */
};


void mycdb_init(struct cdb *db, int fd);
void mycdb_free(struct cdb *db);
int mycdb_findnext(struct cdb *db, char *key);
char* mycdb_read(struct cdb *db);
uint32_t mycdb_hash(char *str, uint32_t len);

#endif
