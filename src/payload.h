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
#include "utils/data_structures.h"

/**
 * Function to prepare payload data it compresses the data and calculates the CRC32
 * and signs the data
 * @param filename is the path to the file
 * @param final_size is the size of the final data
 * @return
 * 1 if the function was successful
 * 0 if there was an error
 */
bool prepare_payload_data(const char *filename, PayloadArray *ptr_return_output);

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
int extract_payload_from_data(PayloadArray hidden_data, char **ptr_return_payload);

#endif //FAV_PC_SP_23_24_BINARYDATAINIMG_PAYLOAD_H
