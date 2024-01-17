//
// Author: Filip Cerny
// Date: 07.11.2023
// Description: File contains functions for hiding and extracting data in/from image.
//

#ifndef FAV_PC_SP_23_24_BINARYDATAINIMG_IMAGE_H
#define FAV_PC_SP_23_24_BINARYDATAINIMG_IMAGE_H

#include <stdbool.h>
#include <stdint.h>
#include "utils/binary_data.h"


/**
 * Represents the color value type
 */
typedef uint8_t colorvalue_type;


/**
 * Extracts binary data from the least significant bit (LSB) of each RGB value in a BMP file.
 * @param input_filepath The path to the input BMP file.
 * @param hide_data Pointer to an array of binary data to be hidden.
 * @param output_filepath The path to the output BMP file.
 * @return
 * 0 succesfull
 * 1 cannot open file
 * 2 if input file is not BMP or PNG
 * 3 if payload is too big for image
 * 6 other error
 */
int hide_data_lsb(const char *input_filepath, const struct binarydataarray hide_data,
                  char *output_filepath);

/**
 * Extracts binary data from the least significant bit (LSB) of each RGB value in a BMP file.
 * @param input_image_filepath The path to the input BMP file.
 * @param ptr_return_hidden_data Pointer to an array of binary data to be hidden.
 * @return
 * 0 succesfull
 * 1 cannot open file
 * 2 if input file is not BMP or PNG; not 24 bit RGB
 * 6 other error
 */
int extract_data_lsb(const char *input_image_filepath, struct binarydataarray *ptr_return_hidden_data);


#endif //FAV_PC_SP_23_24_BINARYDATAINIMG_IMAGE_H
