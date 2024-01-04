//
// Author: Filip Cerny
// Date: 02.12.2023
// Description: 
//

//todo check, test

//Local includes
#include "payload.h"
#include "utils/utils.h"
#include "compression.h"

//Lib includes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <zlib.h>

#define SIGNATURE "KIVPCSP_FilipCerny_hidden_payload_data"


//region FUNCTIONS DECLARATION
static bool get_array_from_payload_file(const char *filename, PayloadArray *ptr_output);

static uLong compute_crc32(PayloadArray data);

//endregion

//region FUNCTIONS DEFINITIONS

/**
 * Function to prepare payload data it compresses the data and calculates the CRC32
 * and signs the data
 * @param filename is the path to the file
 * @param final_size is the size of the final data
 * @return  FREE MEMORY - the pointer to the final data
 *         NULL if there was an error
 */
bool prepare_payload_data(const char *filename, PayloadArray *output) {
    bool is_success;
    //todo rewrite it to unsigned char array

    // Get payload data from file
    PayloadArray payload_data;
    is_success = get_array_from_payload_file(filename, &payload_data);
    if (!is_success) {
        LOG_MESSAGE(ERROR, "Error: Unable to read the entire file.");
        return false;
    }


    // Compress payload data
    PayloadArray compressed_payload;
    is_success = compress_payload(payload_data, &compressed_payload);
    if (!is_success) {
        LOG_MESSAGE(ERROR, "Error: Unable to compress payload data.");
        return NULL;
    }

    char *signature = SIGNATURE;
    size_t signature_size_bytes = strlen(signature) * sizeof(char);

    size_t compressed_payload_size_bytes = compressed_payload.length * sizeof(PayloadType);

    // Calculate the size of the final data (including signature and CRC32)
    size_t final_size_bytes =
            compressed_payload_size_bytes + signature_size_bytes + sizeof(uLong);

    // Allocate memory for the final data
    output->array = TRACKED_MALLOC(final_size_bytes);
    if (!output->array) {
        LOG_MESSAGE(ERROR, "Error: Memory allocation failed.");
        TRACKED_FREE(output->array);
        TRACKED_FREE(payload_data.array);
        return false;
    }

    // Add signature at the beginning of the final data
    memcpy(output->array, signature, signature_size_bytes);

    // Compute CRC32
    uLong crc32 = compute_crc32(compressed_payload);

    // Add CRC32 after the signature
    memcpy(output->array + signature_size_bytes, &crc32, sizeof(uLong));

    // Add compressed payload data after CRC32
    memcpy(output->array + signature_size_bytes + sizeof(uLong), compressed_payload.array, compressed_payload_size_bytes);

    // Clean up
    TRACKED_FREE(payload_data.array);
    TRACKED_FREE(compressed_payload.array);

    return true;
}

/**
 * Function to extract payload data from the final data
 * @param hidden_data is the final data
 * @param output is the pointer to the output array
 * @return
 * 0 success
 * 4 there is no signature
 * 5 file was corupted, crc32 doesnt match, cannot decompress
 * 6 other error
 */
int extract_payload_from_data(PayloadArray hidden_data, PayloadArray *output) {

    //check if signature correct
    char *original_signature = SIGNATURE;
    size_t signature_size_bytes = strlen(original_signature) * sizeof(char);
    char *signature = TRACKED_MALLOC(signature_size_bytes);
    if (!signature) {
        LOG_MESSAGE(ERROR, "Error: Memory allocation failed.");
        TRACKED_FREE(signature);
        return 6;
    }
    memcpy(signature, hidden_data.array, signature_size_bytes);

    if (strcmp(signature, original_signature) != 0) {
        LOG_MESSAGE(ERROR, "Error: There is no signature.");
        TRACKED_FREE(signature);
        return 4;
    }


    //Save payload to payload_data
    PayloadArray payload_data;

    payload_data.length = hidden_data.length - signature_size_bytes - sizeof(uLong);
    payload_data.array = TRACKED_MALLOC(payload_data.length * sizeof(PayloadType));
    if (!payload_data.array){
        LOG_MESSAGE(ERROR, "Error: Memory allocation failed.");
        TRACKED_FREE(payload_data.array);
        TRACKED_FREE(signature);
        return 6;
    }

    memcpy(payload_data.array, hidden_data.array + signature_size_bytes + sizeof(uLong), payload_data.length * sizeof(PayloadType));

    //check if crc32 is correct
    uLong computed_crc32 = compute_crc32(payload_data);

    uLong loaded_crc32;
    memcpy(&loaded_crc32, hidden_data.array + signature_size_bytes, sizeof(uLong));

    if (computed_crc32 != loaded_crc32) {
        LOG_MESSAGE(ERROR, "Error: File was corrupted, CRC32 doesnt match.");
        TRACKED_FREE(payload_data.array);
        TRACKED_FREE(signature);
        return 5;
    }


    //Decompress payload_data
    bool is_success = decompress_payload(payload_data, output);
    if (!is_success) {
        LOG_MESSAGE(ERROR, "Error: Unable to decompress payload data.");
        TRACKED_FREE(payload_data.array);
        return 5;
    }

    return 0;
}

/**
 * CHECKED
 *
 * Function to read binary data from a file and return an array of boolean values
 * @param filename  is the path to the file
 * @return
 *  a pointer to the boolean array
 *  NULL if there was an error
 */
static bool get_array_from_payload_file(const char *filename, PayloadArray *ptr_output) {
    //SANITY CHECK
    if (!filename) {
        return false;
    }

    // Open the file
    FILE *file = fopen(filename, "rb");
    if (!file) {
        LOG_MESSAGE(ERROR, "Error: Unable to open or read file %s", filename);
        return false;
    }

    // Determine the size of the file
    fseek(file, 0, SEEK_END);
    size_t file_size_bytes = ftell(file);
    rewind(file);

    // Allocate memory for the output array
    ptr_output->length = (size_t) ((double) file_size_bytes * (1 / (double) sizeof(PayloadType)));
    ptr_output->array = TRACKED_MALLOC(ptr_output->length * sizeof(PayloadType));
    if (!ptr_output->array) {
        TRACKED_FREE(ptr_output->array);
        fclose(file);
        LOG_MESSAGE(ERROR, "Error: Memory allocation failed.");
        return false;
    }

    // Read binary data and convert to boolean values
    size_t bytes_read = fread(ptr_output->array, sizeof(PayloadType), ptr_output->length, file);
    if (bytes_read != file_size_bytes) {
        fclose(file);
        TRACKED_FREE(ptr_output->array);
        LOG_MESSAGE(ERROR, "Error: Unable to read the entire file.");
        return false;
    }


    // Close the file
    fclose(file);

    return true;
}

/**
 * Function to compute the CRC32 of a boolean array
 * @param data  is the boolean array
 * @return  the CRC32
 * 0 if there was an error
 */
static uLong compute_crc32(const PayloadArray data) {
    //SANITY CHECK
    if (!data.array) {
        log_message(ERROR, __FILE__, __LINE__, "Error: Invalid array pointer.");
        return 0;
    }

    //check if it 

    uLong crc = crc32(0L, Z_NULL, 0);  // Initialize CRC32

    // Iterate through the boolean array and update CRC32
    for (size_t i = 0; i < data.length; ++i) {
        // Convert boolean value to a byte (0 or 1)
        PayloadType element = data.array[i];

        // Update CRC32 with the byte
        crc = crc32(crc, &element, sizeof(PayloadType));
    }

    return crc;
}
//endregion