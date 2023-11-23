//
// Author: Lenovo
// Date: 07.11.2023
// Description: 
//

#include "decompress.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DICTIONARY_SIZE 4096
#define INITIAL_DICTIONARY_SIZE 256

typedef struct {
    unsigned short prefix;
    unsigned char suffix;
} DictionaryEntry;

// Function to decompress binary data using LZW algorithm
unsigned char* lzw_decompress_binary(const unsigned short* compressed_data, size_t compressed_size, size_t* decompressed_size) {
    DictionaryEntry dictionary[DICTIONARY_SIZE];
    unsigned char* result = NULL;
    size_t result_capacity = 1024;
    size_t result_size = 0;

    // Initialize the dictionary with single-byte entries
    for (unsigned short i = 0; i < INITIAL_DICTIONARY_SIZE; ++i) {
        dictionary[i].prefix = 0xFFFF;  // Indicates no prefix
        dictionary[i].suffix = (unsigned char)i;
    }

    // Start decompression
    unsigned short currentCode = compressed_data[0];
    result = (unsigned char*)malloc(result_capacity);
    if (!result) {
        fprintf(stderr, "Error: Memory allocation failed.\n");
        return NULL;
    }

    result[result_size++] = (unsigned char)currentCode;

    for (size_t i = 1; i < compressed_size; ++i) {
        unsigned short code = compressed_data[i];

        if (code < DICTIONARY_SIZE) {
            // Output the code as a single character
            result[result_size++] = (unsigned char)code;
        } else {
            if (code == currentCode) {
                // Special case for handling repeating codes
                unsigned char repeatChar = result[result_size - 1];
                result[result_size++] = repeatChar;
            } else {
                // Output the string represented by the code
                unsigned short prefix = code;
                while (prefix >= INITIAL_DICTIONARY_SIZE) {
                    result[result_size++] = dictionary[prefix].suffix;
                    prefix = dictionary[prefix].prefix;
                }

                result[result_size++] = (unsigned char)prefix;

                // Add a new entry to the dictionary
                if (currentCode < DICTIONARY_SIZE) {
                    dictionary[currentCode].prefix = currentCode;
                    dictionary[currentCode].suffix = (unsigned char)prefix;
                    ++currentCode;
                }
            }
        }

        // Resize the result buffer if needed
        if (result_size == result_capacity) {
            result_capacity *= 2;
            result = realloc(result, result_capacity);
            if (!result) {
                fprintf(stderr, "Error: Memory allocation failed.\n");
                free(result);
                return NULL;
            }
        }
    }

    *decompressed_size = result_size;
    return result;
}

int main() {
    // Example compressed data
    unsigned short compressed_data[] = {0x0041, 0x0042, 0x0043, 0x0003, 0x0045, 0x0044, 0x0046};

    // Decompress the data
    size_t decompressed_size;
    unsigned char* decompressed_data = lzw_decompress_binary(compressed_data, sizeof(compressed_data) / sizeof(compressed_data[0]), &decompressed_size);

    // Output the decompressed data (just printing for demonstration)
    printf("Decompressed Data: ");
    for (size_t i = 0; i < decompressed_size; ++i) {
        printf("%02X ", decompressed_data[i]);
    }
    printf("\n");

    // Clean up
    free(decompressed_data);

    return 0;
}
