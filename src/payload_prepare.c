//
// Author: Filip Cerny
// Date: 02.12.2023
// Description: 
//

//todo check, test

//Local includes
#include "payload_prepare.h"
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
static bool get_array_from_payload_file(const char *filename, const BoolArray *output_arr);

static uint32_t compute_crc32(const BoolArray data);

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
bool ** prepare_payload_data(const char *filename) {
    // Get payload data from file
    bool *payload_data = get_array_from_payload_file(filename, NULL);
    if (payload_data == NULL) {
        LOG_MESSAGE(ERROR, "Error: Unable to read the entire file.");
        return NULL;
    }

    // Compute CRC32
    uint32_t crc32 = compute_crc32(payload_data);

    // Compress payload data
    size_t compressed_size;
    unsigned short *compressed_payload = compress_binary_data((const unsigned char *) payload_data, sizeof(bool),
                                                              &compressed_size);

    // Calculate the size of the final data (including signature and CRC32)
    size_t final_size = compressed_size + sizeof(SIGNATURE) + sizeof(uint32_t);

    // Allocate memory for the final data
    bool *final_data = TRACKED_MALLOC(final_size);
    if (!final_data) {
        LOG_MESSAGE(ERROR, "Error: Memory allocation failed.");
        free(payload_data);
        free(compressed_payload);
        return NULL;
    }

    //todo not safe check memcpy

    // Add signature at the beginning of the final data
    memcpy(final_data, SIGNATURE, sizeof(SIGNATURE));

    // Add CRC32 after the signature
    memcpy(final_data + sizeof(SIGNATURE), &crc32, sizeof(uint32_t));

    // Add compressed payload data after CRC32
    memcpy(final_data + sizeof(SIGNATURE) + sizeof(uint32_t), compressed_payload, compressed_size);

    // Clean up
    free(payload_data);
    free(compressed_payload);

    return &final_data;
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
static bool get_array_from_payload_file(const char *filename, BoolArray *output_arr) {
    //SANITY CHECK
    if (!filename) {
        return false;
    }

    // Open the file
    FILE *file = fopen(filename, "rb");
    if (!file) {
        LOG_MESSAGE(ERROR, "Error: Unable to open or read file %s.", filename);
        return false;
    }

    // Determine the size of the file
    fseek(file, 0, SEEK_END);
    size_t size = ftell(file);
    rewind(file);

    // Allocate memory for the boolean array
    bool *data = (bool *) TRACKED_MALLOC(size * sizeof(bool));
    if (!data) {
        fclose(file);
        LOG_MESSAGE(ERROR, "Error: Memory allocation failed.");
        return false;
    }

    // Read binary data and convert to boolean values
    size_t bytesRead = fread(data, sizeof(bool), size, file);
    if (bytesRead != size) {
        fclose(file);
        free(data);
        LOG_MESSAGE(ERROR, "Error: Unable to read the entire file.");
        return false;
    }

    *output_arr = TRACKED_MALLOC(sizeof(*output_arr));
    if (*output_arr == NULL) {
        // Memory allocation failed
        // Handle error
    }
    *output_arr = {data, size};


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
static uint32_t compute_crc32(const BoolArray data) {
    //SANITY CHECK
    if (!data.array_ptr) {
        log_message(ERROR, __FILE__, __LINE__, "Error: Invalid array pointer.");
        return 0;
    }

    //check if it 

    uint32_t crc = crc32(0L, Z_NULL, 0);  // Initialize CRC32

    // Iterate through the boolean array and update CRC32
    for (size_t i = 0; i < data.length; ++i) {
        // Convert boolean value to a byte (0 or 1)
        uint8_t byte = data.array_ptr[i] ? 1 : 0;

        // Update CRC32 with the byte
        crc = crc32(crc, &byte, 1);
    }

    return crc;
}
//endregion