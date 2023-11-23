#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DICTIONARY_SIZE 4096

typedef struct {
    unsigned short prefix;
    unsigned char suffix;
} DictionaryEntry;

// Function to compress binary data using LZW algorithm
unsigned short* lzw_compress_binary(const unsigned char* data, size_t size, size_t* compressed_size) {
    DictionaryEntry dictionary[DICTIONARY_SIZE];
    unsigned short* result = NULL;
    size_t result_capacity = 1024;
    size_t result_size = 0;

    // Initialize the dictionary with single-byte entries
    for (unsigned short i = 0; i < 256; ++i) {
        dictionary[i].prefix = 0xFFFF;  // Indicates no prefix
        dictionary[i].suffix = (unsigned char)i;
    }

    // Start compression
    unsigned short currentCode = 256;
    unsigned short currentPrefix = data[0];
    for (size_t i = 1; i < size; ++i) {
        unsigned char currentChar = data[i];
        unsigned short combinedCode = (currentPrefix << 8) | currentChar;

        int found = 0;
        for (unsigned short j = 0; j < currentCode; ++j) {
            if (dictionary[j].prefix == currentPrefix && dictionary[j].suffix == currentChar) {
                found = 1;
                break;
            }
        }

        if (found) {
            currentPrefix = combinedCode;
        } else {
            if (result_size == result_capacity) {
                result_capacity *= 2;
                result = realloc(result, result_capacity * sizeof(unsigned short));
                if (!result) {
                    fprintf(stderr, "Error: Memory allocation failed.\n");
                    free(result);
                    return NULL;
                }
            }

            result[result_size++] = currentPrefix;

            if (currentCode < DICTIONARY_SIZE) {
                dictionary[currentCode].prefix = currentPrefix;
                dictionary[currentCode].suffix = currentChar;
                ++currentCode;
            }

            currentPrefix = currentChar;
        }
    }

    if (result_size == result_capacity) {
        result_capacity += 1;
        result = realloc(result, result_capacity * sizeof(unsigned short));
        if (!result) {
            fprintf(stderr, "Error: Memory allocation failed.\n");
            free(result);
            return NULL;
        }
    }

    result[result_size++] = currentPrefix;

    *compressed_size = result_size;
    return result;
}

int main() {
    // Example binary data
    unsigned char binary_data[] = {0x41, 0x42, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46};

    // Compress the binary data
    size_t compressed_size;
    unsigned short* compressed_data = lzw_compress_binary(binary_data, sizeof(binary_data), &compressed_size);

    // Output the compressed data (just printing for demonstration)
    printf("Compressed Data: ");
    for (size_t i = 0; i < compressed_size; ++i) {
        printf("%04X ", compressed_data[i]);
    }
    printf("\n");

    // Clean up
    free(compressed_data);

    return 0;
}
