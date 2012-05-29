#ifndef __MYCDB__H
#define __MYCDB__H
#define vprintf(...) if (mycdb_options.verbose) fprintf(stderr, __VA_ARGS__)

extern struct mycdb_options {
    bool verbose;
    const char* filename;
    const char* key;
} mycdb_options;

#endif
