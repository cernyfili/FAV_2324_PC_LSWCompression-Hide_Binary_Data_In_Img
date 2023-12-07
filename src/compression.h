//
// Author: Lenovo
// Date: 07.11.2023
// Description: 
//

#ifndef FAV_PC_SP_23_24_BINARYDATAINIMG_COMPRESSION_H
#define FAV_PC_SP_23_24_BINARYDATAINIMG_COMPRESSION_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

unsigned short* compress_binary_data(const unsigned char* data, size_t size, size_t* compressed_size);

unsigned char* decompress_binary_data(const unsigned short* compressed_data, size_t compressed_size, size_t* decompressed_size);

#endif //FAV_PC_SP_23_24_BINARYDATAINIMG_COMPRESSION_H
