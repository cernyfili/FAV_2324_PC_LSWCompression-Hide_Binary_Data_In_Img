//
// Author: Lenovo
// Date: 07.11.2023
// Description: 
//

#include <stdbool.h>
#include "utils/data_structures.h"

#ifndef FAV_PC_SP_23_24_BINARYDATAINIMG_IMAGE_H
#define FAV_PC_SP_23_24_BINARYDATAINIMG_IMAGE_H


#define NUM_COLOR_PIXEL 3

/**
 * Extracts binary data from the least significant bit (LSB) of each RGB value in a BMP file.
 * @param input_filepath
 * @param hide_data
 * @param output_filepath
 * @return
 */
int hide_data_lsb(const char *input_filepath, const PayloadArray hide_data,
                   const char *output_filepath);

/**
 * Extracts binary data from the least significant bit (LSB) of each RGB value in a BMP file.
 * @param input_image_filepath
 * @param ptr_hidden_data
 * @return
 */
int extract_data_lsb(const char *input_image_filepath, PayloadArray *ptr_hidden_data);


#endif //FAV_PC_SP_23_24_BINARYDATAINIMG_IMAGE_H
