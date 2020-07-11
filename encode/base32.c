#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "./common.h"


static int is_valid_b32_input(const char *user_data, size_t data_len);

static int get_char_index(unsigned char c);

static const unsigned char b32_alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ234567";


char *
base32_encode(const unsigned char *user_data, size_t data_len, baseencode_error_t *err)
{
    baseencode_error_t error;
    check_input(user_data, data_len, MAX_ENCODE_INPUT_LEN, &error);
    if (error != SUCCESS) {
        *err = error;
        if (error == EMPTY_STRING) {
            return strdup("");
        } else {
            return NULL;
        }
    }

    size_t user_data_chars = 0, total_bits = 0;
    int num_of_equals = 0;
    for (int i = 0; i < data_len; i++) {
        // As it's not known whether data_len is with or without the +1 for the null byte, a manual check is required.
        if (user_data[i] != '\0') {
            total_bits += 8;
            user_data_chars += 1;
        } else {
            break;
        }
    }
    switch (total_bits % 40) {
        case 8:
            num_of_equals = 6;
            break;
        case 16:
            num_of_equals = 4;
            break;
        case 24:
            num_of_equals = 3;
            break;
        case 32:
            num_of_equals = 1;
            break;
        default:
            break;
    }

    size_t output_length = (user_data_chars * 8 + 4) / 5;
    char *encoded_data = calloc(output_length + num_of_equals + 1, 1);
    if (encoded_data == NULL) {
        *err = MEMORY_ALLOCATION;
        return NULL;
    }

    uint64_t first_octet, second_octet, third_octet, fourth_octet, fifth_octet;
    uint64_t quintuple;
    for (int i = 0, j = 0; i < user_data_chars;) {
        first_octet = i < user_data_chars ? user_data[i++] : 0;
        second_octet = i < user_data_chars ? user_data[i++] : 0;
        third_octet = i < user_data_chars ? user_data[i++] : 0;
        fourth_octet = i < user_data_chars ? user_data[i++] : 0;
        fifth_octet = i < user_data_chars ? user_data[i++] : 0;
        quintuple =
                ((first_octet >> 3) << 35) +
                ((((first_octet & 0x7) << 2) | (second_octet >> 6)) << 30) +
                (((second_octet & 0x3F) >> 1) << 25) +
                ((((second_octet & 0x01) << 4) | (third_octet >> 4)) << 20) +
                ((((third_octet & 0xF) << 1) | (fourth_octet >> 7)) << 15) +
                (((fourth_octet & 0x7F) >> 2) << 10) +
                ((((fourth_octet & 0x3) << 3) | (fifth_octet >> 5)) << 5) +
                (fifth_octet & 0x1F);

        encoded_data[j++] = b32_alphabet[(quintuple >> 35) & 0x1F];
        encoded_data[j++] = b32_alphabet[(quintuple >> 30) & 0x1F];
        encoded_data[j++] = b32_alphabet[(quintuple >> 25) & 0x1F];
        encoded_data[j++] = b32_alphabet[(quintuple >> 20) & 0x1F];
        encoded_data[j++] = b32_alphabet[(quintuple >> 15) & 0x1F];
        encoded_data[j++] = b32_alphabet[(quintuple >> 10) & 0x1F];
        encoded_data[j++] = b32_alphabet[(quintuple >> 5) & 0x1F];
        encoded_data[j++] = b32_alphabet[(quintuple >> 0) & 0x1F];
    }

    for (int i = 0; i < num_of_equals; i++) {
        encoded_data[output_length + i] = '=';
    }
    encoded_data[output_length + num_of_equals] = '\0';

    *err = SUCCESS;
    return encoded_data;
}


unsigned char *
base32_decode(const char *user_data_untrimmed, size_t data_len, baseencode_error_t *err)
{
    baseencode_error_t error;
    check_input((unsigned char *)user_data_untrimmed, data_len, MAX_DECODE_BASE32_INPUT_LEN, &error);
    if (error != SUCCESS) {
        *err = error;
        if (error == EMPTY_STRING) {
            return (unsigned char *) strdup("");
        } else {
            return NULL;
        }
    }

    char *user_data = strdup(user_data_untrimmed);
    data_len -= strip_char(user_data, ' ');

    if (!is_valid_b32_input(user_data, data_len)) {
        *err = INVALID_B32_DATA;
        free(user_data);
        return NULL;
    }

    size_t user_data_chars = 0;
    for (int i = 0; i < data_len; i++) {
        // As it's not known whether data_len is with or without the +1 for the null byte, a manual check is required.
        if (user_data[i] != '=' && user_data[i] != '\0') {
            user_data_chars += 1;
        }
    }

    size_t output_length = (size_t) ((user_data_chars + 1.6 - 1) / 1.6);  // round up
    unsigned char *decoded_data = calloc(output_length + 1, 1);
    if (decoded_data == NULL) {
        *err = MEMORY_ALLOCATION;
        free(user_data);
        return NULL;
    }

    uint8_t mask = 0, current_byte = 0;
    int bits_left = 8;
    for (int i = 0, j = 0; i < user_data_chars; i++) {
        int char_index = get_char_index((unsigned char)user_data[i]);
        if (bits_left > BITS_PER_B32_BLOCK) {
            mask = (uint8_t) char_index << (bits_left - BITS_PER_B32_BLOCK);
            current_byte = (uint8_t) (current_byte | mask);
            bits_left -= BITS_PER_B32_BLOCK;
        } else {
            mask = (uint8_t) char_index >> (BITS_PER_B32_BLOCK - bits_left);
            current_byte = (uint8_t) (current_byte | mask);
            decoded_data[j++] = current_byte;
            current_byte = (uint8_t) (char_index << (BITS_PER_BYTE - BITS_PER_B32_BLOCK + bits_left));
            bits_left += BITS_PER_BYTE - BITS_PER_B32_BLOCK;
        }
    }
    decoded_data[output_length] = '\0';

    free(user_data);

    *err = SUCCESS;
    return decoded_data;
}


static int
is_valid_b32_input(const char *user_data, size_t data_len)
{
    size_t found = 0, b32_alphabet_len = sizeof(b32_alphabet);
    for (int i = 0; i < data_len; i++) {
        if (user_data[i] == '\0') {
            found++;
            break;
        }
        for(int j = 0; j < b32_alphabet_len; j++) {
            if(user_data[i] == b32_alphabet[j] || user_data[i] == '=') {
                found++;
                break;
            }
        }
    }
    if (found != data_len) {
        return 0;
    } else {
        return 1;
    }
}


static int
get_char_index(unsigned char c)
{
    for (int i = 0; i < sizeof(b32_alphabet); i++) {
        if (b32_alphabet[i] == c) {
            return i;
        }
    }
    return -1;
}