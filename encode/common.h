#include "encode.h"
#include <stdio.h>

#define BITS_PER_BYTE               8
#define BITS_PER_B32_BLOCK          5
#define BITS_PER_B64_BLOCK          6

// 64 MB should be more than enough
#define MAX_ENCODE_INPUT_LEN        64*1024*1024
// if 64 MB of data is encoded than it should be also possible to decode it. That's why a bigger input is allowed for decoding
#define MAX_DECODE_BASE32_INPUT_LEN ((MAX_ENCODE_INPUT_LEN * 8 + 4) / 5)
#define MAX_DECODE_BASE64_INPUT_LEN ((MAX_ENCODE_INPUT_LEN * 8 + 4) / 6)


static int
strip_char(char *str, char strip)
{
    int found = 0;
    char *p, *q;
    for (q = p = str; *p; p++) {
        if (*p != strip) {
            *q++ = *p;
        } else {
            found++;
        }
    }
    *q = '\0';
    return found;
}


static void
check_input(const unsigned char *user_data, size_t data_len, int max_len, baseencode_error_t *err)
{
    if (user_data == NULL || (data_len == 0 && user_data[0] != '\0')) {
        *err = INVALID_INPUT;
        return;
    } else if (user_data[0] == '\0') {
        *err = EMPTY_STRING;
        return;
    }

    if (data_len > max_len) {
        *err = INPUT_TOO_BIG;
        return;
    }

    *err = SUCCESS;
}