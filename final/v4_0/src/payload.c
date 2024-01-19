//
// Author: Filip Cerny
// Date: 02.12.2023
// Description: File contains functions for working with payload data.
//

//Local includes
#include "payload.h"
#include "utils/utils.h"
#include "compression.h"

//Lib includes
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <zlib.h>



/**
 * Max value of the payload size
 */
#define PAYLOADSIZE_MAXVALUE UINT32_MAX

/**
 * Size of the CRC32 in bytes
 */
#define CRC32_SIZE sizeof(crc32_type)

/**
 * Max value of the CRC32
 */
#define CRC32_MAX_VALUE UINT32_MAX

//region STRUCTS
/**
 * Represents type for crc32
 */
typedef uint32_t crc32_type;

//endregion


//region FUNCTIONS DECLARATION
/**
 * Function to read binary data from a file and return an array of byte values
 * @param filename  is the path to the file
 * @param ptr_return_output  is the pointer to the output array
 * @return
 * true if the function was successful
 * false if there was an error
 */
static bool
get_array_from_payload_file(const char *filename, struct binarydataarray *ptr_return_output);

/**
 * Function to compute the CRC32 of a byte array
 * @param data  is the byte array
 * @return  the CRC32
 */
static bool compute_crc32(struct binarydataarray data, crc32_type *ptr_return_result);

//endregion

//region FUNCTIONS DEFINITIONS

//region PUBLIC FUNCTIONS


/**
 * Function copies payload size from payload array to ptr_return_size
 * @param array  payload array
 * @param ptr_return_size  pointer to size_t where the payload size will be copied
 * @return
 * 0 success
 * 4 there is no signature
 * 6 other error
 */
int payload_get_payloadsize(struct binarydataarray *array, size_t *ptr_return_size) {
    if (!array->array || !ptr_return_size) {
        LOG_MESSAGE(ERROR, "Invalid arguments.");
        return 6;
    }
    //Prepare ptr_return
    payloadsize_type payloadsize = 0;

    size_t payloadsize_size = PAYLOADSIZE_SIZE;
    size_t signature_size_bytes = SIGNATURE_SIZE;

    size_t size = payloadsize_size + signature_size_bytes;

    void * start = (void *) array->array;

    if (array->length < size) {
        LOG_MESSAGE(ERROR, "Payload is too small.");
        return 6;
    }
    memcpy(&payloadsize, start, payloadsize_size);

    //check if signature correct
    char *original_signature = SIGNATURE;

    //READ signature
    char *signature = TRACKED_MALLOC(STR_ADD_ONE(signature_size_bytes));
    if (!signature) {
        LOG_MESSAGE(ERROR, "Memory allocation failed.");
        return 6;
    }
    memcpy(signature, start + payloadsize_size, signature_size_bytes);

    //add null terminator
    char null_terminator = STRING_NULL_TERMINATOR;
    memcpy(signature + signature_size_bytes, &null_terminator, sizeof(char));

    //compare signatures
    if (strcmp(signature, original_signature) != 0) {
        LOG_MESSAGE(ERROR, "There is no signature.");
        TRACKED_FREE(signature);
        return 4;
    }

    TRACKED_FREE(signature);
    (*ptr_return_size) = payloadsize;

    return 0;
}

/**
 * Function to prepare payload data it compresses the data and calculates the CRC32
 * and signs the data
 * @param filename is the path to the file
 * @param final_size is the size of the final data
 * @return  FREE MEMORY - the pointer to the final data
 *         NULL if there was an error
 */
bool prepare_payload_data(const char *filename, struct binarydataarray *ptr_return_output) {
    //Check arguments
    if (!filename || !ptr_return_output) {
        LOG_MESSAGE(ERROR, "Wrong arguments.");
        return false;
    }
    //Check if file exists
    if (!file_exists(filename)) {
        LOG_MESSAGE(ERROR, "File does not exist.");
        return false;
    }

    //Prepare ptr_hidden_data
    ptr_return_output->length = 0;
    ptr_return_output->capacity = 0;
    ptr_return_output->array = NULL;

    bool is_success;
    struct cleanupcommand *cleanup_list = NULL;

    // Get payload data from file
    struct binarydataarray payload_data;
    is_success = get_array_from_payload_file(filename, &payload_data);
    if (!is_success) {
        LOG_MESSAGE(ERROR, "Unable to read the entire file.");
        return false;
    }


    LOG_MESSAGE(WARNING, "Payload data size: %zu bytes", payload_data.length * sizeof(binarydata_type));
    // Compress payload data
    struct binarydataarray compressed_payload;
    is_success = compress_payload(payload_data, &compressed_payload);
    if (!is_success) {
        LOG_MESSAGE(ERROR, "Unable to compress payload data.");
        TRACKED_FREE(payload_data.array);
        cleanup_run_commands(&cleanup_list);
        return NULL;
    }
    CLEANUP_ADD_COMMAND(&cleanup_list, compressed_payload.array);
    LOG_MESSAGE(WARNING, "Compressed payload data size: %zu bytes", compressed_payload.length * sizeof(binarydata_type));

    char *signature = SIGNATURE;
    size_t signature_size_bytes = SIGNATURE_SIZE;

    size_t compressed_payload_size_bytes = compressed_payload.length * sizeof(binarydata_type);

    size_t crc_size_bytes = CRC32_SIZE;

    size_t payloadsize_size = PAYLOADSIZE_SIZE;

    // Calculate the size of the final data (including signature and CRC32)
    size_t final_size_bytes =
            payloadsize_size + signature_size_bytes + crc_size_bytes + compressed_payload_size_bytes;

    payloadsize_type payloadsize_value = final_size_bytes;
    if (final_size_bytes > PAYLOADSIZE_MAXVALUE){
        LOG_MESSAGE(ERROR, "Payload data is too big.");
        cleanup_run_commands(&cleanup_list);
        TRACKED_FREE(payload_data.array);
        return false;
    }

    // Compute CRC32
    crc32_type crc32;
    is_success = compute_crc32(compressed_payload, &crc32);
    if (!is_success) {
        LOG_MESSAGE(ERROR, "Unable to compute CRC32.");
        cleanup_run_commands(&cleanup_list);
        TRACKED_FREE(payload_data.array);
        return false;
    }

    // Allocate memory for the final data
    ptr_return_output->length = 0;
    ptr_return_output->capacity = (size_t) ((double) final_size_bytes / (double) sizeof(binarydata_type));
    ptr_return_output->array = TRACKED_MALLOC(ptr_return_output->capacity * sizeof(binarydata_type));
    if (!ptr_return_output->array) {
        LOG_MESSAGE(ERROR, "Memory allocation failed.");
        cleanup_run_commands(&cleanup_list);
        TRACKED_FREE(payload_data.array);
        return false;
    }

    void * start = (void *) ptr_return_output->array;
    size_t offset = 0;

    //Add payload size at the beginning of the final data
    memcpy(start + offset, &payloadsize_value, payloadsize_size);
    ptr_return_output->length += payloadsize_size / sizeof(binarydata_type);
    offset += payloadsize_size;

    // Add signature at the beginning of the final data
    memcpy(start + offset, signature, signature_size_bytes);
    ptr_return_output->length += signature_size_bytes / sizeof(binarydata_type);
    offset += signature_size_bytes;

    // Add CRC32 after the signature
    memcpy(start + offset, &crc32, crc_size_bytes);
    ptr_return_output->length += crc_size_bytes / sizeof(binarydata_type);
    offset += crc_size_bytes;

    // Add compressed payload data after CRC32
    memcpy(start + offset,
           compressed_payload.array, compressed_payload_size_bytes);
    ptr_return_output->length += compressed_payload_size_bytes / sizeof(binarydata_type);
    offset += compressed_payload_size_bytes;

    // Clean up
    cleanup_run_commands(&cleanup_list);
    TRACKED_FREE(payload_data.array);

    return true;
}

/**
 * Function to extract payload data from the final data
 * @param hidden_data is the final data
 * @param ptr_return_payload is the pointer to the output array
 * @return
 * 0 success
 * 4 there is no signature
 * 5 file was corupted, crc32 doesnt match, cannot decompress
 * 6 other error
 */
int extract_payload_from_data(struct binarydataarray *hidden_data, struct dicvaluearray *ptr_return_payload) {
    //SANITY CHECK
    if (!hidden_data->array || !ptr_return_payload) {
        LOG_MESSAGE(ERROR, "Wrong arguments.");
        return 6;
    }

    struct cleanupcommand *cleanup_list = NULL;
    bool is_success;

    void * start = (void *) hidden_data->array;
    size_t offset = 0;

    //READ size
    payloadsize_type payloadsize_value;
    size_t payloadsize_size = PAYLOADSIZE_SIZE;
    memcpy(&payloadsize_value, start + offset, payloadsize_size);
    offset += payloadsize_size;
    if (payloadsize_value != hidden_data->length * sizeof(binarydata_type)){
        LOG_MESSAGE(ERROR, "Payload size is not correct.");
        return 6;
    }


    //check if signature correct
    char *original_signature = SIGNATURE;

    //READ signature
    size_t signature_size_bytes = SIGNATURE_SIZE;
    char *signature = TRACKED_MALLOC(STR_ADD_ONE(signature_size_bytes));
    if (!signature) {
        LOG_MESSAGE(ERROR, "Memory allocation failed.");
        return 6;
    }
    CLEANUP_ADD_COMMAND(&cleanup_list, signature);
    memcpy(signature, start + offset, signature_size_bytes);
    offset += signature_size_bytes;

    //add null terminator
    char null_terminator = STRING_NULL_TERMINATOR;
    memcpy(signature + signature_size_bytes, &null_terminator, sizeof(char));

    //compare signatures
    if (strcmp(signature, original_signature) != 0) {
        LOG_MESSAGE(ERROR, "There is no signature.");
        cleanup_run_commands(&cleanup_list);
        return 4;
    }

    //READ crc32
    crc32_type loaded_crc32;
    size_t crc32_size = CRC32_SIZE;
    memcpy(&loaded_crc32, start + offset, crc32_size);
    offset += crc32_size;

    //READ payload_data
    struct binarydataarray payload_data;
    payload_data.capacity = payloadsize_value - (payloadsize_size + signature_size_bytes + crc32_size);
    payload_data.length = payload_data.capacity;
    payload_data.array = TRACKED_MALLOC(payload_data.length * sizeof(binarydata_type));
    if (!payload_data.array) {
        LOG_MESSAGE(ERROR, "Memory allocation failed.");
        cleanup_run_commands(&cleanup_list);
        return 6;
    }
    CLEANUP_ADD_COMMAND(&cleanup_list, payload_data.array);
    memcpy(payload_data.array, start + offset, payload_data.length * sizeof(binarydata_type));


    //check crc32
    crc32_type computed_crc32;
    is_success = compute_crc32(payload_data, &computed_crc32);
    if (!is_success) {
        LOG_MESSAGE(ERROR, "Unable to compute CRC32.");
        cleanup_run_commands(&cleanup_list);
        return 6;
    }

    if (computed_crc32 != loaded_crc32) {
        LOG_MESSAGE(ERROR, "File was corrupted, CRC32 doesnt match.");
        cleanup_run_commands(&cleanup_list);
        return 5;
    }

    //Decompress payload_data
    is_success = decompress_payload(payload_data, ptr_return_payload);
    if (!is_success) {
        LOG_MESSAGE(ERROR, "Unable to decompress payload data.");
        cleanup_run_commands(&cleanup_list);
        return 5;
    }

    cleanup_run_commands(&cleanup_list);


    return 0;
}
//endregion

//region PRIVATE FUNCTIONS
/**
 * Function to read binary data from a file and return an array of boolean values
 * @param filename  is the path to the file
 * @return
 *  a pointer to the boolean array
 *  NULL if there was an error
 */
static bool
get_array_from_payload_file(const char *filename, struct binarydataarray *ptr_return_output) {
    //Check arguments
    if (!filename || !ptr_return_output) {
        LOG_MESSAGE(ERROR, "Wrong arguments.");
        return false;
    }
    //Check if file exists
    if (!file_exists(filename)) {
        LOG_MESSAGE(ERROR, "File does not exist.");
        return false;
    }


    //Prepare ptr_output
    ptr_return_output->length = 0;
    ptr_return_output->capacity = 0;
    ptr_return_output->array = NULL;

    // Open the file
    FILE *file = fopen(filename, "rb");
    if (!file) {
        LOG_MESSAGE(ERROR, "Unable to open or read file %s", filename);
        return false;
    }

    // Determine the size of the file
    fseek(file, 0, SEEK_END);
    size_t file_size_bytes = ftell(file);
    rewind(file);

    // Allocate memory for the output array
    ptr_return_output->capacity = (size_t) ((double) file_size_bytes * (1 / (double) sizeof(binarydata_type)));
    ptr_return_output->array = TRACKED_MALLOC(ptr_return_output->capacity * sizeof(binarydata_type));
    if (!ptr_return_output->array) {
        fclose(file);
        LOG_MESSAGE(ERROR, "Memory allocation failed.");
        return false;
    }

    size_t bytes_read = fread(ptr_return_output->array, sizeof(binarydata_type), ptr_return_output->capacity, file);
    if (ferror(file) || feof(file)) {
        // An error occurred during fread
        fclose(file);
        TRACKED_FREE(ptr_return_output->array);
        LOG_MESSAGE(ERROR, "Error reading file.");
        return false;
    }
    if (bytes_read != file_size_bytes) {
        fclose(file);
        TRACKED_FREE(ptr_return_output->array);
        LOG_MESSAGE(ERROR, "Unable to read the entire file.");
        return false;
    }
    ptr_return_output->length = ptr_return_output->capacity;


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
static bool compute_crc32(const struct binarydataarray data, crc32_type *ptr_return_result) {
    //SANITY CHECK
    if (!data.array || !ptr_return_result) {
        LOG_MESSAGE(ERROR, "Wrong arguments.");
        return false;
    }

    //check if it 

    uLong crc = crc32(0L, Z_NULL, 0);  // Initialize CRC32

    // Iterate through the boolean array and update CRC32
    for (size_t i = 0; i < data.length; ++i) {
        // Convert boolean value to a byte (0 or 1)
        binarydata_type element = data.array[i];

        // Update CRC32 with the byte
        crc = crc32(crc, &element, sizeof(binarydata_type));
    }
    if (crc > CRC32_MAX_VALUE){
        LOG_MESSAGE(ERROR, "CRC32 is too big.");
        return false;
    }
    (*ptr_return_result) = crc;

    return true;
}
//endregion
//endregion
