#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>

#include "debug.h"
#include "socket_helper.h"
#include "mycdb.h"
#include "mycdbserver.h"


struct mycdb_options {
    bool verbose;
    const char* filename;
    short int port;
} mycdb_options;


static void print_usage(const char *prog_name) {
    printf("OVERVIEW: %s [OPTIONS]\n\n", prog_name);
    printf("OPTIONS:\n" \
           "-f DB.CDB\tCDB database to open.\n"
           "-p PORT [7171] TCP port to listen on.\n"
           "-h\t\tShow this help screen.\n");
}


static void parse_args(int argc, char* argv[]) {
    char arg;
    char *endptr; // for strtol

    mycdb_options.filename = NULL;
    mycdb_options.port = MYCDB_DEFAULT_PORT;

    while ((arg = getopt(argc, argv, "hf:p:")) != -1) {
        switch (arg) {
            case 'f':
                mycdb_options.filename = optarg;
                break;
            case 'p':
                mycdb_options.port = strtol(optarg, &endptr, 0);
                if (*endptr) {
                    fprintf(stderr, "FATAL: Invalid port specified.\n");
                    goto error;
                }
                break;
            case 'h':
                print_usage(argv[0]);
                exit(EXIT_SUCCESS);
            default:
                fprintf(stderr, "FATAL: Unknown option '%c' specified.\n", arg);
                goto error;
        }
    }

    if (mycdb_options.filename == NULL) {
        fprintf(stderr, "FATAL: No database file specified.\n");
        goto error;
    }

    return;
error:
    exit(EXIT_FAILURE);
}


static struct cdb* open_cdb(int fd) {
    struct cdb *db = NULL;
    db = calloc(1, sizeof(struct cdb));
    check_mem(db);
    mycdb_init(db, fd);

    return db;
error:
    return NULL;
}


static int mycdb_listen_loop(int list_s) {
    int conn_s;
    char buf[255];

    debug("Listening on 0.0.0.0:%d", mycdb_options.port);
    while (1) {
        conn_s = accept(list_s, NULL, NULL);
        // Probably better to continue here.
        check(conn_s > -1, "Error reading socket");

        debug("Accepted connection.");

        socket_readline(conn_s, buf, sizeof(buf) - 1);
        buf[254] = '\0';
        socket_writeline(conn_s, buf, strlen(buf));
    }

    return 0;
error:
    return -1;
}


int main(int argc, char* argv[]) {
    int fd;
    int list_s;
    struct cdb *db = NULL;
    struct sockaddr_in *serveraddr = NULL;

    parse_args(argc, argv);

    debug("Opening %s …", mycdb_options.filename);
    fd = open(mycdb_options.filename, O_RDONLY);
    check(fd, "FATAL: Error opening database file.");
    db = open_cdb(fd);
    check(db != NULL, "Can't initialize internal data structures.");

    // Set up the listening socket.
    list_s = socket(AF_INET, SOCK_STREAM, 0);
    check(list_s != -1, "Error creating listening socket");

    // Set up the options for the listening socket.
    serveraddr = calloc(1, sizeof(struct sockaddr_in));
    check_mem(serveraddr);
    serveraddr->sin_family = AF_INET;
    serveraddr->sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr->sin_port = htons(mycdb_options.port);

    // Bind the socket to the listening address.
    check(bind(list_s, (struct sockaddr *) serveraddr, sizeof(struct sockaddr_in)) == 0,
            "Error binding listening socket");

    check(listen(list_s, MYCDB_LISTENQ) == 0, "Error listening on socket");

    check(mycdb_listen_loop(list_s) == 0, "Error during request/response cycle");

    free(db);
    free(serveraddr);
    return EXIT_SUCCESS;
error:
    if (db != NULL) free(db);
    if (serveraddr != NULL) free(serveraddr);
    return EXIT_FAILURE;
}
