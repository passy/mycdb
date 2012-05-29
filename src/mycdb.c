#include <sys/mman.h>
#include <stdio.h>

#include "mycdb.h"

void mycdb_free(struct cdb *db) {
    if (db->map) {
        munmap(db->map, db->size);
        db->map = 0;
    }
}


void mycdb_init(struct cdb *db, FILE fd) {
    mycdb_free(db);

    db->fd = fd;
}
