#include <stdlib.h>
#include <stdio.h>
// I really like bools.
#include <stdbool.h>
#include <unistd.h>

#include "mycdb.h"


struct mycdb_options mycdb_options;


static void print_usage(const char *prog_name) {
    printf("OVERVIEW: %s [OPTIONS] KEY\n\n", prog_name);
    printf("OPTIONS:\n" \
           "-f DB.CDB\tCDB database to open."
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
    struct cdb *db;

    parse_args(argc, argv);

    vprintf("Opening %s …", mycdb_options.filename);
    FILE *fd = fopen(mycdb_options.filename, "rb");

    if (fd == NULL) {
        perror("FATAL: Error opening database file");
        exit(EXIT_FAILURE);
    }

    db = malloc(sizeof(struct cdb));
    mycdb_init(db, *fd);

    free(db);
    fclose(fd);
    return EXIT_SUCCESS;
}
