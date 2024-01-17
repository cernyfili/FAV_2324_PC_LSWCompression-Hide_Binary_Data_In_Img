//
// Author: Filip Cerny
// Date: 07.11.2023
// Description: File for compression and decompression of data
//

#ifndef FAV_PC_SP_23_24_BINARYDATAINIMG_COMPRESSION_H
#define FAV_PC_SP_23_24_BINARYDATAINIMG_COMPRESSION_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utils/data_structures.h"


/**
 * Last char in the compressed data
 */
#define LZW_DECOMPRESS_LAST_CHAR '\000'

/**
 * Compresses the data using LZW algorithm
 * @param data  The data to be compressed
 * @param ptr_return_compressed_data  The pointer to the compressed data
 * @return  True if the compression was successful, false otherwise
 */
bool compress_payload(struct payloadarray data, struct payloadarray *ptr_return_compressed_data);

/**
 * Decompresses the data using LZW algorithm
 * @param compressed_data  The data to be decompressed
 * @param ptr_return_uncompressed_data  The pointer to the uncompressed data
 * @return  True if the decompression was successful, false otherwise
 */
bool decompress_payload(struct payloadarray compressed_data, char **ptr_return_uncompressed_data);


#endif //FAV_PC_SP_23_24_BINARYDATAINIMG_COMPRESSION_H
