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


#define STRING_NULL_TERMINATOR '\0'
#define DIC_CODE_INVALID -1

#define LZW_COMPRESSION_RATE 0.6
#define DIC_VALUE_INVALID NULL
typedef unsigned int DicCodeType;
typedef unsigned char *DicValueType;


typedef struct {
    size_t length;
    unsigned char array[];
} CharArray;

bool lzw_compress(const char *data, DicCodeArray *compressed_data);

bool lzw_decompress(const DicCodeArray *compressed_data, DicValueArray *decompressed_data);

#endif //FAV_PC_SP_23_24_BINARYDATAINIMG_COMPRESSION_H
