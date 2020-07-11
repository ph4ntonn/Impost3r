#include <stdio.h>

typedef enum _baseencode_errno {
    SUCCESS = 0,
    INVALID_INPUT = 1,
    EMPTY_STRING = 2,
    INPUT_TOO_BIG = 3,
    INVALID_B32_DATA = 4,
    INVALID_B64_DATA = 5,
    MEMORY_ALLOCATION = 6,
} baseencode_error_t;


char            *base32_encode (const unsigned char *user_data,
                                size_t               data_len,
                                baseencode_error_t  *err);

unsigned char   *base32_decode (const char          *user_data,
                                size_t               data_len,
                                baseencode_error_t  *err);

char            *base64_encode (const unsigned char *input_string,
                                size_t               input_length,
                                baseencode_error_t  *err);

unsigned char   *base64_decode (const char          *input_string,
                                size_t               input_length,
                                baseencode_error_t  *err);