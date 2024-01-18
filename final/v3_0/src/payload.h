//
// Author: Filip Cerny
// Date: 02.12.2023
// Description: File contains functions for working with payload data.
//

#ifndef FAV_PC_SP_23_24_BINARYDATAINIMG_PAYLOAD_H
#define FAV_PC_SP_23_24_BINARYDATAINIMG_PAYLOAD_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "utils/binary_data.h"
#include "utils/dictionary.h"

/**
 * BMP payloadsize offset in bytes
 */
#define PAYLOAD_CHECK_OFFSET (PAYLOADSIZE_SIZE + SIGNATURE_SIZE + 1)

/**
 * Represents the type for payloadsize data
 */
typedef uint32_t payloadsize_type;
/**
 * Signature of the payload data
 */
#define SIGNATURE "KIVPCSP_HiddenData"

/**
 * Signature len of the payload data
 */
#define SIGNATURE_SIZE strlen(SIGNATURE)

/**
 * Size of the payload size in bytes
 */
#define PAYLOADSIZE_SIZE sizeof(payloadsize_type)

/**
 * Function to prepare payload data it compresses the data and calculates the CRC32
 * and signs the data
 * @param filename is the path to the file
 * @param final_size is the size of the final data
 * @return
 * 1 if the function was successful
 * 0 if there was an error
 */
bool prepare_payload_data(const char *filename, struct binarydataarray *ptr_return_output);

/**
 * Function extracts payload data from data extracted from image
 * @param hidden_data  is the data extracted from image
 * @param ptr_return_payload  is the pointer to the payload data
 * @return
 * 0 success
 * 4 there is no signature
 * 5 file was corupted, crc32 doesnt match, cannot decompress
 * 6 other error
 */
int extract_payload_from_data(struct binarydataarray *hidden_data, struct dicvaluearray *ptr_return_payload);

/**
 * Checks if the input file is a BMP file.
 * @param input_file  The input file.
 * @return true if the input file is a BMP file, false otherwise.
 */
int payload_get_payloadsize(struct binarydataarray *array, size_t *ptr_return_size);

#endif //FAV_PC_SP_23_24_BINARYDATAINIMG_PAYLOAD_H
