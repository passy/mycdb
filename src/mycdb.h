#ifndef __MYCDB__H
#define __MYCDB__H

#include <stdbool.h>

#define vprintf(...) if (mycdb_options.verbose) fprintf(stderr, __VA_ARGS__)

extern struct mycdb_options {
    bool verbose;
    const char* filename;
    const char* key;
} mycdb_options;


struct cdb {
    void *map; /* 0 if not initialized */
    FILE fd; /* file descriptor */
    size_t size; /* keeps the file size, only if *map is initialized */
};


void mycdb_init(struct cdb *db, FILE fd);
void mycdb_free(struct cdb *db);


#endif
