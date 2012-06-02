#include <stdint.h>
#include "unpack.h"
#include "debug.h"

/**
 * Unpacks a 4-byte little-endian string in a uint32_t.
 */
void unpack_uint32_t(const char in[4], uint32_t *out) {
    for (int i = 3; i >= 0; i--) {
        *out += (unsigned char) in[i];
        if (i != 0) *out <<= 8;
    }
}

