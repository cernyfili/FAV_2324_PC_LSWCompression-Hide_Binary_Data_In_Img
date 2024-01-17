//
// Author: Filip Cerny
// Date: 02.12.2023
// Description: This file contains functions for copying files and checking if file exists. and is imports headers
//

#ifndef FAV_PC_SP_23_24_BINARYDATAINIMG_UTILS_H
#define FAV_PC_SP_23_24_BINARYDATAINIMG_UTILS_H

#include "logger.h"
#include "memory_management.h"
#include "binary_data.h"


//region DEFINE, MACROS
/**
 * Calculates the memory size required for storing n elements of type char.
 */
#define CALC_STR_MEM_SIZE(n) ((n + 1) * sizeof(char))

/**
 * Calculates the real memory size required for storing n elements of type char.
 */
#define CALC_STR_REAL_MEM_SIZE(n) (( strlen(n) + 1 ) * sizeof(char))

/**
 *  Adds one to the value n.
 */
#define STR_ADD_ONE(n) ((n) + 1)

/**
 * Calculates the real length of a string, excluding the null terminator.
 */
#define CALC_STR_REAL_LEN(n) (strlen(n) + 1)

/**
 * Specifies the number of bits in a byte.
 */
#define BITS_IN_BYTE 8.0

/**
 * Specifies the null terminator character for strings.
 */
#define STRING_NULL_TERMINATOR '\0'

/**
 * Buffer size for copying files
 */
#define BUFFER_SIZE 4096

//endregion

/**
 * Copy file from input_file to output_file
 * @param input_file is file from which we copy
 * @param output_file is file to which we copy
 * @return true if copy was successful, false otherwise
 */
bool copy_files(FILE *input_file, FILE *output_file);

/**
 * Check if file exists
 * @param filename is name of file
 * @return true if file exists, false otherwise
 */
bool file_exists(const char *filename);

#endif //FAV_PC_SP_23_24_BINARYDATAINIMG_UTILS_H
