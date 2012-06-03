#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
// I really like bools.
#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>

#include "debug.h"
#include "mycdb.h"


struct mycdb_options {
    bool verbose;
    const char* filename;
    char* key;
} mycdb_options;


static void print_usage(const char *prog_name) {
    printf("OVERVIEW: %s [OPTIONS] KEY\n\n", prog_name);
    printf("OPTIONS:\n" \
           "-f DB.CDB\tCDB database to open.\n"
           "-v\t\tBe verbose.\n"
           "-h\t\tShow this help screen.\n");
}


static void parse_args(int argc, char* argv[]) {
    char arg;

    while ((arg = getopt(argc, argv, "hvf:")) != -1) {
        switch (arg) {
            case 'v':
                mycdb_options.verbose = true;
                break;
            case 'f':
                mycdb_options.filename = optarg;
                break;
            case 'h':
                print_usage(argv[0]);
                exit(EXIT_SUCCESS);
            default:
                fprintf(stderr, "Unknown option '%c' specified.\n", arg);
                exit(EXIT_FAILURE);
        }
    }

    // optind is incremented by getopt on every successful call
    if (optind >= argc) {
        fprintf(stderr, "FATAL: Expected KEY to come after options.\n\n");
        print_usage(argv[0]);
        exit(EXIT_FAILURE);
    }

    mycdb_options.key = argv[optind];
}


int main(int argc, char* argv[]) {
    struct cdb *db = NULL;
    char *result = NULL;
    int fd = 0;

    parse_args(argc, argv);

    debug("Opening %s …", mycdb_options.filename);
    fd = open(mycdb_options.filename, O_RDONLY);
    check(fd, "FATAL: Error opening database file.");

    db = calloc(1, sizeof(struct cdb));
    check_mem(db);
    mycdb_init(db, fd);
    check(mycdb_findnext(db, mycdb_options.key) == 0, "Didn't find key.");
    result = mycdb_read(db);
    check(result != NULL, "Couldn't read payload.");

    printf("%s", result);

    free(result);
    free(db);
    close(fd);
    return EXIT_SUCCESS;
error:
    if (db != NULL) free(db);
    if (result != NULL) free(result);
    if (fd != 0) close(fd);
    return EXIT_FAILURE;
}
