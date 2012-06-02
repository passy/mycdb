#include <sys/mman.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <inttypes.h>

#include "mycdb.h"
#include "debug.h"
#include "unpack.h"

#define BUFFER_SIZE 8

void mycdb_free(struct cdb *db) {
    if (db->map) {
        munmap(db->map, db->size);
        db->map = 0;
    }
}


void mycdb_init(struct cdb *db, int fd) {
    struct stat stat;
    mycdb_free(db);

    check(fstat(fd, &stat) == 0, "Could not stat database file");

    db->fd = fd;
    db->size = stat.st_size;

    db->map = mmap(0, stat.st_size, PROT_READ, MAP_SHARED, db->fd, 0);
    check_fatal(db->map != MAP_FAILED, "mmap failed");

    return;
error:
    exit(EXIT_FAILURE);
}


int mycdb_read(struct cdb *db, char *buf, unsigned int len, uint32_t pos) {
    // Check if the requested range is readable.
    check((pos < db->size) && (len < db->size - pos),
            "Invalid position requested.");
    debug("Copying %u from %p", len, db->map + pos);
    memcpy(buf, (db->map + pos), len);
    return 0;
error:
    return -1;
}


static void mycdb_initsearch(struct cdb *db, char *key) {
    uint32_t keylen = strlen(key);
    char buf[BUFFER_SIZE];

    uint32_t hash = mycdb_hash(key, keylen);
    // Read tuple of address and bucket size into the buffer, starting at
    // (hash % 256) * (8bit/bucket), which can be optimized as
    // (hash * 2**3) % 2048 where the first term equals a left-shift by 3 and
    // as 2048 == 2^11 it can be replaced by a bitmask of (2048-1).
    check_fatal(mycdb_read(db, buf, BUFFER_SIZE, (hash << 3) & 2047) == 0,
            "Couldn't read.");
    // Read as chunks of 4 bytes each and unpack into the struct.
    unpack_uint32_t(buf, &db->haddr);
    unpack_uint32_t(buf + 4, &db->hslots);

    debug("Address: %"PRIu32, db->haddr);
    debug("Size: %"PRIu32, db->hslots);
}

/**
 * Find the next occurence of the given key within the cdb instance and set the
 * appropriate cursors.
 */
int mycdb_findnext(struct cdb *db, char *key) {

    if (!db->loop) {
        mycdb_initsearch(db, key);
    }

    // TODO
    return 0;
}


/**
 * The hash algorithm specified for the CDB file format:
 * http://cr.yp.to/cdb/cdb.txt
 */
uint32_t mycdb_hash(char *str, uint32_t len) {
    uint32_t h = MYCDB_HASH_START;

    while (len--) {
        h = ((h << 5) + h) ^ *(str++);
    }

    return h;
}
