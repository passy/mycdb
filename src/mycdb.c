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


int mycdb_read_mem(struct cdb *db, char *buf, unsigned int len, uint32_t pos) {
    // Check if the requested range is readable.
    check((pos < db->size) && (len < db->size - pos),
            "Invalid position requested.");
    debug("Copying %u from %p", len, db->map + pos);
    memcpy(buf, (db->map + pos), len);
    return 0;
error:
    return -1;
}


static void mycdb_initsearch(struct cdb *db, uint32_t hash) {
    char buf[BUFFER_SIZE];

    // Read tuple of address and bucket size into the buffer, starting at
    // (hash % 256) * (8bit/bucket), which can be optimized as
    // (hash * 2**3) % 2048 where the first term equals a left-shift by 3 and
    // as 2048 == 2^11 it can be replaced by a bitmask of (2048-1).
    check_fatal(mycdb_read_mem(db, buf, BUFFER_SIZE, (hash << 3) & 2047) == 0,
            "Couldn't read.");
    // Read as chunks of 4 bytes each and unpack into the struct.
    unpack_uint32_t(buf, &db->haddr);
    unpack_uint32_t(buf + (BUFFER_SIZE / 2), &db->hslots);

    debug("Address: %"PRIu32, db->haddr);
    debug("Size: %"PRIu32, db->hslots);
}


static int mycdb_find_data(struct cdb *db, uint32_t hash) {
    char buf[BUFFER_SIZE];
    uint32_t verify_hash = 0;

    check(mycdb_read_mem(db, buf, BUFFER_SIZE, db->haddr) == 0,
            "Couldn't read hash meta data.");

    unpack_uint32_t(buf, &verify_hash);
    debug("hash: %"PRIu32"; verify_hash: %"PRIu32, hash, verify_hash);
    // As pure values, we should be able to compare them without memcmp, right?
    check(hash == verify_hash, "Unexpected hash found.");

    unpack_uint32_t(buf + 4, &db->maddr);
    return 0;
error:
    return -1;
}


static int mycdb_read_data(struct cdb *db) {
    char buf[BUFFER_SIZE];
    check(mycdb_read_mem(db, buf, BUFFER_SIZE, db->maddr) == 0,
            "Couldn't read hash data.");
    unpack_uint32_t(buf, &db->ksize);
    unpack_uint32_t(buf + 4, &db->dsize);
    return 0;
error:
    return -1;
}


/**
 * Find the next occurence of the given key within the cdb instance and set the
 * appropriate cursors.
 */
int mycdb_findnext(struct cdb *db, char *key) {
    // Calculate the hash based on the key.
    uint32_t keylen = strlen(key);
    uint32_t hash = mycdb_hash(key, keylen);

    if (!db->loop) {
        mycdb_initsearch(db, hash);
    }

    check(db->hslots != 0, "Key not found.");
    // Verify that the given haddr contains the requested hash. Also sets the
    // data position.
    check(mycdb_find_data(db, hash) == 0, "Hash doesn't match.");
    debug("maddr: %"PRIu32, db->maddr);
    check(mycdb_read_data(db) == 0, "Couldn't read data.");
    debug("ksize: %"PRIu32, db->ksize);
    debug("dsize: %"PRIu32, db->dsize);

    return 0;
error:
    return -1;
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

/**
 * Write the data for the current key into ``out``. Make sure to run
 * mycdb_findnext() first. Otherwise the behavior of this function is undefined.
 *
 * Memory for the string is obtained via malloc and must be free()'d.
 */
char* mycdb_read(struct cdb *db) {
    const uint32_t pos = db->maddr + 8 + db->ksize;
    char *out = malloc(db->dsize);
    check_mem(out);

    debug("Reading %d bytes payload data from %"PRIu32, db->dsize, pos);

    memcpy(out, db->map + pos, db->dsize);

    return out;
error:
    return NULL;
}
