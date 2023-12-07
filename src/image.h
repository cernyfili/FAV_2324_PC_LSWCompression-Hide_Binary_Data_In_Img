//
// Author: Lenovo
// Date: 07.11.2023
// Description: 
//

#include <stdbool.h>

#ifndef FAV_PC_SP_23_24_BINARYDATAINIMG_IMAGE_H
#define FAV_PC_SP_23_24_BINARYDATAINIMG_IMAGE_H

/**
 * Hides binary data in the Least Significant Bit (LSB) of each pixel in a PNG image.
 *
 * @param input_filepath - The path to the input PNG file.
 * @param binary_data - The binary data to hide. Each bit of the data will be embedded in the LSB of the image.
 * @param output_filepath - The path to the output PNG file.
 * @return true if the data hiding is successful, false otherwise.
 */
bool hide_data_lsb_bmp(const char *input_filepath, const bool *binary_data,
                       const char *output_filepath);

/**
 * Hides binary data in the least significant bit (LSB) of each RGB value in a BMP file.
 *
 * @param input_filepath - Path to the input BMP file.
 * @param binary_data - Pointer to an array of binary data to be hidden.
 * @param output_filepath - Path to the output BMP file.
 * @return 0 if the hiding process is successful, non-zero otherwise.
 */
bool hide_data_lsb_bmp(const char *input_filepath, const bool *binary_data,
                       const char *output_filepath);



#endif //FAV_PC_SP_23_24_BINARYDATAINIMG_IMAGE_H
